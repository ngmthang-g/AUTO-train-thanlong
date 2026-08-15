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
                          L"v1.2.0: Revive/Đầu thai ONLY. AutoFight/NPC/Sell vẫn khóa. Queue MAX=1 + POST thật.",
                          WS_CHILD | WS_VISIBLE, 448, 292, 720, 26, hwnd, nullptr, nullptr, nullptr);
            g_log = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_VSCROLL |
                                    ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
                                    12, 332, 1160, 250, hwnd, reinterpret_cast<HMENU>(IDC_LOG), nullptr, nullptr);

            const IntegratedSelfTestResult selfTest = RunIntegratedControlSelfTest();
            const ReviveSelfTestResult reviveTest = RunReviveControlSelfTest();
            g_controlSelfTestPass = selfTest.Pass() && reviveTest.Pass();
            Log(std::wstring(L"CONTROL SELF TEST ") +
                (selfTest.Pass() ? L"PASS" : L"FAIL") +
                L" • " + std::to_wstring(selfTest.passed) + L"/" +
                std::to_wstring(selfTest.total) + L" • failed=" +
                std::to_wstring(selfTest.failed) +
                L" • FSM/SafetyGuard/Queue/Watchdog/HarmlessEnvelope");
            Log(std::wstring(L"REVIVE CONTROL SELF TEST ") +
                (reviveTest.Pass() ? L"PASS" : L"FAIL") +
                L" • " + std::to_wstring(reviveTest.passed) + L"/" +
                std::to_wstring(reviveTest.total) + L" • failed=" +
                std::to_wstring(reviveTest.failed) +
                L" • Revive exclusive gate/dispatch ACK/map-change POST");

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
                if (!g_controlSelfTestPass) {
                    Log(L"CONTROL/REVIVE SELF TEST FAIL • foundation bị khóa fail-closed");
                    return 0;
                }
                for (DWORD pid : CheckedPids()) {
                    auto it = g_sessions.find(pid);
