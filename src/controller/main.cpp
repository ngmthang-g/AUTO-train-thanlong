#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cwchar>
#include <cstring>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <set>
#include <algorithm>
#include <chrono>
#include "../common/protocol.h"

using namespace tlcore;
using Clock = std::chrono::steady_clock;

namespace {

template <typename T>
bool ResolveProcAddress(HMODULE module, const char* name, T& out) {
    out = nullptr;
    FARPROC raw = GetProcAddress(module, name);
    if (!raw) return false;
    static_assert(sizeof(raw) == sizeof(out), "Windows function pointer size mismatch");
    std::memcpy(&out, &raw, sizeof(out));
    return out != nullptr;
}

constexpr wchar_t kTitle[] = L"Thần Long Auto - NewCore v1.0.4 Foundation Validator";
constexpr wchar_t kModule[] = L"GameAssembly.dll";
constexpr int IDC_LIST = 1001;
constexpr int IDC_SCAN = 1002;
constexpr int IDC_VALIDATE = 1003;
constexpr int IDC_DISCONNECT = 1004;
constexpr int IDC_LOG = 1005;
constexpr UINT_PTR TIMER_ID = 1;

std::wstring ExeDirectory() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (wchar_t* slash = wcsrchr(path, L'\\')) *slash = 0;
    return path;
}

bool HasModule(DWORD pid, const wchar_t* moduleName) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    bool found = false;
    if (Module32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szModule, moduleName) == 0) {
                found = true;
                break;
            }
        } while (Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return found;
}

struct GameProcess {
    DWORD pid = 0;
    HWND window = nullptr;
    DWORD threadId = 0;
    std::wstring title;
};

struct WindowCollector {
    std::map<DWORD, std::pair<HWND, std::wstring>> windows;
};

BOOL CALLBACK CollectWindow(HWND window, LPARAM lp) {
    if (!IsWindowVisible(window) || GetWindowTextLengthW(window) <= 0) return TRUE;
    DWORD pid = 0;
    DWORD tid = GetWindowThreadProcessId(window, &pid);
    if (!pid || !tid) return TRUE;
    wchar_t title[512]{};
    GetWindowTextW(window, title, _countof(title));
    auto& c = *reinterpret_cast<WindowCollector*>(lp);
    if (!c.windows.count(pid)) c.windows[pid] = {window, title};
    return TRUE;
}

std::vector<GameProcess> FindGames() {
    WindowCollector c;
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
    ReadyForNextPhase,
    Faulted,
};

