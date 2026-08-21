#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <string>
#include <vector>
#include <algorithm>

#pragma comment(lib, "comctl32.lib")

namespace {

constexpr wchar_t kTitle[] = L"Thần Long Hidden Auto Chat - TEST";
constexpr wchar_t kGameModule[] = L"GameAssembly.dll";

constexpr int IDC_GAME_LIST = 1001;
constexpr int IDC_SCAN = 1002;
constexpr int IDC_MESSAGE = 1003;
constexpr int IDC_INTERVAL = 1004;
constexpr int IDC_REPEAT = 1005;
constexpr int IDC_START = 1006;
constexpr int IDC_STOP = 1007;
constexpr int IDC_STATUS = 1008;
constexpr UINT_PTR TIMER_SEND = 1;

struct GameWindow {
    DWORD pid{};
    DWORD threadId{};
    HWND hwnd{};
    std::wstring title;
};

HWND g_main{};
HWND g_list{};
HWND g_scan{};
HWND g_message{};
HWND g_interval{};
HWND g_repeat{};
HWND g_start{};
HWND g_stop{};
HWND g_status{};
std::vector<GameWindow> g_games;
GameWindow g_target{};
int g_remaining = 0;
bool g_running = false;

bool HasModule(DWORD pid, const wchar_t* moduleName) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W me{};
    me.dwSize = sizeof(me);
    bool found = false;
    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(me.szModule, moduleName) == 0) {
                found = true;
                break;
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return found;
}

BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lp) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    const int len = GetWindowTextLengthW(hwnd);
    if (len <= 0) return TRUE;

    DWORD pid = 0;
    const DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    if (!pid || !tid || !HasModule(pid, kGameModule)) return TRUE;

    auto* out = reinterpret_cast<std::vector<GameWindow>*>(lp);
    for (const auto& item : *out) if (item.pid == pid) return TRUE;

    std::wstring title(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(hwnd, title.data(), static_cast<int>(title.size()));
    title.resize(wcslen(title.c_str()));
    out->push_back({pid, tid, hwnd, title});
    return TRUE;
}

std::vector<GameWindow> FindGames() {
    std::vector<GameWindow> out;
    EnumWindows(EnumProc, reinterpret_cast<LPARAM>(&out));
    std::sort(out.begin(), out.end(), [](const auto& a, const auto& b) { return a.pid < b.pid; });
    return out;
}

void SetStatus(const std::wstring& text) {
    SetWindowTextW(g_status, text.c_str());
}

std::wstring GetText(HWND h) {
    const int len = GetWindowTextLengthW(h);
    std::wstring text(static_cast<size_t>(len) + 1, L'\0');
    GetWindowTextW(h, text.data(), static_cast<int>(text.size()));
    text.resize(wcslen(text.c_str()));
    return text;
}

int GetPositiveInt(HWND h, int fallback, int minValue, int maxValue) {
    wchar_t buf[64]{};
    GetWindowTextW(h, buf, _countof(buf));
    wchar_t* end = nullptr;
    const long v = wcstol(buf, &end, 10);
    if (end == buf || v < minValue || v > maxValue) return fallback;
    return static_cast<int>(v);
}

