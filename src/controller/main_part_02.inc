            error = L"Không load được bridge. Win32=" + std::to_wstring(loadError) +
                    L" (" + std::wstring(sys) + L") Path=" + bridgePath;
            Close();
            return false;
        }
        HOOKPROC proc = nullptr;
        if (!ResolveProcAddress(localBridge_, "TlGetMessageHook", proc)) {
            error = L"Bridge không export TlGetMessageHook";
            Close();
            return false;
        }
        hook_ = SetWindowsHookExW(WH_GETMESSAGE, proc, localBridge_, game.threadId);
        if (!hook_) {
            error = L"SetWindowsHookEx thất bại; thử chạy tool cùng quyền với game";
            Close();
            return false;
        }
        if (!PostThreadMessageW(game.threadId, kWakeMessage, 0, 0)) {
            error = L"Không đánh thức được message thread game";
            Close();
            return false;
        }
        attached_ = true;
        return true;
    }

    void Close() {
        if (hook_) UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
        if (localBridge_) FreeLibrary(localBridge_);
        localBridge_ = nullptr;
        if (shared_) UnmapViewOfFile(shared_);
        shared_ = nullptr;
        if (mapping_) CloseHandle(mapping_);
        mapping_ = nullptr;
        attached_ = false;
        outstandingSeq_ = 0;
    }

    bool Attached() const { return attached_ && hook_ && shared_; }
    bool Busy() const { return outstandingSeq_ != 0; }
    bool LoadedInTarget() const { return shared_ && shared_->bridgeLoaded != 0; }

    bool Begin(BridgeCommand command, std::wstring& error) {
        if (!Attached()) {
            error = L"Bridge chưa attach";
            return false;
        }
        if (Busy()) {
            error = L"Đang có probe khác; không chạy song song";
            return false;
        }
        const LONG next = shared_->requestSeq + 1;
        shared_->request = {};
        shared_->request.command = static_cast<std::uint32_t>(command);
        MemoryBarrier();
        InterlockedExchange(&shared_->requestSeq, next);
        outstandingSeq_ = next;
        sentAt_ = Clock::now();
        if (!PostThreadMessageW(game_.threadId, kWakeMessage, 0, 0)) {
            outstandingSeq_ = 0;
            error = L"PostThreadMessage thất bại";
            return false;
        }
        return true;
    }

    enum class PollResult { Idle, Pending, Complete, Timeout };

    PollResult Poll(BridgeResponse& out) {
        if (!Busy()) return PollResult::Idle;
        if (shared_->completedSeq == outstandingSeq_) {
            MemoryBarrier();
            out = shared_->response;
            outstandingSeq_ = 0;
            return PollResult::Complete;
        }
        if (Clock::now() - sentAt_ > std::chrono::seconds(5)) {
            outstandingSeq_ = 0;
            return PollResult::Timeout;
        }
        return PollResult::Pending;
    }

private:
    GameProcess game_{};
    HANDLE mapping_ = nullptr;
    SharedBlock* shared_ = nullptr;
    HMODULE localBridge_ = nullptr;
    HHOOK hook_ = nullptr;
    bool attached_ = false;
    LONG outstandingSeq_ = 0;
    Clock::time_point sentAt_{};
};

enum class FoundationState {
    Stopped,
    Attaching,
    ValidatingNative,
    InspectingFgMainThread,
    InspectingUnityDispatcher,
    ProvingUnityMainThread,
    ReadingSnapshot,
    WaitingStableMap,
    ObservingPartial,
    Observing,
    RecoveringScanner,
    Faulted,
};

const wchar_t* StateName(FoundationState state) {
    switch (state) {
        case FoundationState::Stopped: return L"Đã dừng";
        case FoundationState::Attaching: return L"Đang gắn bridge";
        case FoundationState::ValidatingNative: return L"Kiểm tra Hook/IL2CPP";
        case FoundationState::InspectingFgMainThread: return L"Inspect FG MainThread";
        case FoundationState::InspectingUnityDispatcher: return L"Inspect Unity Dispatcher";
        case FoundationState::ProvingUnityMainThread: return L"Chứng minh MainThread";
        case FoundationState::ReadingSnapshot: return L"Đọc Snapshot";
        case FoundationState::WaitingStableMap: return L"CHỜ MAP ỔN ĐỊNH";
        case FoundationState::ObservingPartial: return L"SCANNER PARTIAL";
        case FoundationState::Observing: return L"SCANNER PASS";
        case FoundationState::RecoveringScanner: return L"SCANNER RECOVERY";
        case FoundationState::Faulted: return L"VALIDATOR FAIL";
    }
    return L"?";
}

class Session {
public:
    explicit Session(GameProcess game) : game_(std::move(game)) {}

    DWORD Pid() const { return game_.pid; }
    const GameProcess& Game() const { return game_; }
    FoundationState State() const { return state_; }
