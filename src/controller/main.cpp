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

constexpr wchar_t kTitle[] = L"Thần Long Auto - NewCore v1.0.2";
constexpr wchar_t kModule[] = L"GameAssembly.dll";
constexpr int IDC_LIST = 1001;
constexpr int IDC_SCAN = 1002;
constexpr int IDC_START = 1003;
constexpr int IDC_STOP = 1004;
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
    MODULEENTRY32W entry{}; entry.dwSize = sizeof(entry);
    bool found = false;
    if (Module32FirstW(snap, &entry)) {
        do {
            if (_wcsicmp(entry.szModule, moduleName) == 0) { found = true; break; }
        } while (Module32NextW(snap, &entry));
    }
    CloseHandle(snap);
    return found;
}

struct GameProcess { DWORD pid = 0; HWND window = nullptr; DWORD threadId = 0; std::wstring title; };
struct WindowCollector { std::map<DWORD, std::pair<HWND,std::wstring>> windows; };
BOOL CALLBACK CollectWindow(HWND window, LPARAM lp) {
    if (!IsWindowVisible(window) || GetWindowTextLengthW(window) <= 0) return TRUE;
    DWORD pid = 0; DWORD tid = GetWindowThreadProcessId(window, &pid);
    if (!pid || !tid) return TRUE;
    wchar_t title[512]{}; GetWindowTextW(window, title, _countof(title));
    auto& c = *reinterpret_cast<WindowCollector*>(lp);
    if (!c.windows.count(pid)) c.windows[pid] = {window, title};
    return TRUE;
}

std::vector<GameProcess> FindGames() {
    WindowCollector c; EnumWindows(CollectWindow, reinterpret_cast<LPARAM>(&c));
    std::vector<GameProcess> out;
    for (const auto& [pid, item] : c.windows) {
        if (!HasModule(pid, kModule)) continue;
        DWORD realPid = 0; DWORD tid = GetWindowThreadProcessId(item.first, &realPid);
        if (realPid != pid || !tid) continue;
        out.push_back({pid, item.first, tid, item.second});
    }
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b){ return a.pid < b.pid; });
    return out;
}

class BridgeClient {
public:
    ~BridgeClient() { Close(); }
    bool Attach(const GameProcess& game, std::wstring& error) {
        Close(); game_ = game;
        wchar_t name[96]{}; MappingName(game.pid, name, _countof(name));
        mapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                      sizeof(SharedBlock), name);
        if (!mapping_) { error = L"Không tạo được shared mapping"; return false; }
        shared_ = reinterpret_cast<SharedBlock*>(MapViewOfFile(mapping_, FILE_MAP_ALL_ACCESS, 0, 0,
                                                               sizeof(SharedBlock)));
        if (!shared_) { error = L"Không map được shared block"; Close(); return false; }
        ZeroMemory(shared_, sizeof(*shared_));
        shared_->magic = kMagic; shared_->protocolVersion = kProtocolVersion;
        shared_->targetPid = game.pid; shared_->targetWindowThreadId = game.threadId;