void ScanGames() {
    g_games = FindGames();
    SendMessageW(g_list, CB_RESETCONTENT, 0, 0);
    for (const auto& game : g_games) {
        const std::wstring line = L"PID " + std::to_wstring(game.pid) + L"  |  " + game.title;
        SendMessageW(g_list, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
    if (!g_games.empty()) {
        SendMessageW(g_list, CB_SETCURSEL, 0, 0);
        SetStatus(L"Đã quét thấy " + std::to_wstring(g_games.size()) + L" cửa sổ game.");
    } else {
        SetStatus(L"Không tìm thấy cửa sổ có GameAssembly.dll.");
    }
}

bool PostKey(HWND hwnd, UINT vk) {
    if (!IsWindow(hwnd)) return false;
    const UINT scanCode = MapVirtualKeyW(vk, MAPVK_VK_TO_VSC);
    const LPARAM down = 1 | (static_cast<LPARAM>(scanCode) << 16);
    const LPARAM up = down | (static_cast<LPARAM>(1) << 30) | (static_cast<LPARAM>(1) << 31);
    return PostMessageW(hwnd, WM_KEYDOWN, vk, down) && PostMessageW(hwnd, WM_KEYUP, vk, up);
}

bool PostUnicodeText(HWND hwnd, const std::wstring& text) {
    if (!IsWindow(hwnd)) return false;
    for (wchar_t ch : text) {
        if (ch == L'\r' || ch == L'\n') continue;
        if (!PostMessageW(hwnd, WM_CHAR, static_cast<WPARAM>(ch), 1)) return false;
    }
    return true;
}

bool SendHiddenChatOnce(const GameWindow& game, const std::wstring& message) {
    if (!IsWindow(game.hwnd) || message.empty()) return false;

    // Experimental background path only: no SetForegroundWindow, SendInput,
    // cursor movement or clipboard. Ordering is preserved by the target thread queue.
    if (!PostKey(game.hwnd, VK_RETURN)) return false;
    if (!PostUnicodeText(game.hwnd, message)) return false;
    if (!PostKey(game.hwnd, VK_RETURN)) return false;
    return true;
}

void StopRun(const wchar_t* reason) {
    KillTimer(g_main, TIMER_SEND);
    g_running = false;
    EnableWindow(g_start, TRUE);
    EnableWindow(g_scan, TRUE);
    EnableWindow(g_list, TRUE);
    EnableWindow(g_stop, FALSE);
    if (reason) SetStatus(reason);
}

void StartRun() {
    if (g_running) return;
    const int selected = static_cast<int>(SendMessageW(g_list, CB_GETCURSEL, 0, 0));
    if (selected < 0 || selected >= static_cast<int>(g_games.size())) {
        MessageBoxW(g_main, L"Hãy quét và chọn một cửa sổ game.", kTitle, MB_ICONWARNING);
        return;
    }
    const std::wstring message = GetText(g_message);
    if (message.empty()) {
        MessageBoxW(g_main, L"Nội dung chat đang trống.", kTitle, MB_ICONWARNING);
        return;
    }

    g_target = g_games[selected];
    const int intervalSec = GetPositiveInt(g_interval, 10, 1, 86400);
    g_remaining = GetPositiveInt(g_repeat, 1, 1, 1000000);

    g_running = true;
    EnableWindow(g_start, FALSE);
    EnableWindow(g_scan, FALSE);
    EnableWindow(g_list, FALSE);
    EnableWindow(g_stop, TRUE);

    if (!SendHiddenChatOnce(g_target, message)) {
        StopRun(L"Gửi lần đầu thất bại; cửa sổ game có thể đã đóng.");
        return;
    }
    --g_remaining;
    SetStatus(L"Đã gửi 1 lần. Còn " + std::to_wstring(g_remaining) + L" lần.");
    if (g_remaining <= 0) {
        StopRun(L"Hoàn tất số lần lặp.");
        return;
    }
    SetTimer(g_main, TIMER_SEND, static_cast<UINT>(intervalSec * 1000), nullptr);
}

void OnTimer() {
    if (!g_running) return;
    if (!IsWindow(g_target.hwnd)) {
        StopRun(L"Cửa sổ game đã đóng; đã dừng.");
        return;
    }
    const std::wstring message = GetText(g_message);
    if (message.empty()) {
        StopRun(L"Nội dung chat bị xóa; đã dừng.");
        return;
    }
    if (!SendHiddenChatOnce(g_target, message)) {
        StopRun(L"Gửi chat thất bại; đã dừng để tránh spam lỗi.");
        return;
    }
    --g_remaining;
    if (g_remaining <= 0) {
        StopRun(L"Hoàn tất số lần lặp.");
    } else {
        SetStatus(L"Đã gửi. Còn " + std::to_wstring(g_remaining) + L" lần.");
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE: {
            HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
            CreateWindowW(L"STATIC", L"Cửa sổ game", WS_CHILD | WS_VISIBLE, 14, 10, 120, 18, hwnd, nullptr, nullptr, nullptr);
            g_list = CreateWindowW(WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 14, 30, 470, 260, hwnd, reinterpret_cast<HMENU>(IDC_GAME_LIST), nullptr, nullptr);
            g_scan = CreateWindowW(L"BUTTON", L"Quét lại", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 500, 30, 90, 28, hwnd, reinterpret_cast<HMENU>(IDC_SCAN), nullptr, nullptr);

            CreateWindowW(L"STATIC", L"Nội dung chat", WS_CHILD | WS_VISIBLE, 14, 68, 120, 18, hwnd, nullptr, nullptr, nullptr);
            g_message = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL, 14, 88, 576, 90, hwnd, reinterpret_cast<HMENU>(IDC_MESSAGE), nullptr, nullptr);

            CreateWindowW(L"STATIC", L"Khoảng lặp (giây)", WS_CHILD | WS_VISIBLE, 14, 190, 120, 18, hwnd, nullptr, nullptr, nullptr);
            CreateWindowW(L"STATIC", L"Số lần", WS_CHILD | WS_VISIBLE, 154, 190, 90, 18, hwnd, nullptr, nullptr, nullptr);
            g_interval = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"10", WS_CHILD | WS_VISIBLE | ES_NUMBER, 14, 210, 110, 26, hwnd, reinterpret_cast<HMENU>(IDC_INTERVAL), nullptr, nullptr);
            g_repeat = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"5", WS_CHILD | WS_VISIBLE | ES_NUMBER, 154, 210, 110, 26, hwnd, reinterpret_cast<HMENU>(IDC_REPEAT), nullptr, nullptr);
            g_start = CreateWindowW(L"BUTTON", L"Bắt đầu", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 294, 208, 90, 30, hwnd, reinterpret_cast<HMENU>(IDC_START), nullptr, nullptr);
            g_stop = CreateWindowW(L"BUTTON", L"Dừng", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 394, 208, 90, 30, hwnd, reinterpret_cast<HMENU>(IDC_STOP), nullptr, nullptr);
            EnableWindow(g_stop, FALSE);

            g_status = CreateWindowW(L"STATIC", L"Sẵn sàng.", WS_CHILD | WS_VISIBLE, 14, 252, 576, 46, hwnd, reinterpret_cast<HMENU>(IDC_STATUS), nullptr, nullptr);

            for (HWND h : {g_list, g_scan, g_message, g_interval, g_repeat, g_start, g_stop, g_status}) {
                SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            }
            ScanGames();
            return 0;
        }
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case IDC_SCAN: ScanGames(); return 0;
                case IDC_START: StartRun(); return 0;
                case IDC_STOP: StopRun(L"Đã dừng thủ công."); return 0;
            }
            break;
        case WM_TIMER:
            if (wp == TIMER_SEND) { OnTimer(); return 0; }
            break;
        case WM_DESTROY:
            KillTimer(hwnd, TIMER_SEND);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    INITCOMMONCONTROLSEX icc{sizeof(icc), ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    WNDCLASSEXW wc{sizeof(wc)};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = L"ThanLongHiddenAutoChatTest";
    if (!RegisterClassExW(&wc)) return 1;

    g_main = CreateWindowExW(0, wc.lpszClassName, kTitle,
                             WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                             CW_USEDEFAULT, CW_USEDEFAULT, 620, 350,
                             nullptr, nullptr, instance, nullptr);
    if (!g_main) return 2;
    ShowWindow(g_main, show);
    UpdateWindow(g_main);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
