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
                L"PID", L"Nhân vật", L"Trạng thái", L"Bridge", L"MainThread",
                L"RoleID", L"Map", L"X,Y", L"HP", L"Bag", L"Dead", L"Riding",
                L"Moving", L"AutoFight", L"AutoPath", L"MapReady", L"WaitMap", L"Chi tiết"
            };
            int widths[] = {75, 150, 140, 80, 105, 80, 55, 100, 120, 60, 60, 65, 65, 75, 75, 80, 75, 420};
            for (int i = 0; i < 18; ++i) {
                LVCOLUMNW c{};
                c.mask = LVCF_TEXT | LVCF_WIDTH;
                c.pszText = const_cast<wchar_t*>(names[i]);
                c.cx = widths[i];
                ListView_InsertColumn(g_list, i, &c);
            }

            CreateWindowW(L"BUTTON", L"Quét client", WS_CHILD | WS_VISIBLE,
                          12, 284, 120, 34, hwnd, reinterpret_cast<HMENU>(IDC_SCAN), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Kiểm tra nền + Scanner", WS_CHILD | WS_VISIBLE,
                          144, 284, 140, 34, hwnd, reinterpret_cast<HMENU>(IDC_VALIDATE), nullptr, nullptr);
            CreateWindowW(L"BUTTON", L"Ngắt bridge", WS_CHILD | WS_VISIBLE,
                          296, 284, 130, 34, hwnd, reinterpret_cast<HMENU>(IDC_DISCONNECT), nullptr, nullptr);
            CreateWindowW(L"STATIC",
                          L"Phase 4: CONTINUOUS READ-ONLY SCANNER 500 ms. Donor guard map-transition; action game vẫn khóa.",
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
                Log(L"Bắt đầu foundation + main-thread proof + continuous scanner read-only cho các client đã tick");
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
