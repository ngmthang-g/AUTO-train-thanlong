                    if (it != g_sessions.end()) it->second->ValidateWanted();
                }
                Log(L"Bắt đầu foundation + scanner + SafetyGuard/ActionQueue/FSM dry-run cho các client đã tick");
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
