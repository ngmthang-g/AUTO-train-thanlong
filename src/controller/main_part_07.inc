            Send(BridgeCommand::ProveUnityMainThread, L"ProveUnityMainThread");
            return;
        }
        if (completed == BridgeCommand::ProveUnityMainThread) {
            if ((snapshot_.validMask & ValidUnityMainThread) == 0) {
                Fail(L"MainThread proof không đặt ValidUnityMainThread; fail-closed");
                return;
            }
            Enter(FoundationState::ReadingSnapshot,
                  L"MAINTHREAD PROVEN; đọc snapshot đầu rồi chuyển sang continuous scanner read-only");
            Send(BridgeCommand::ReadGameSnapshot, L"ReadGameSnapshot");
            return;
        }
        if (completed == BridgeCommand::ProveHookActionEnvelope) {
            HandleHarmlessProofResponse(response);
            return;
        }
    }

    bool PopEvent(std::wstring& out) {
        if (events_.empty()) return false;
        out = std::move(events_.front());
        events_.pop_front();
        return true;
    }

private:
    static bool IsScannerState(FoundationState state) {
        return state == FoundationState::WaitingStableMap ||
               state == FoundationState::ObservingPartial ||
               state == FoundationState::QualifyingScanner ||
               state == FoundationState::Observing ||
               state == FoundationState::RecoveringScanner;
    }

    enum RuntimeEdge : std::uint32_t {
        EdgeDead          = 1u << 0,
        EdgeRiding        = 1u << 1,
        EdgeMoving        = 1u << 2,
        EdgeAutoPath      = 1u << 3,
        EdgeMapTransition = 1u << 4,
    };

    static constexpr int kQualificationStableScans = 60; // ~30 s at 500 ms

    std::wstring PendingRuntimeEdges() const {
        std::wstring pending;
        auto add = [&](std::uint32_t bit, const wchar_t* name) {
            if (edgeCoverageMask_ & bit) return;
            if (!pending.empty()) pending += L", ";
            pending += name;
        };
        add(EdgeMoving, L"moving");
        add(EdgeRiding, L"riding");
        add(EdgeAutoPath, L"autoPath");
        add(EdgeDead, L"dead");
        add(EdgeMapTransition, L"mapTransition");
        return pending.empty() ? L"none" : pending;
    }

    void EmitCoverageIfChanged(std::uint32_t beforeMask) {
        if (beforeMask == edgeCoverageMask_) return;
        events_.push_back(L"PID " + std::to_wstring(game_.pid) +
                          L": RUNTIME EDGE COVERAGE • pending=" + PendingRuntimeEdges());
    }

    static int CountBits(std::uint32_t value) {
        int count = 0;
        while (value) {
            count += static_cast<int>(value & 1u);
            value >>= 1u;
        }
        return count;
    }

    void SetControlFsmState(ControlFsmState next, const std::wstring& why) {
        if (controlState_ == next) return;
        const ControlFsmState before = controlState_;
        controlState_ = next;
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": FSM • " + ControlFsmStateName(before) + L" → " +
            ControlFsmStateName(next) + L" • " + why);
    }

    SafetyGuardContext BuildSafetyGuardContext() const {
        SafetyGuardContext ctx{};
        ctx.bridgeAttached = bridge_.Attached();
        ctx.mainThreadProven = (snapshot_.validMask & ValidUnityMainThread) != 0;
        ctx.scannerQualified = qualificationStableScans_ >= kQualificationStableScans;
        ctx.observerStable =
            (gameSnapshot_.validMask & kRequiredObserverStableMask) == kRequiredObserverStableMask;
        ctx.transitionActive = transitionLatched_;
        ctx.scannerHealthy = scannerFailures_ == 0;
        ctx.roleID = gameSnapshot_.roleID;
        ctx.mapID = gameSnapshot_.mapID;
        ctx.dead = gameSnapshot_.dead != 0;
        return ctx;
    }

    void HandleHarmlessProofResponse(const BridgeResponse& response) {
        auto& tracker = g_harmlessProofTrackers[game_.pid];
        if (!tracker.issued || tracker.activeToken == 0 || actionQueue_.Active() != 1) {
            Fail(L"HARMLESS POST FAIL: không có active infrastructure intent; fail-closed");
            return;
        }
        if ((response.snapshot.validMask & ValidUnityMainThread) == 0) {
            Fail(L"HARMLESS POST FAIL: bridge không re-prove Unity main thread; fail-closed");
            return;
        }

        InfrastructureProofObserved observed{};
        observed.token = response.infrastructureProof.token;
        observed.sequence = response.infrastructureProof.sequence;
        observed.callbackThreadId = response.infrastructureProof.callbackThreadId;
        observed.currentManagedThreadId = response.infrastructureProof.currentManagedThreadId;
        observed.unityMainManagedThreadId = response.infrastructureProof.unityMainManagedThreadId;
        observed.completed = response.infrastructureProof.completed != 0;

        if (!InfrastructureProofPostconditionSatisfied(
                tracker.activeToken,
                tracker.lastSequence,
                tracker.expectedHookThreadId,
                tracker.expectedManagedThreadId,
                tracker.expectedUnityMainThreadId,
                observed)) {
            Fail(L"HARMLESS POST FAIL: token/sequence/thread identity không khớp PRE; fail-closed");
            return;
        }
        if (!actionQueue_.CompleteActive()) {
            Fail(L"HARMLESS POST FAIL: ActionQueue active completion invariant lỗi");
            return;
        }

        tracker.lastSequence = observed.sequence;
        tracker.passed = true;
        tracker.issued = false;
        tracker.activeToken = 0;
        lastGuardBlockReason_.clear();
        SetControlFsmState(ControlFsmState::IdleStable,
                           L"harmless hook/main-thread envelope POST verified; gameplay mutation vẫn khóa");
        detail_ = L"HARMLESS DISPATCHER PROOF PASS • token/seq/thread POST verified • queue 0/1 • gameplay mutation LOCKED";
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
        nextSnapshotDue_ = Clock::now();
    }

    void ConsiderHarmlessProof() {
        auto& tracker = g_harmlessProofTrackers[game_.pid];
        if (tracker.passed || tracker.issued) return;

        ActionIntent intent{};
        intent.kind = ActionIntentKind::HarmlessInfrastructureProof;
        intent.roleID = gameSnapshot_.roleID;
        intent.mapID = gameSnapshot_.mapID;
        intent.snapshotSequence = gameSnapshot_.sequence;
        intent.token = tracker.nextToken++;
        if (intent.token == 0) intent.token = tracker.nextToken++;

        const SafetyGuardDecision guard = SafetyGuard::Evaluate(intent, BuildSafetyGuardContext());
        if (!guard.preconditionsPass) {
            const std::wstring reason = guard.reason;
            if (reason != lastGuardBlockReason_) {
                lastGuardBlockReason_ = reason;
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": HARMLESS SAFETY GUARD BLOCK • reason=" + reason +
                    L" • no command sent");
            }
            return;
        }

        if (!actionQueue_.TryEnqueue(intent)) {
            Fail(L"HARMLESS PROOF: ActionQueue TryEnqueue vượt MAX=1");
            return;
        }
        ActionIntent active{};
        if (!actionQueue_.BeginNext(active) ||
            active.kind != ActionIntentKind::HarmlessInfrastructureProof ||
            active.token != intent.token) {
            Fail(L"HARMLESS PROOF: queued→active invariant lỗi");
            return;
        }
        const DispatchDecision dispatch = InfrastructureDispatcherGate::Evaluate(active.kind);
        if (!dispatch.allowed) {
            Fail(L"HARMLESS PROOF: infrastructure dispatcher gate từ chối unexpectedly");
            return;
        }

        tracker.issued = true;
        tracker.activeToken = active.token;
        tracker.expectedHookThreadId = snapshot_.hookThreadId;
        tracker.expectedManagedThreadId = snapshot_.currentManagedThreadId;
        tracker.expectedUnityMainThreadId = snapshot_.unityMainManagedThreadId;
        lastGuardBlockReason_.clear();

        SetControlFsmState(ControlFsmState::ActionPrepared,
                           L"harmless infrastructure intent queued→active; PRE PASS");
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": ACTION QUEUE • depth=" + std::to_wstring(actionQueue_.Depth()) +
            L"/1 active=" + std::to_wstring(actionQueue_.Active()) +
            L" • harmless token=" + std::to_wstring(active.token));

        if (!Send(BridgeCommand::ProveHookActionEnvelope,
                  L"ProveHookActionEnvelope", true, active.token)) return;
        SetControlFsmState(ControlFsmState::AwaitingPostcondition,
                           L"harmless command sent on proven hook/main thread; chờ token/seq/thread POST");
    }

    void DrainDryRunActionQueue() {
        ActionIntent intent{};
        if (!actionQueue_.Pop(intent)) return;
        if (kGameplayMutationEnabled) {
            Fail(L"Compile-time mutation invariant bị phá; fail-closed");
            return;
        }
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": DISPATCHER GATE • intent=" + ActionIntentName(intent.kind) +
            L" • BLOCKED: kGameplayMutationEnabled=false; không phát BridgeCommand/action game");
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": ACTION QUEUE • depth=" + std::to_wstring(actionQueue_.Depth()) +
            L"/1 active=" + std::to_wstring(actionQueue_.Active()) +
            L" • dry-run envelope đã tiêu thụ an toàn");
    }

    void ConsiderReviveDryRun() {
        ActionIntent intent{};
        intent.kind = ActionIntentKind::Revive;
        intent.roleID = gameSnapshot_.roleID;
        intent.mapID = gameSnapshot_.mapID;
        intent.snapshotSequence = gameSnapshot_.sequence;

        const SafetyGuardDecision decision = SafetyGuard::Evaluate(intent, BuildSafetyGuardContext());
        if (!decision.preconditionsPass) {
            const std::wstring reason = decision.reason;
            if (reason != lastGuardBlockReason_) {
                lastGuardBlockReason_ = reason;
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": SAFETY GUARD BLOCK • intent=Revive • reason=" + reason +
                    L" • mutation vẫn KHÓA");
            }
            return;
        }

        lastGuardBlockReason_.clear();
        if (reviveDryRunIssued_) return;
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": SAFETY GUARD PASS(dry-run) • intent=Revive • PRE stable=1 dead=1 transition=0 scanner=60/60"
            L" • mutation permission=LOCKED");

        if (!actionQueue_.TryEnqueue(intent)) {
            Fail(L"ActionQueue invariant lỗi: depth vượt MAX=1");
            return;
        }
        reviveDryRunIssued_ = true;
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": ACTION QUEUE • depth=" + std::to_wstring(actionQueue_.Depth()) +
            L"/1 active=" + std::to_wstring(actionQueue_.Active()) +
            L" • accepted DRY-RUN intent=Revive seq=" +
            std::to_wstring(intent.snapshotSequence));
        DrainDryRunActionQueue();
    }

    void UpdateControlScaffold() {
        if (state_ == FoundationState::Faulted) {
            actionQueue_.Clear();
            SetControlFsmState(ControlFsmState::Faulted, L"foundation/scanner fault");
            return;
        }
        if (transitionLatched_) {
            actionQueue_.Clear();
            lastGuardBlockReason_.clear();
            SetControlFsmState(ControlFsmState::MapTransition,
                               L"transition active; queue cleared; fail-closed");
            return;
        }
        if (!haveGameSnapshot_) {
            actionQueue_.Clear();
            SetControlFsmState(ControlFsmState::SafePaused, L"chưa có snapshot");
            return;
        }

        const bool observerStable =
            (gameSnapshot_.validMask & kRequiredObserverStableMask) == kRequiredObserverStableMask;
        if (scannerFailures_ != 0 || !observerStable) {
            actionQueue_.Clear();
            SetControlFsmState(ControlFsmState::SafePaused,
                               L"scanner/observer chưa đủ điều kiện");
            return;
        }

        if (gameSnapshot_.dead) {
            SetControlFsmState(ControlFsmState::DeadDetected,
                               L"snapshot dead=1; tạo candidate nhưng chưa mutate");
            ConsiderReviveDryRun();
            return;
        }

        reviveDryRunIssued_ = false;
        lastGuardBlockReason_.clear();
        if (qualificationStableScans_ < kQualificationStableScans) {
            SetControlFsmState(ControlFsmState::ScannerQualifying,
                               L"chờ scanner qualification 60/60");
            return;
        }

        if (!g_harmlessProofTrackers[game_.pid].passed) {
            ConsiderHarmlessProof();
            return;
        }

        SetControlFsmState(ControlFsmState::IdleStable,
                           L"scanner qualified + harmless dispatcher proof PASS + alive + stable");
    }

    void ObserveAutoPathProbe(const GameSnapshot& game) {
        if (game.autoPathProbeMask != lastAutoPathProbeMask_ || !autoPathProbeLogged_) {
            lastAutoPathProbeMask_ = game.autoPathProbeMask;
            autoPathProbeLogged_ = true;
            const bool complete =
                (game.autoPathProbeMask & kAutoPathProbeCompleteMask) == kAutoPathProbeCompleteMask;
            events_.push_back(
                L"PID " + std::to_wstring(game_.pid) +
                L": AUTOPATH PROBE • " + std::to_wstring(CountBits(game.autoPathProbeMask)) +
                L"/" + std::to_wstring(CountBits(kAutoPathProbeCompleteMask)) +
                L" resolved • mask=" + std::to_wstring(game.autoPathProbeMask) +
                L" • value=" +
                ((game.validMask & ValidAutoPathState) ? std::to_wstring(game.autoPathing) : L"?") +
                L" • " + (complete ? L"resolver PASS" : L"resolver PARTIAL"));
        }

        if ((game.validMask & ValidAutoPathState) && game.autoPathing) {
            const std::uint32_t coverageBefore = edgeCoverageMask_;
            edgeCoverageMask_ |= EdgeAutoPath;
            EmitCoverageIfChanged(coverageBefore);
            if (!autoPathObservedOnLogged_) {
                autoPathObservedOnLogged_ = true;
                events_.push_back(L"PID " + std::to_wstring(game_.pid) +
                                  L": AUTOPATH OBSERVED ON • getter đã trả 1; semantics read-only PROVEN");
            }
        }
    }

    static bool IdentityChanged(const GameSnapshot& before, const GameSnapshot& after) {
        constexpr std::uint32_t mask = ValidRoleIdentity | ValidMapId;
        if (!HasBoth(before, after, mask)) return false;
        return before.roleID != after.roleID || before.mapID != after.mapID;
    }

    static bool HasBoth(const GameSnapshot& a, const GameSnapshot& b, std::uint32_t mask) {
        return (a.validMask & mask) == mask && (b.validMask & mask) == mask;
    }