        const std::wstring bridgePath = ExeDirectory() + L"\\ThanLongNewCoreBridge.dll";
        localBridge_ = LoadLibraryW(bridgePath.c_str());
        if (!localBridge_) { error = L"Thiếu ThanLongNewCoreBridge.dll"; Close(); return false; }
        HOOKPROC proc = nullptr;
        if (!ResolveProcAddress(localBridge_, "TlGetMessageHook", proc)) {
            error = L"Bridge không export TlGetMessageHook"; Close(); return false;
        }
        hook_ = SetWindowsHookExW(WH_GETMESSAGE, proc, localBridge_, game.threadId);
        if (!hook_) { error = L"SetWindowsHookEx thất bại; thử chạy quyền Admin"; Close(); return false; }
        if (!PostThreadMessageW(game.threadId, kWakeMessage, 0, 0)) {
            error = L"Không đánh thức được thread game"; Close(); return false;
        }
        attached_ = true; return true;
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
        attached_ = false; outstandingSeq_ = 0;
    }
    bool Attached() const { return attached_ && hook_ && shared_; }
    bool Busy() const { return outstandingSeq_ != 0; }

    bool Begin(BridgeCommand command, int arg0, std::wstring& error) {
        if (!Attached() || Busy()) return false;
        const LONG next = shared_->requestSeq + 1;
        shared_->request = {};
        shared_->request.command = static_cast<std::uint32_t>(command);
        shared_->request.intArg0 = arg0;
        MemoryBarrier();
        InterlockedExchange(&shared_->requestSeq, next);
        outstandingSeq_ = next; sentAt_ = Clock::now();
        commandTimeout_ = (command == BridgeCommand::Probe) ? std::chrono::milliseconds(3000)
                                                            : std::chrono::milliseconds(5000);
        if (!PostThreadMessageW(game_.threadId, kWakeMessage, 0, 0)) {
            outstandingSeq_ = 0; error = L"PostThreadMessage thất bại"; return false;
        }
        return true;
    }

    enum class PollResult { Idle, Pending, Complete, Timeout };
    PollResult Poll(BridgeResponse& out) {
        if (!Busy()) return PollResult::Idle;
        if (shared_->completedSeq == outstandingSeq_) {
            MemoryBarrier(); out = shared_->response; outstandingSeq_ = 0; return PollResult::Complete;
        }
        if (Clock::now() - sentAt_ > commandTimeout_) {
            outstandingSeq_ = 0; return PollResult::Timeout;
        }
        return PollResult::Pending;
    }
private:
    GameProcess game_{};
    HANDLE mapping_ = nullptr; SharedBlock* shared_ = nullptr;
    HMODULE localBridge_ = nullptr; HHOOK hook_ = nullptr;
    bool attached_ = false; LONG outstandingSeq_ = 0; Clock::time_point sentAt_{};
    std::chrono::milliseconds commandTimeout_{3000};
};

enum class AutoState {
    Stopped, Initializing, WaitingStable, StartingCombat, Training,
    StoppingCombat, WaitingCombatStop, SafePaused, Faulted
};

const wchar_t* StateName(AutoState s) {
    switch (s) {
        case AutoState::Stopped: return L"Đã dừng";
        case AutoState::Initializing: return L"Khởi tạo";
        case AutoState::WaitingStable: return L"Chờ snapshot ổn định";
        case AutoState::StartingCombat: return L"Đang bật Đánh quái";
        case AutoState::Training: return L"Đang train";
        case AutoState::StoppingCombat: return L"Chuẩn bị dừng AUTO";
        case AutoState::WaitingCombatStop: return L"Chờ xác nhận AUTO dừng";
        case AutoState::SafePaused: return L"SAFE PAUSE";
        case AutoState::Faulted: return L"FAULTED";
    }
    return L"?";
}

struct Action { BridgeCommand command; int arg0; const wchar_t* name; };

class ActionQueue {
public:
    bool Empty() const { return queue_.empty() && !active_; }
    void ClearPending() { queue_.clear(); }
    void Push(Action a) { if (queue_.size() < 8) queue_.push_back(a); }

    bool Tick(BridgeClient& bridge, BridgeResponse& completed, bool& hasCompleted,
              std::wstring& error) {
        hasCompleted = false;
        if (active_) {
            auto poll = bridge.Poll(completed);
            if (poll == BridgeClient::PollResult::Complete) {
                active_ = false; activeName_.clear(); hasCompleted = true; return true;
            }
            if (poll == BridgeClient::PollResult::Timeout) {
                active_ = false;
                error = L"Action timeout: " + activeName_ + L"; fail-closed";
                activeName_.clear();
                return false;
            }
            return true;
        }
        if (queue_.empty()) return true;
        const Action a = queue_.front(); queue_.pop_front();
        if (!bridge.Begin(a.command, a.arg0, error)) return false;
        active_ = true; activeName_ = a.name ? a.name : L"action"; return true;
    }
private:
    std::deque<Action> queue_; bool active_ = false; std::wstring activeName_;
};

