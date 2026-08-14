#include <windows.h>
#include <stdio.h>

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        fprintf(stderr, "BRIDGE SELFTEST FAIL: missing DLL path\n");
        return 2;
    }

    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    SetLastError(ERROR_SUCCESS);
    HMODULE module = LoadLibraryW(argv[1]);
    if (!module) {
        const DWORD error = GetLastError();
        fprintf(stderr, "BRIDGE SELFTEST FAIL: LoadLibrary Win32=%lu\n",
                static_cast<unsigned long>(error));
        return 10;
    }

    FARPROC hookExport = GetProcAddress(module, "TlGetMessageHook");
    if (!hookExport) {
        const DWORD error = GetLastError();
        fprintf(stderr, "BRIDGE SELFTEST FAIL: missing TlGetMessageHook export; Win32=%lu\n",
                static_cast<unsigned long>(error));
        FreeLibrary(module);
        return 11;
    }

    fprintf(stdout, "BRIDGE SELFTEST PASS\n");
    FreeLibrary(module);
    return 0;
}
