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
            if (reviveAwaitingPost_) {
                Fail(L"REVIVE POST FAIL: scanner query lỗi khi action đang active; không retry callback");
                return;
            }
            ++scannerFailures_;
            qualificationStableScans_ = 0;
            recoveryCandidateValid_ = false;
            postTransitionStableScans_ = 0;
            actionQueue_.Clear();
            SetControlFsmState(ControlFsmState::SafePaused,
                               L"scanner query failure; queue cleared; fail-closed");
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
        const bool hadPreviousSnapshot = haveGameSnapshot_;
        StoreSnapshot(response.gameSnapshot);
        const bool identityChanged =
            hadPreviousSnapshot && IdentityChanged(previousGameSnapshot_, gameSnapshot_);
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
                transitionLatched_ = true;
                transitionStartedByIdentity_ = false;
                transitionFlagJoinedLogged_ = true;
                EmitCoverageIfChanged(coverageBefore);
                events_.push_back(L"PID " + std::to_wstring(game_.pid) +
                                  L": MAP TRANSITION • source=flags • MapReady/WaitingChangeMap active • action LOCKED");
            } else if (transitionStartedByIdentity_ && !transitionFlagJoinedLogged_) {
                transitionFlagJoinedLogged_ = true;
                events_.push_back(L"PID " + std::to_wstring(game_.pid) +
                                  L": MAP TRANSITION SIGNAL • source=flags joined existing source=identity • action LOCKED");
            }
            transitionLatched_ = true;
            recoveryCandidateValid_ = false;
            postTransitionStableScans_ = 0;
            if (!ApplyTransitionQueueBoundary(L"flags")) return;
            lastGuardBlockReason_.clear();
            SetControlFsmState(ControlFsmState::MapTransition,
                               reviveAwaitingPost_
                                   ? L"source=flags; active Revive preserved; all new mutations locked"
                                   : L"source=flags; queue cleared; fail-closed");
            if (state_ != FoundationState::WaitingStableMap) {
