    bool ReviveMetadataReady() const {
        return actionCapabilities_.probeVersion != 0 &&
               (actionCapabilities_.metadataMask & kReviveMetadataSupportMask) ==
                   kReviveMetadataSupportMask;
    }

    bool ApplyTransitionQueueBoundary(const wchar_t* source) {
        if (reviveAwaitingPost_ && actionQueue_.Active() == 1) {
            if ((gameSnapshot_.validMask & ValidRoleIdentity) && reviveExpectedRoleID_ > 0 &&
                gameSnapshot_.roleID != reviveExpectedRoleID_) {
                Fail(L"REVIVE POST FAIL: RoleID đổi trong map transition; fail-closed");
                return false;
            }
            if (!reviveTransitionPreserveLogged_) {
                reviveTransitionPreserveLogged_ = true;
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": ACTION QUEUE • active Revive PRESERVED across map transition source=" +
                    std::wstring(source ? source : L"?") +
                    L" • active=1/1 • KHÓA mọi mutation mới • chỉ chờ dead=0 POST");
            }
            return true;
        }
        actionQueue_.Clear();
        return true;
    }

    void ResetReviveEpisodeAfterAlive() {
        reviveDispatchIssued_ = false;
        reviveAwaitingPost_ = false;
        reviveBlockedThisDeath_ = false;
        reviveActiveToken_ = 0;
        reviveExpectedRoleID_ = 0;
        revivePreMapID_ = 0;
        reviveExpectedHookThreadId_ = 0;
        reviveExpectedManagedThreadId_ = 0;
        reviveExpectedUnityMainThreadId_ = 0;
        deadStableScans_ = 0;
        reviveAliveStableScans_ = 0;
        reviveTransitionPreserveLogged_ = false;
        reviveDeadline_ = Clock::time_point{};
        reviveDryRunIssued_ = true; // keep v1.0.12 dry-run path suppressed.
    }

    void HandleReviveDispatchResponse(const BridgeResponse& response) {
        if (!reviveDispatchIssued_ || reviveActiveToken_ == 0 || actionQueue_.Active() != 1) {
            Fail(L"REVIVE DISPATCH ACK FAIL: không có active Revive intent; fail-closed");
            return;
        }

        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " +
                          std::wstring(response.detail));

        if (!response.ok) {
            // Fail closed for this death episode without retry spam. Keep scanner alive so the
            // user can revive manually and the next death episode can start from a fresh UI tree.
            actionQueue_.CancelAll();
            reviveDispatchIssued_ = false;
            reviveAwaitingPost_ = false;
            reviveBlockedThisDeath_ = true;
            reviveActiveToken_ = 0;
            reviveAliveStableScans_ = 0;
            reviveDeadline_ = Clock::time_point{};
            SetControlFsmState(ControlFsmState::DeadDetected,
                               L"Revive resolver/action BLOCKED; không retry trong death episode này");
            detail_ = L"REVIVE BLOCKED FAIL-CLOSED • " + std::wstring(response.detail) +
                      L" • queue 0/1 • chờ hồi sinh thủ công hoặc death episode mới";
            events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
            nextSnapshotDue_ = Clock::now();
            return;
        }

        ReviveDispatchObserved observed{};
        observed.token = response.reviveAction.token;
        observed.callbackThreadId = response.reviveAction.callbackThreadId;
        observed.currentManagedThreadId = response.reviveAction.currentManagedThreadId;
        observed.unityMainManagedThreadId = response.reviveAction.unityMainManagedThreadId;
        observed.preRoleID = response.reviveAction.preRoleID;
        observed.preMapID = response.reviveAction.preMapID;
        observed.preDead = response.reviveAction.preDead != 0;
        observed.invoked = response.reviveAction.invoked != 0;

        if (!ReviveDispatchAckSatisfied(
                reviveActiveToken_,
                reviveExpectedHookThreadId_,
                reviveExpectedManagedThreadId_,
                reviveExpectedUnityMainThreadId_,
                reviveExpectedRoleID_,
                revivePreMapID_,
                observed)) {
            Fail(L"REVIVE DISPATCH ACK FAIL: token/thread/identity/PRE không khớp; fail-closed");
            return;
        }

        reviveAwaitingPost_ = true;
        reviveAliveStableScans_ = 0;
        reviveDeadline_ = Clock::now() + std::chrono::seconds(30);
        SetControlFsmState(ControlFsmState::AwaitingPostcondition,
                           L"UIButton.HandleClickEvent() đã gọi đúng 1 lần; chờ dead=1→0 POST thật");
        detail_ = L"REVIVE DISPATCH ACK PASS • button=“" +
                  std::wstring(response.reviveAction.buttonLabel) +
                  L"” • queue active=1/1 • NO RETRY • chờ scanner POST";
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
        nextSnapshotDue_ = Clock::now();
    }

    void MaybeRunReviveLifecycle(Clock::time_point now) {
        // Always suppress the old v1.0.12 dry-run path. This function owns Revive in v1.2.0.
        reviveDryRunIssued_ = true;

        if (!haveGameSnapshot_) return;
        const bool stableObserver =
            (gameSnapshot_.validMask & kRequiredObserverStableMask) == kRequiredObserverStableMask;
        if (!stableObserver || scannerFailures_ != 0 || transitionLatched_) return;

        if (reviveAwaitingPost_) {
            if (reviveExpectedRoleID_ <= 0 ||
                ((gameSnapshot_.validMask & ValidRoleIdentity) &&
                 gameSnapshot_.roleID != reviveExpectedRoleID_)) {
                Fail(L"REVIVE POST FAIL: RoleID drift sau callback; fail-closed");
                return;
            }
            if (reviveDeadline_ != Clock::time_point{} && now > reviveDeadline_) {
                Fail(L"REVIVE POST TIMEOUT: không thấy dead=0 trong 30s; không retry callback");
                return;
            }

            if (gameSnapshot_.dead) {
                reviveAliveStableScans_ = 0;
                SetControlFsmState(ControlFsmState::AwaitingPostcondition,
                                   L"Revive active; dead vẫn 1; chỉ chờ observer, không gửi lại action");
                return;
            }

            RevivePostState before{};
            before.roleID = reviveExpectedRoleID_;
            before.mapID = revivePreMapID_;
            before.dead = true;
            before.mapReady = true;
            before.waitingChangeMap = false;

            RevivePostState after{};
            after.roleID = gameSnapshot_.roleID;
            after.mapID = gameSnapshot_.mapID;
            after.dead = gameSnapshot_.dead != 0;
            after.mapReady = gameSnapshot_.mapReady != 0;
            after.waitingChangeMap = gameSnapshot_.waitingChangeMap != 0;

            if (!RevivePostconditionSatisfied(before, after)) {
                reviveAliveStableScans_ = 0;
                return;
            }

            if (reviveAliveStableScans_ < 2) ++reviveAliveStableScans_;
            if (reviveAliveStableScans_ == 1) {
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": REVIVE POST QUALIFY 1/2 • same RoleID=" +
                    std::to_wstring(gameSnapshot_.roleID) + L" • map=" +
                    std::to_wstring(gameSnapshot_.mapID) +
                    L" • dead=0 • queue active=1/1");
                return;
            }

            if (!actionQueue_.CompleteActive()) {
                Fail(L"REVIVE POST FAIL: ActionQueue active completion invariant lỗi");
                return;
            }

            const std::int32_t finalMap = gameSnapshot_.mapID;
            const std::int32_t finalRole = gameSnapshot_.roleID;
            ResetReviveEpisodeAfterAlive();
            lastGuardBlockReason_.clear();
            if (qualificationStableScans_ >= kQualificationStableScans) {
                SetControlFsmState(ControlFsmState::IdleStable,
                                   L"REVIVE POST verified; scanner qualified; queue complete");
            } else {
                SetControlFsmState(ControlFsmState::ScannerQualifying,
                                   L"REVIVE POST verified; map recovery xong, scanner requalifying");
            }
            detail_ = L"REVIVE POST PASS 2/2 • role=" + std::to_wstring(finalRole) +
                      L" • map=" + std::to_wstring(finalMap) +
                      L" • dead=0 • queue 0/1 • fresh UI pointer discarded";
            events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
            return;
        }

        if (!gameSnapshot_.dead) {
            ResetReviveEpisodeAfterAlive();
            return;
        }

        reviveAliveStableScans_ = 0;
        if (deadStableScans_ < 2) ++deadStableScans_;
        if (deadStableScans_ == 1) {
            events_.push_back(
                L"PID " + std::to_wstring(game_.pid) +
                L": REVIVE PRE QUALIFY 1/2 • dead=1 mapReady=1 waitingMap=0 • chưa mutate");
            return;
        }

        if (reviveBlockedThisDeath_ || reviveDispatchIssued_) return;

        const auto harmlessIt = g_harmlessProofTrackers.find(game_.pid);
        const bool harmlessProofPassed =
            harmlessIt != g_harmlessProofTrackers.end() && harmlessIt->second.passed;
        if (!harmlessProofPassed) {
            if (lastGuardBlockReason_ != L"revive-hook-proof") {
                lastGuardBlockReason_ = L"revive-hook-proof";
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": REVIVE BLOCK • harmless hook/main-thread proof chưa PASS; không mutate");
            }
            return;
        }

        const bool capabilityReady = ReviveMetadataReady();
        const DispatchDecision dispatchGate =
            ReviveDispatcherGate::Evaluate(capabilityReady, harmlessProofPassed);
        if (!dispatchGate.allowed) {
            const std::wstring reason = dispatchGate.reason;
            if (reason != lastGuardBlockReason_) {
                lastGuardBlockReason_ = reason;
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": REVIVE DISPATCHER BLOCK • reason=" + reason +
                    L" • metadataMask=" + std::to_wstring(actionCapabilities_.metadataMask));
            }
            return;
        }

        ActionIntent intent{};
        intent.kind = ActionIntentKind::Revive;
        intent.roleID = gameSnapshot_.roleID;
        intent.mapID = gameSnapshot_.mapID;
        intent.snapshotSequence = gameSnapshot_.sequence;
        intent.token = nextReviveToken_++;
        if (intent.token == 0) intent.token = nextReviveToken_++;

        const SafetyGuardDecision guard = SafetyGuard::Evaluate(intent, BuildSafetyGuardContext());
        if (!guard.preconditionsPass) {
            const std::wstring reason = guard.reason;
            if (reason != lastGuardBlockReason_) {
                lastGuardBlockReason_ = reason;
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": REVIVE SAFETY GUARD BLOCK • reason=" + reason + L" • không mutate");
            }
            return;
        }

        if (!actionQueue_.TryEnqueue(intent)) {
            Fail(L"REVIVE ACTION: ActionQueue TryEnqueue vượt MAX=1");
            return;
        }
        ActionIntent active{};
        if (!actionQueue_.BeginNext(active) || active.kind != ActionIntentKind::Revive ||
            active.token != intent.token) {
            Fail(L"REVIVE ACTION: queued→active invariant lỗi");
            return;
        }

        reviveDispatchIssued_ = true;
        reviveActiveToken_ = active.token;
        reviveExpectedRoleID_ = active.roleID;
        revivePreMapID_ = active.mapID;
        reviveExpectedHookThreadId_ = snapshot_.hookThreadId;
        reviveExpectedManagedThreadId_ = snapshot_.currentManagedThreadId;
        reviveExpectedUnityMainThreadId_ = snapshot_.unityMainManagedThreadId;
        reviveTransitionPreserveLogged_ = false;
        lastGuardBlockReason_.clear();

        SetControlFsmState(ControlFsmState::ActionPrepared,
                           L"Revive PRE PASS; fresh resolve sẽ diễn ra trong proven main-thread command");
        events_.push_back(
            L"PID " + std::to_wstring(game_.pid) +
            L": ACTION QUEUE • depth=" + std::to_wstring(actionQueue_.Depth()) +
            L"/1 active=" + std::to_wstring(actionQueue_.Active()) +
            L" • intent=Revive token=" + std::to_wstring(active.token) +
            L" role=" + std::to_wstring(active.roleID) +
            L" map=" + std::to_wstring(active.mapID));

        if (!Send(BridgeCommand::InvokeReviveButton, L"InvokeReviveButton", true,
                  active.token, active.roleID, active.mapID)) {
            return; // Send() already fails closed on transport failure.
        }
        SetControlFsmState(ControlFsmState::AwaitingPostcondition,
                           L"Revive command gửi tới proven hook/main thread; chờ dispatch ACK rồi dead=0 POST");
    }