class Session {
public:
    explicit Session(GameProcess game) : game_(std::move(game)) {}
    DWORD Pid() const { return game_.pid; }
    const GameProcess& Game() const { return game_; }
    AutoState State() const { return state_; }
    const GameSnapshot& Snapshot() const { return snapshot_; }
    const std::wstring& Detail() const { return detail_; }

    void StartWanted() { wanted_ = true; if (state_ == AutoState::Stopped || state_ == AutoState::Faulted || state_ == AutoState::SafePaused) Enter(AutoState::Initializing, L"Bắt đầu NewCore"); }
    void StopWanted() { wanted_ = false; if (state_ != AutoState::Stopped) Enter(AutoState::StoppingCombat, L"Yêu cầu dừng"); }

    void Tick() {
        BridgeResponse completed{}; bool hasCompleted = false; std::wstring error;
        if (!actions_.Tick(bridge_, completed, hasCompleted, error)) {
            Enter(AutoState::Faulted, error); return;
        }
        if (hasCompleted) {
            if (completed.snapshot.validMask) {
                snapshot_ = completed.snapshot;
                lastSnapshot_ = Clock::now();
                ++snapshotSeq_;
            }
            detail_ = completed.detail;
            if (!completed.ok) {
                Enter(AutoState::Faulted, L"Bridge/probe lỗi: " + std::wstring(completed.detail)); return;
            }
        }

        if (state_ == AutoState::Initializing) {
            if (!bridge_.Attached()) {
                if (!bridge_.Attach(game_, error)) { Enter(AutoState::Faulted, error); return; }
            }
            stableScans_ = 0; lastStableSeq_ = 0; actions_.ClearPending();
            Enter(AutoState::WaitingStable, L"Bridge đã gắn; chỉ probe, chưa action");
        }

        if (state_ == AutoState::WaitingStable) {
            RequestProbeIfIdle();
            if (snapshotSeq_ != lastStableSeq_) {
                lastStableSeq_ = snapshotSeq_;
                if (SnapshotFresh() && SafeSnapshot()) ++stableScans_;
                else stableScans_ = 0;
            }
            if (stableScans_ >= 3) {
                if (!wanted_) { Enter(AutoState::Stopped, L"Không có yêu cầu chạy"); return; }
                if ((snapshot_.validMask & ValidDead) && snapshot_.dead) {
                    Enter(AutoState::SafePaused, L"Nhân vật đang chết; NewCore không bấm mù"); return;
                }
                if (!snapshot_.mapReady) { Enter(AutoState::SafePaused, L"Map chưa ready"); return; }
                if (snapshot_.autoFight) Enter(AutoState::Training, L"AUTO đã chạy");
                else {
                    actions_.Push({BridgeCommand::StartAutoFight, 0, L"StartAutoFight"});
                    postDeadline_ = Clock::now() + std::chrono::seconds(4);
                    Enter(AutoState::StartingCombat, L"PRE OK → StartAutoFight");
                }
            }
            return;
        }

        if (state_ == AutoState::StartingCombat) {
            RequestProbeIfIdle();
            if (SnapshotFresh() && (snapshot_.validMask & ValidAutoFight) && snapshot_.autoFight) {
                Enter(AutoState::Training, L"POST OK: AUTO đã bật"); return;
            }
            if (Clock::now() > postDeadline_) { Enter(AutoState::SafePaused, L"POST fail: AUTO chưa bật; không retry spam"); return; }
            return;
        }

        if (state_ == AutoState::Training) {
            RequestProbeIfIdle();
            if (!wanted_) { Enter(AutoState::StoppingCombat, L"Người dùng dừng"); return; }
            if (!SnapshotFresh()) { Enter(AutoState::SafePaused, L"Snapshot stale >2s"); return; }
            if (!SafeSnapshot()) { Enter(AutoState::SafePaused, L"Mất main-thread proof hoặc state bắt buộc"); return; }
            if (snapshot_.dead) { Enter(AutoState::SafePaused, L"Phát hiện chết; action bị khóa"); return; }
            if (!snapshot_.mapReady) { Enter(AutoState::SafePaused, L"Map chuyển trạng thái; action bị khóa"); return; }
            if ((snapshot_.validMask & ValidAutoFight) && !snapshot_.autoFight) {
                // Fail closed: AUTO tự tắt thì không tự spam bật lại.
                Enter(AutoState::SafePaused, L"AUTO tự tắt; SAFE PAUSE"); return;
            }
            return;
        }

        if (state_ == AutoState::StoppingCombat) {
            if (!bridge_.Attached()) { Enter(AutoState::Stopped, L"Tool đã dừng; bridge không gắn"); return; }
            if (!SnapshotFresh() || !(snapshot_.validMask & ValidAutoFight)) {
                RequestProbeIfIdle();
                return;
            }
            if (!snapshot_.autoFight) { Enter(AutoState::Stopped, L"AUTO đã dừng sẵn"); return; }
            if (!SafeSnapshot() || !snapshot_.mapReady || ((snapshot_.validMask & ValidDead) && snapshot_.dead)) {
                Enter(AutoState::Stopped, L"Tool đã dừng; không mutate game vì snapshot không an toàn");
                return;
            }
            if (actions_.Empty()) {
                actions_.Push({BridgeCommand::StopAutoFight, 0, L"StopAutoFight"});
                postDeadline_ = Clock::now() + std::chrono::seconds(4);
                Enter(AutoState::WaitingCombatStop, L"PRE OK → StopAutoFight");
            }
            return;
        }

        if (state_ == AutoState::WaitingCombatStop) {
            RequestProbeIfIdle();
            if (SnapshotFresh() && (snapshot_.validMask & ValidAutoFight) && !snapshot_.autoFight) {
                Enter(AutoState::Stopped, L"POST OK: AUTO đã dừng"); return;
            }
            if (Clock::now() > postDeadline_) {
                Enter(AutoState::SafePaused, L"POST fail: chưa xác minh AUTO dừng; không retry"); return;
            }
            return;
        }

        if (state_ == AutoState::SafePaused) {
            RequestProbeIfIdle();
            return; // Không tự action từ SAFE PAUSE.
        }
    }

private:
    bool SnapshotFresh() const { return lastSnapshot_.time_since_epoch().count() && Clock::now() - lastSnapshot_ < std::chrono::seconds(2); }
    bool SafeSnapshot() const {
        const std::uint32_t need = ValidMainThread | ValidMapReady | ValidAutoFight;
        return (snapshot_.validMask & need) == need;
    }
    void RequestProbeIfIdle() {
        if (actions_.Empty() && !bridge_.Busy() && Clock::now() >= nextProbe_) {
            actions_.Push({BridgeCommand::Probe, 0, L"Probe"});
            nextProbe_ = Clock::now() + std::chrono::milliseconds(500);
        }
    }
    void Enter(AutoState next, const std::wstring& why) {
        state_ = next; detail_ = why;
        if (next == AutoState::Faulted || next == AutoState::SafePaused) actions_.ClearPending();
        if (next == AutoState::Faulted) bridge_.Close();
    }

