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

constexpr wchar_t kTitle[] = L"Thần Long Auto - NewCore v1.0.10 AutoPath + Identity Guard";
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
