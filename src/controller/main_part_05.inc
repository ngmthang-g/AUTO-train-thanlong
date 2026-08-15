        if (!response.ok) {
            if (firstSnapshot) {
                AppendReport(response.detail);
                Fail(L"Bridge snapshot lỗi " + std::to_wstring(response.errorCode) + L": " + std::wstring(response.detail));
                return;
            }
            ++scannerFailures_;
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
        StoreSnapshot(response.gameSnapshot);
        detail_ = response.detail;
        if (firstSnapshot) AppendReport(response.detail);

        const bool transitionKnown =
            (gameSnapshot_.validMask & (ValidMapReadyState | ValidMapTransitionState)) ==
            (ValidMapReadyState | ValidMapTransitionState);
        const bool inTransition = transitionKnown &&
            (!gameSnapshot_.mapReady || gameSnapshot_.waitingChangeMap);
        if (inTransition) {
            transitionLatched_ = true;
            postTransitionStableScans_ = 0;
            if (state_ != FoundationState::WaitingStableMap) {
                Enter(FoundationState::WaitingStableMap,
                      L"MAP TRANSITION: chỉ scan MapReady/WaitingChangeMap; bỏ qua Leader/AutoPath theo donor guard");
            }
            nextSnapshotDue_ = now + std::chrono::milliseconds(500);
            return;
        }

        if ((gameSnapshot_.validMask & kRequiredGameCoreMask) != kRequiredGameCoreMask) {
            Fail(L"Snapshot stable không đủ core valid mask; fail-closed");
            return;
        }

        if (transitionLatched_) {
            ++postTransitionStableScans_;
            if (postTransitionStableScans_ < 2) {
                state_ = FoundationState::WaitingStableMap;
                detail_ = L"MAP RECOVERY 1/2: đã thấy map stable một lần; chờ scan stable thứ hai theo donor guard";
                nextSnapshotDue_ = now + std::chrono::milliseconds(500);
                return;
            }
            transitionLatched_ = false;
            postTransitionStableScans_ = 0;
            events_.push_back(L"PID " + std::to_wstring(game_.pid) +
                              L": MAP RECOVERY PASS 2/2 • state ổn định trở lại");
        }

        const std::wstring missing = MissingObserverFields(gameSnapshot_);
        if ((gameSnapshot_.validMask & kRequiredObserverStableMask) == kRequiredObserverStableMask) {
            if (state_ != FoundationState::Observing) {
                Enter(FoundationState::Observing,
                      L"SCANNER PASS: donor-state parity read-only đã đủ; quét liên tục 500 ms; action game vẫn KHÓA");
            }
        } else {
            if (state_ != FoundationState::ObservingPartial) {
                Enter(FoundationState::ObservingPartial,
                      L"SCANNER PARTIAL: thiếu " + missing + L"; vẫn chỉ đọc và action tiếp tục KHÓA");
            }
        }
        nextSnapshotDue_ = now + std::chrono::milliseconds(500);
    }

    bool Send(BridgeCommand command, const wchar_t* label, bool logEvent = true) {
        if (pending_ != BridgeCommand::None) {
            Fail(L"Invariant lỗi: có hơn 1 command đang chạy");
            return false;
        }
        std::wstring error;
        if (!bridge_.Begin(command, error)) {
            Fail(std::wstring(label) + L": " + error);
            return false;
        }
        pending_ = command;
        if (logEvent) events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": gửi " + label);
        return true;
    }

    void AppendReport(const std::wstring& line) {
        if (!report_.empty()) report_ += L"\r\n";
        report_ += line;
        detail_ = line;
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + line);
    }

    void Enter(FoundationState next, const std::wstring& why) {
        state_ = next;
        detail_ = why;
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + why);
    }

    void Fail(const std::wstring& why) {
        pending_ = BridgeCommand::None;
        bridge_.Close();
        Enter(FoundationState::Faulted, why);
    }

    GameProcess game_{};
    BridgeClient bridge_{};
    FoundationState state_ = FoundationState::Stopped;
