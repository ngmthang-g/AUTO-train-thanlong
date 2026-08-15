        add(ValidMovingState, L"moving");
        add(ValidAutoPathState, L"autoPath");
        return missing;
    }

    void StoreSnapshot(const GameSnapshot& next) {
        if (haveGameSnapshot_) {
            previousGameSnapshot_ = gameSnapshot_;
            EmitStateEdges(previousGameSnapshot_, next);
        }
        gameSnapshot_ = next;
        haveGameSnapshot_ = true;
    }

    void HandleSnapshotResponse(const BridgeResponse& response, Clock::time_point now) {
        const bool firstSnapshot = state_ == FoundationState::ReadingSnapshot;
        if (!response.ok) {
            if (firstSnapshot) {
                AppendReport(response.detail);
                Fail(L"Bridge snapshot lỗi " + std::to_wstring(response.errorCode) + L": " + std::wstring(response.detail));
                return;
            }
            ++scannerFailures_;
            qualificationStableScans_ = 0;
            if (scannerFailures_ == 1) ++scannerRecoveries_;
            detail_ = response.detail;
            if (scannerFailures_ >= 3) {
                Fail(L"Continuous scanner lỗi 3 lần liên tiếp; fail-closed: " + std::wstring(response.detail));
                return;
            }
            state_ = FoundationState::RecoveringScanner;
            nextSnapshotDue_ = now + std::chrono::seconds(1);
            events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": SCANNER RECOVERY " +
                              std::to_wstring(scannerFailures_) + L"/3 • " + std::wstring(response.detail));
            return;
        }

        scannerFailures_ = 0;
        ++totalGoodSnapshots_;
        StoreSnapshot(response.gameSnapshot);
        detail_ = response.detail;
        if (firstSnapshot) AppendReport(response.detail);

        const bool transitionKnown =
            (gameSnapshot_.validMask & (ValidMapReadyState | ValidMapTransitionState)) ==
            (ValidMapReadyState | ValidMapTransitionState);
        const bool inTransition = transitionKnown &&
            (!gameSnapshot_.mapReady || gameSnapshot_.waitingChangeMap);
        if (inTransition) {
            qualificationStableScans_ = 0;
            if (!transitionLatched_) {
                const std::uint32_t coverageBefore = edgeCoverageMask_;
                edgeCoverageMask_ |= EdgeMapTransition;
                ++mapTransitionCount_;
                EmitCoverageIfChanged(coverageBefore);
            }
            transitionLatched_ = true;
            postTransitionStableScans_ = 0;
            if (state_ != FoundationState::WaitingStableMap) {
