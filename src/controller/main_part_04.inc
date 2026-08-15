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
    QualifyingScanner,
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
        case FoundationState::QualifyingScanner: return L"SCANNER QUALIFYING";
        case FoundationState::Observing: return L"SCANNER QUALIFIED";
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