const wchar_t* StateName(FoundationState state) {
    switch (state) {
        case FoundationState::Stopped: return L"Đã dừng";
        case FoundationState::Attaching: return L"Đang gắn bridge";
        case FoundationState::ValidatingNative: return L"Kiểm tra Hook/IL2CPP";
        case FoundationState::InspectingFgMainThread: return L"Inspect FG MainThread";
        case FoundationState::InspectingUnityDispatcher: return L"Inspect Unity Dispatcher";
        case FoundationState::ReadyForNextPhase: return L"VALIDATOR PASS";
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
    const std::wstring& Detail() const { return detail_; }
    bool BridgeLoaded() const { return bridge_.LoadedInTarget(); }

    void ValidateWanted() {
        if (state_ == FoundationState::Attaching ||
            state_ == FoundationState::ValidatingNative ||
            state_ == FoundationState::InspectingFgMainThread ||
            state_ == FoundationState::InspectingUnityDispatcher) return;
        report_.clear();
        snapshot_ = {};
        Enter(FoundationState::Attaching, L"Bắt đầu kiểm tra nền read-only");
    }

    void Disconnect() {
        bridge_.Close();
        pending_ = BridgeCommand::None;
        snapshot_ = {};
        report_.clear();
        Enter(FoundationState::Stopped, L"Đã ngắt bridge; không có action game nào được gọi");
    }

    void Tick() {
        if (state_ == FoundationState::Stopped ||
            state_ == FoundationState::ReadyForNextPhase ||
            state_ == FoundationState::Faulted) return;

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

        AppendReport(response.detail);
        if (!response.ok) {
            Fail(L"Bridge validator lỗi " + std::to_wstring(response.errorCode) + L": " + std::wstring(response.detail));
            return;
        }

        if (completed == BridgeCommand::ValidateNative) {
            Enter(FoundationState::InspectingFgMainThread,
                  L"Native PASS; inspect metadata FGStudio.Engine.Utilities.MainThread");
            Send(BridgeCommand::InspectFgMainThread, L"InspectFgMainThread");
            return;
        }
        if (completed == BridgeCommand::InspectFgMainThread) {
            Enter(FoundationState::InspectingUnityDispatcher,
                  L"FG MainThread đã đọc metadata; inspect UnityMainThreadDispatcher");
            Send(BridgeCommand::InspectUnityDispatcher, L"InspectUnityDispatcher");
            return;
        }
        if (completed == BridgeCommand::InspectUnityDispatcher) {
            Enter(FoundationState::ReadyForNextPhase,
                  L"PASS nền read-only. MainThread action vẫn KHÓA cho tới khi dispatcher được chứng minh.");
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
    bool Send(BridgeCommand command, const wchar_t* label) {
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
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": gửi " + label);
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
    BridgeCommand pending_ = BridgeCommand::None;
    FoundationSnapshot snapshot_{};
    std::wstring detail_ = L"Đã dừng";
    std::wstring report_{};
    std::deque<std::wstring> events_{};
};

HWND g_main = nullptr;
HWND g_list = nullptr;
HWND g_log = nullptr;
std::map<DWORD, std::unique_ptr<Session>> g_sessions;

void Log(const std::wstring& text) {
    if (!g_log) return;
    const int len = GetWindowTextLengthW(g_log);
    SendMessageW(g_log, EM_SETSEL, len, len);
    const std::wstring line = text + L"\r\n";
    SendMessageW(g_log, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
}

void RebuildList() {
    std::set<DWORD> checked;
    const int oldCount = ListView_GetItemCount(g_list);
    for (int i = 0; i < oldCount; ++i) {
        if (!ListView_GetCheckState(g_list, i)) continue;
        LVITEMW old{};
        old.mask = LVIF_PARAM;
        old.iItem = i;
        if (ListView_GetItem(g_list, &old)) checked.insert(static_cast<DWORD>(old.lParam));
    }

    ListView_DeleteAllItems(g_list);
    int row = 0;
    for (const auto& [pid, session] : g_sessions) {
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = row;
        std::wstring pidText = std::to_wstring(pid);
        item.pszText = pidText.data();
        item.lParam = pid;
        ListView_InsertItem(g_list, &item);

        const auto& s = session->Snapshot();
        std::wstring title = session->Game().title;
        std::wstring bridge = session->BridgeLoaded() ? L"LOADED" : L"NO";
        std::wstring tid = s.hookThreadId ?
            (std::to_wstring(s.hookThreadId) + L"/" + std::to_wstring(s.windowThreadId)) : L"?";
        std::wstring il2cpp = s.requiredExports ?
            (std::to_wstring(s.resolvedExports) + L"/" + std::to_wstring(s.requiredExports)) : L"?";
        std::wstring fg = (s.validMask & ValidFgMainThreadType) ?
            (L"M" + std::to_wstring(s.fgMainThreadMethodCount) + L" F" + std::to_wstring(s.fgMainThreadFieldCount)) : L"?";
        std::wstring ud = (s.validMask & ValidUnityDispatcher) ?
            (L"M" + std::to_wstring(s.unityDispatcherMethodCount) + L" F" + std::to_wstring(s.unityDispatcherFieldCount)) : L"?";
        std::wstring mainThread = L"LOCKED";
        std::wstring detail = session->Detail();

        ListView_SetItemText(g_list, row, 1, title.data());
        ListView_SetItemText(g_list, row, 2, const_cast<wchar_t*>(StateName(session->State())));
        ListView_SetItemText(g_list, row, 3, bridge.data());
        ListView_SetItemText(g_list, row, 4, tid.data());
        ListView_SetItemText(g_list, row, 5, il2cpp.data());
        ListView_SetItemText(g_list, row, 6, fg.data());
        ListView_SetItemText(g_list, row, 7, ud.data());
        ListView_SetItemText(g_list, row, 8, mainThread.data());
        ListView_SetItemText(g_list, row, 9, detail.data());
        if (checked.count(pid)) ListView_SetCheckState(g_list, row, TRUE);
        ++row;
    }
}

std::vector<DWORD> CheckedPids() {
    std::vector<DWORD> out;
    const int n = ListView_GetItemCount(g_list);
    for (int i = 0; i < n; ++i) {
        if (!ListView_GetCheckState(g_list, i)) continue;
        LVITEMW it{};
        it.mask = LVIF_PARAM;
        it.iItem = i;
        if (ListView_GetItem(g_list, &it)) out.push_back(static_cast<DWORD>(it.lParam));
    }
    return out;
}

void ScanGames() {
    auto found = FindGames();
    std::map<DWORD, std::unique_ptr<Session>> next;
    for (auto& game : found) {
        auto old = g_sessions.find(game.pid);
        if (old != g_sessions.end() &&
            old->second->Game().window == game.window &&
            old->second->Game().threadId == game.threadId) {
            next[game.pid] = std::move(old->second);
            g_sessions.erase(old);
        } else {
            next[game.pid] = std::make_unique<Session>(game);
        }
    }
    g_sessions = std::move(next);
    RebuildList();
    Log(L"Quét thấy " + std::to_wstring(g_sessions.size()) + L" client GameAssembly.dll");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            g_list = CreateWindowW(WC_LISTVIEWW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
                                   12, 12, 1160, 260, hwnd, reinterpret_cast<HMENU>(IDC_LIST), nullptr, nullptr);
            ListView_SetExtendedListViewStyle(g_list, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
            const wchar_t* names[] = {
                L"PID", L"Client", L"Trạng thái", L"Bridge", L"Hook/Window TID",
                L"IL2CPP", L"FG MainThread", L"UnityDispatcher", L"MainThread", L"Chi tiết"
            };
            int widths[] = {80, 190, 170, 90, 140, 80, 120, 130, 100, 430};
            for (int i = 0; i < 10; ++i) {
                LVCOLUMNW c{};
                c.mask = LVCF_TEXT | LVCF_WIDTH;
                c.pszText = const_cast<wchar_t*>(names[i]);
                c.cx = widths[i];
                ListView_InsertColumn(g_list, i, &c);
            }

            CreateWindowW(L"BUTTON", L"Quét client", WS_CHILD | WS_VISIBLE,
                          12, 284, 120, 34, hwnd, reinterpret_cast<HMENU>(IDC_SCAN), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Kiểm tra nền", WS_CHILD | WS_VISIBLE,
                          144, 284, 140, 34, hwnd, reinterpret_cast<HMENU>(IDC_VALIDATE), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Ngắt bridge", WS_CHILD | WS_VISIBLE,
                          296, 284, 130, 34, hwnd, reinterpret_cast<HMENU>(IDC_DISCONNECT), nullptr, nullptr);
            CreateWindowW(L"STATIC",
                          L"Phase 1: READ-ONLY validator. Không gọi hành động game. MainThread phải được chứng minh trước khi mở action.",
                          WS_CHILD | WS_VISIBLE, 448, 292, 720, 26, hwnd, nullptr, nullptr, nullptr);
            g_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                    ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                    12, 332, 1160, 250, hwnd, reinterpret_cast<HMENU>(IDC_LOG), nullptr, nullptr);
            SetTimer(hwnd, TIMER_ID, 100, nullptr);
            ScanGames();
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wp) == IDC_SCAN) {
                ScanGames();
                return 0;
            }
            if (LOWORD(wp) == IDC_VALIDATE) {
                for (DWORD pid : CheckedPids()) {
                    auto it = g_sessions.find(pid);
                    if (it != g_sessions.end()) it->second->ValidateWanted();
                }
                Log(L"Bắt đầu validator read-only cho các client đã tick");
                return 0;
            }
            if (LOWORD(wp) == IDC_DISCONNECT) {
                for (DWORD pid : CheckedPids()) {
                    auto it = g_sessions.find(pid);
                    if (it != g_sessions.end()) it->second->Disconnect();
                }
                Log(L"Đã ngắt bridge các client đã tick");
                return 0;
            }
            break;
        case WM_TIMER:
            if (wp == TIMER_ID) {
                for (auto& entry : g_sessions) {
                    entry.second->Tick();
                    std::wstring event;
                    while (entry.second->PopEvent(event)) Log(event);
                }
                RebuildList();
                return 0;
            }
            break;
        case WM_SIZE: {
            const int w = LOWORD(lp);
            const int h = HIWORD(lp);
            MoveWindow(g_list, 12, 12, w - 24, 260, TRUE);
            MoveWindow(g_log, 12, 332, w - 24, std::max(80, h - 344), TRUE);
            return 0;
        }
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_ID);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES};
    InitCommonControlsEx(&icc);
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"ThanLongNewCoreWindow";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(1));
    RegisterClassExW(&wc);

    g_main = CreateWindowExW(0, wc.lpszClassName, kTitle, WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, 1200, 640,
                             nullptr, nullptr, instance, nullptr);
    if (!g_main) return 1;
    ShowWindow(g_main, show);
    UpdateWindow(g_main);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
