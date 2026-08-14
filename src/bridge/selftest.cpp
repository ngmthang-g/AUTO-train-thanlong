#include <windows.h>
#include <cstdio>

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) return 2;
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    HMODULE m = LoadLibraryW(argv[1]);
    if (!m) {
        const DWORD e = GetLastError();
        wchar_t msg[512]{};
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, e,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, _countof(msg), nullptr);
        std::fwprintf(stderr, L"BRIDGE SELFTEST FAIL: LoadLibrary Win32=%lu: %ls\n", e, msg);
        return 10;
    }
    FARPROC p = GetProcAddress(m, "TlGetMessageHook");
    if (!p) {
        std::fwprintf(stderr, L"BRIDGE SELFTEST FAIL: missing TlGetMessageHook export\n");
        FreeLibrary(m);
        return 11;
    }
    std::fwprintf(stdout, L"BRIDGE SELFTEST PASS\n");
    FreeLibrary(m);
    return 0;
}
