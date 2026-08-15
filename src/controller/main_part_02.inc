    EnumWindows(CollectWindow, reinterpret_cast<LPARAM>(&c));
    std::vector<GameProcess> out;
    for (const auto& [pid, item] : c.windows) {
        if (!HasModule(pid, kModule)) continue;
        DWORD realPid = 0;
        DWORD tid = GetWindowThreadProcessId(item.first, &realPid);
        if (realPid != pid || !tid) continue;
        out.push_back({pid, item.first, tid, item.second});
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.pid < b.pid; });
    return out;
}

class BridgeClient {
public:
    ~BridgeClient() { Close(); }

    bool Attach(const GameProcess& game, std::wstring& error) {
        Close();
        game_ = game;
        wchar_t name[96]{};
        MappingName(game.pid, name, _countof(name));
        mapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                      sizeof(SharedBlock), name);
        if (!mapping_) {
            error = L"Không tạo được shared mapping";
            return false;
        }
        shared_ = reinterpret_cast<SharedBlock*>(MapViewOfFile(mapping_, FILE_MAP_ALL_ACCESS, 0, 0,
                                                               sizeof(SharedBlock)));
        if (!shared_) {
            error = L"Không map được shared block";
            Close();
            return false;
        }
        ZeroMemory(shared_, sizeof(*shared_));
        shared_->magic = kMagic;
        shared_->protocolVersion = kProtocolVersion;
        shared_->targetPid = game.pid;
        shared_->targetWindowThreadId = game.threadId;

        const std::wstring bridgePath = ExeDirectory() + L"\\ThanLongNewCoreBridge.dll";
        const UINT oldMode = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
        SetLastError(ERROR_SUCCESS);
        localBridge_ = LoadLibraryW(bridgePath.c_str());
        const DWORD loadError = GetLastError();
        SetErrorMode(oldMode);
        if (!localBridge_) {
            wchar_t sys[512]{};
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, loadError,
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), sys, _countof(sys), nullptr);
            error = L"Không load được bridge. Win32=" + std::to_wstring(loadError) +
                    L" (" + std::wstring(sys) + L") Path=" + bridgePath;
            Close();
            return false;
        }
        HOOKPROC proc = nullptr;
        if (!ResolveProcAddress(localBridge_, "TlGetMessageHook", proc)) {
