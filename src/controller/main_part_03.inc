    const FoundationSnapshot& Snapshot() const { return snapshot_; }
    const GameSnapshot& GameState() const { return gameSnapshot_; }
    const std::wstring& Detail() const { return detail_; }
    bool BridgeLoaded() const { return bridge_.LoadedInTarget(); }

    void ValidateWanted() {
        if (state_ == FoundationState::Attaching ||
            state_ == FoundationState::ValidatingNative ||
            state_ == FoundationState::InspectingFgMainThread ||
            state_ == FoundationState::InspectingUnityDispatcher ||
            state_ == FoundationState::ProvingUnityMainThread ||
            state_ == FoundationState::ReadingSnapshot) return;
        report_.clear();
        snapshot_ = {};
        gameSnapshot_ = {};
        previousGameSnapshot_ = {};
        haveGameSnapshot_ = false;
        scannerFailures_ = 0;
        transitionLatched_ = false;
        postTransitionStableScans_ = 0;
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
        transitionLatched_ = false;
        postTransitionStableScans_ = 0;
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
        snapshot_.validMask |= response.snapshot.validMask;
        if (response.snapshot.hookThreadId) snapshot_.hookThreadId = response.snapshot.hookThreadId;
        if (response.snapshot.windowThreadId) snapshot_.windowThreadId = response.snapshot.windowThreadId;
        if (response.snapshot.resolvedExports) snapshot_.resolvedExports = response.snapshot.resolvedExports;
        if (response.snapshot.requiredExports) snapshot_.requiredExports = response.snapshot.requiredExports;
        if (response.snapshot.fgMainThreadMethodCount) snapshot_.fgMainThreadMethodCount = response.snapshot.fgMainThreadMethodCount;
        if (response.snapshot.fgMainThreadFieldCount) snapshot_.fgMainThreadFieldCount = response.snapshot.fgMainThreadFieldCount;
        if (response.snapshot.unityDispatcherMethodCount) snapshot_.unityDispatcherMethodCount = response.snapshot.unityDispatcherMethodCount;
        if (response.snapshot.unityDispatcherFieldCount) snapshot_.unityDispatcherFieldCount = response.snapshot.unityDispatcherFieldCount;
        if (response.snapshot.currentManagedThreadId) snapshot_.currentManagedThreadId = response.snapshot.currentManagedThreadId;
        if (response.snapshot.unityMainManagedThreadId) snapshot_.unityMainManagedThreadId = response.snapshot.unityMainManagedThreadId;

        if (completed == BridgeCommand::ReadGameSnapshot) {
            HandleSnapshotResponse(response, now);
            return;
        }

        AppendReport(response.detail);
        if (!response.ok) {
            Fail(L"Bridge validator lỗi " + std::to_wstring(response.errorCode) + L": " + std::wstring(response.detail));
            return;
        }

        if (completed == BridgeCommand::ValidateNative) {
            Enter(FoundationState::InspectingFgMainThread,
                  L"Native PASS; inspect metadata FGStudio.Engine.Utilities.MainThread");