    GameProcess game_{}; BridgeClient bridge_; ActionQueue actions_;
    AutoState state_ = AutoState::Stopped; bool wanted_ = false; int stableScans_ = 0;
    std::uint64_t snapshotSeq_ = 0, lastStableSeq_ = 0;
    GameSnapshot snapshot_{}; std::wstring detail_ = L"Đã dừng";
    Clock::time_point lastSnapshot_{}; Clock::time_point nextProbe_{}; Clock::time_point postDeadline_{};
};

HWND g_main = nullptr, g_list = nullptr, g_log = nullptr;
std::map<DWORD, std::unique_ptr<Session>> g_sessions;

void Log(const std::wstring& text) {
    if (!g_log) return;
    int len = GetWindowTextLengthW(g_log);
    SendMessageW(g_log, EM_SETSEL, len, len);
    const std::wstring line = text + L"\r\n";
    SendMessageW(g_log, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
}

void RebuildList() {
    std::set<DWORD> checked;
    const int oldCount = ListView_GetItemCount(g_list);
    for (int i = 0; i < oldCount; ++i) {
        if (!ListView_GetCheckState(g_list, i)) continue;
        LVITEMW old{}; old.mask = LVIF_PARAM; old.iItem = i;
        if (ListView_GetItem(g_list, &old)) checked.insert(static_cast<DWORD>(old.lParam));
    }

    ListView_DeleteAllItems(g_list);
    int row = 0;
    for (const auto& [pid, session] : g_sessions) {
        LVITEMW item{}; item.mask = LVIF_TEXT | LVIF_PARAM; item.iItem = row;
        std::wstring pidText = std::to_wstring(pid); item.pszText = pidText.data(); item.lParam = pid;
        ListView_InsertItem(g_list, &item);
        const auto& s = session->Snapshot();
        std::wstring displayName = s.characterName[0] ?
            (std::wstring(s.characterName) + L" / Role " + std::to_wstring(s.roleId)) : session->Game().title;
        ListView_SetItemText(g_list, row, 1, displayName.data());
        ListView_SetItemText(g_list, row, 2, const_cast<wchar_t*>(StateName(session->State())));
        std::wstring map = (s.validMask & ValidMapId) ? std::to_wstring(s.mapId) : L"?";
        std::wstring pos = (s.validMask & ValidPosition) ? (std::to_wstring(s.x) + L"," + std::to_wstring(s.y)) : L"?";
        std::wstring bag = (s.validMask & ValidBag) ? std::to_wstring(s.freeBagSpace) : L"?";
        std::wstring mt = (s.validMask & ValidMainThread) ? L"UnitySync" : L"NO";
        std::wstring detail = session->Detail();
        ListView_SetItemText(g_list, row, 3, map.data());
        ListView_SetItemText(g_list, row, 4, pos.data());
        ListView_SetItemText(g_list, row, 5, bag.data());
        ListView_SetItemText(g_list, row, 6, mt.data());
        ListView_SetItemText(g_list, row, 7, detail.data());
        if (checked.count(pid)) ListView_SetCheckState(g_list, row, TRUE);
        ++row;
    }
}

std::vector<DWORD> CheckedPids() {
    std::vector<DWORD> out;
    const int n = ListView_GetItemCount(g_list);
    for (int i=0;i<n;++i) if (ListView_GetCheckState(g_list,i)) {
        LVITEMW it{}; it.mask=LVIF_PARAM; it.iItem=i; if (ListView_GetItem(g_list,&it)) out.push_back(static_cast<DWORD>(it.lParam));
    }
    return out;
}

void ScanGames() {
    auto found = FindGames();
    std::map<DWORD, std::unique_ptr<Session>> next;
    for (auto& game : found) {
        auto old = g_sessions.find(game.pid);
        if (old != g_sessions.end() && old->second->Game().window == game.window &&
            old->second->Game().threadId == game.threadId) {
            next[game.pid] = std::move(old->second);
            g_sessions.erase(old);
        } else {
            // Same PID can recreate its window/message thread during display/map transitions.
            // Never keep a hook/session bound to a stale HWND/TID.
            next[game.pid] = std::make_unique<Session>(game);
        }
    }
    g_sessions = std::move(next); RebuildList();
    Log(L"Quét thấy " + std::to_wstring(g_sessions.size()) + L" client GameAssembly.dll");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            g_list = CreateWindowW(WC_LISTVIEWW, L"", WS_CHILD|WS_VISIBLE|LVS_REPORT|LVS_SINGLESEL,
                                  12,12,856,260,hwnd,reinterpret_cast<HMENU>(IDC_LIST),nullptr,nullptr);
            ListView_SetExtendedListViewStyle(g_list, LVS_EX_FULLROWSELECT|LVS_EX_GRIDLINES|LVS_EX_CHECKBOXES);
            const wchar_t* names[] = {L"PID",L"Nhân vật",L"Trạng thái",L"Map",L"X,Y",L"Bag",L"MainThread",L"Chi tiết"};
            int widths[] = {80,220,170,70,110,70,110,360};
            for (int i=0;i<8;++i) { LVCOLUMNW c{}; c.mask=LVCF_TEXT|LVCF_WIDTH; c.pszText=const_cast<wchar_t*>(names[i]); c.cx=widths[i]; ListView_InsertColumn(g_list,i,&c); }
            CreateWindowW(L"BUTTON",L"Quét client",WS_CHILD|WS_VISIBLE,12,284,120,34,hwnd,reinterpret_cast<HMENU>(IDC_SCAN),nullptr,nullptr);
            CreateWindowW(L"BUTTON",L"Bắt đầu",WS_CHILD|WS_VISIBLE,144,284,120,34,hwnd,reinterpret_cast<HMENU>(IDC_START),nullptr,nullptr);
            CreateWindowW(L"BUTTON",L"Dừng",WS_CHILD|WS_VISIBLE,276,284,120,34,hwnd,reinterpret_cast<HMENU>(IDC_STOP),nullptr,nullptr);
            CreateWindowW(L"STATIC",L"NewCore: mỗi client 1 FSM + 1 ActionQueue; action chỉ chạy khi bridge chứng minh Unity main thread.",WS_CHILD|WS_VISIBLE,416,292,450,26,hwnd,nullptr,nullptr,nullptr);
            g_log = CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD|WS_VISIBLE|WS_VSCROLL|ES_MULTILINE|ES_READONLY|ES_AUTOVSCROLL,
                                    12,332,856,170,hwnd,reinterpret_cast<HMENU>(IDC_LOG),nullptr,nullptr);
            SetTimer(hwnd,TIMER_ID,250,nullptr); ScanGames(); return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wp)==IDC_SCAN) { ScanGames(); return 0; }
            if (LOWORD(wp)==IDC_START) {
                for (DWORD pid: CheckedPids()) if (auto it=g_sessions.find(pid); it!=g_sessions.end()) it->second->StartWanted();
                Log(L"Bắt đầu các client đã tick"); return 0;
            }
            if (LOWORD(wp)==IDC_STOP) {
                for (DWORD pid: CheckedPids()) if (auto it=g_sessions.find(pid); it!=g_sessions.end()) it->second->StopWanted();
                Log(L"Dừng các client đã tick"); return 0;
            }
            break;
        case WM_TIMER:
            if (wp==TIMER_ID) { for (auto& entry : g_sessions) entry.second->Tick(); RebuildList(); return 0; }
            break;
        case WM_SIZE: {
            int w=LOWORD(lp),h=HIWORD(lp); MoveWindow(g_list,12,12,w-24,260,TRUE); MoveWindow(g_log,12,332,w-24,std::max(80,h-344),TRUE); return 0;
        }
        case WM_DESTROY: KillTimer(hwnd,TIMER_ID); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd,msg,wp,lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_LISTVIEW_CLASSES}; InitCommonControlsEx(&icc);
    WNDCLASSEXW wc{}; wc.cbSize=sizeof(wc); wc.lpfnWndProc=WndProc; wc.hInstance=instance; wc.lpszClassName=L"ThanLongNewCoreWindow";
    wc.hCursor=LoadCursor(nullptr,IDC_ARROW); wc.hbrBackground=reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
    wc.hIcon=LoadIcon(instance,MAKEINTRESOURCE(1)); RegisterClassExW(&wc);
    g_main=CreateWindowExW(0,wc.lpszClassName,kTitle,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,900,560,nullptr,nullptr,instance,nullptr);
    if(!g_main) return 1; ShowWindow(g_main,show); UpdateWindow(g_main);
    MSG msg{}; while(GetMessageW(&msg,nullptr,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);} return static_cast<int>(msg.wParam);
}
