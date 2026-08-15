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

