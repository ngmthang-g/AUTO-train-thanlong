        report_.clear();
        snapshot_ = {};
        gameSnapshot_ = {};
        previousGameSnapshot_ = {};
        haveGameSnapshot_ = false;
        scannerFailures_ = 0;
        scannerRecoveries_ = 0;
        totalGoodSnapshots_ = 0;
        qualificationStableScans_ = 0;
        edgeCoverageMask_ = 0;
        lastAutoPathProbeMask_ = 0;
        autoPathProbeLogged_ = false;
        autoPathObservedOnLogged_ = false;
        transitionLatched_ = false;
        transitionStartedByIdentity_ = false;
        transitionFlagJoinedLogged_ = false;
        recoveryCandidateValid_ = false;
        recoveryCandidateRoleID_ = 0;
        recoveryCandidateMapID_ = 0;
        postTransitionStableScans_ = 0;
        mapTransitionCount_ = 0;
        nextSnapshotDue_ = Clock::time_point{};
        Enter(FoundationState::Attaching, L"Bắt đầu kiểm tra nền read-only");
    }

    void Disconnect() {
        bridge_.Close();
        pending_ = BridgeCommand::None;
        snapshot_ = {};
        gameSnapshot_ = {};
        previousGameSnapshot_ = {};
        haveGameSnapshot_ = false;
        scannerFailures_ = 0;
        scannerRecoveries_ = 0;
        totalGoodSnapshots_ = 0;
        qualificationStableScans_ = 0;
        edgeCoverageMask_ = 0;
        lastAutoPathProbeMask_ = 0;
        autoPathProbeLogged_ = false;
        autoPathObservedOnLogged_ = false;
        transitionLatched_ = false;
        transitionStartedByIdentity_ = false;
        transitionFlagJoinedLogged_ = false;
        recoveryCandidateValid_ = false;
        recoveryCandidateRoleID_ = 0;
        recoveryCandidateMapID_ = 0;
        postTransitionStableScans_ = 0;
        mapTransitionCount_ = 0;
        nextSnapshotDue_ = Clock::time_point{};
        report_.clear();
        Enter(FoundationState::Stopped, L"Đã ngắt bridge; scanner dừng; không có action game nào được gọi");
    }

    void Tick() {
        if (state_ == FoundationState::Stopped || state_ == FoundationState::Faulted) return;
        const auto now = Clock::now();

        if (state_ == FoundationState::Attaching) {
            std::wstring error;
            if (!bridge_.Attach(game_, error)) {
                Fail(error);
                return;
            }
            Enter(FoundationState::ValidatingNative,
                  L"Bridge hook đã đăng ký; chuẩn bị kiểm tra native-only");
            if (!Send(BridgeCommand::ValidateNative, L"ValidateNative")) return;
        }

        if (IsScannerState(state_) && pending_ == BridgeCommand::None) {
            if (now < nextSnapshotDue_) return;
            if (!Send(BridgeCommand::ReadGameSnapshot, L"ReadGameSnapshot", false)) return;
        }

        if (pending_ == BridgeCommand::None) return;

        BridgeResponse response{};
        const auto poll = bridge_.Poll(response);
        if (poll == BridgeClient::PollResult::Pending || poll == BridgeClient::PollResult::Idle) return;
        if (poll == BridgeClient::PollResult::Timeout) {
            Fail(L"Probe timeout; fail-closed, không retry spam");
            return;
        }

        const BridgeCommand completed = pending_;
        pending_ = BridgeCommand::None;
