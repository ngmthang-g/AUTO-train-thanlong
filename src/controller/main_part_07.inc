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

    static bool HasBoth(const GameSnapshot& a, const GameSnapshot& b, std::uint32_t mask) {
        return (a.validMask & mask) == mask && (b.validMask & mask) == mask;
    }

