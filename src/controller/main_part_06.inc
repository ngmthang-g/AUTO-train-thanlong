    BridgeCommand pending_ = BridgeCommand::None;
    FoundationSnapshot snapshot_{};
    GameSnapshot gameSnapshot_{};
    GameSnapshot previousGameSnapshot_{};
    bool haveGameSnapshot_ = false;
    int scannerFailures_ = 0;
    bool transitionLatched_ = false;
    int postTransitionStableScans_ = 0;
    Clock::time_point nextSnapshotDue_{};
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

        const auto& f = session->Snapshot();
        const auto& g = session->GameState();
        std::wstring title = (g.validMask & ValidCharacterName) && g.characterName[0]
            ? std::wstring(g.characterName) : session->Game().title;
        std::wstring bridge = session->BridgeLoaded() ? L"LOADED" : L"NO";
        std::wstring mainThread = (f.validMask & ValidUnityMainThread)
            ? (L"PROVEN M" + std::to_wstring(f.currentManagedThreadId)) : L"LOCKED";
        std::wstring role = (g.validMask & ValidRoleIdentity) ? std::to_wstring(g.roleID) : L"?";
        std::wstring map = (g.validMask & ValidMapId) ? std::to_wstring(g.mapID) : L"?";
        std::wstring pos = (g.validMask & ValidPosition)
            ? (std::to_wstring(g.x) + L"," + std::to_wstring(g.y)) : L"?";
        std::wstring hp = (g.validMask & ValidVitals)
            ? (std::to_wstring(g.hp) + L"/" + std::to_wstring(g.maxHP)) : L"?";
        std::wstring bag = (g.validMask & ValidBagSpace) ? std::to_wstring(g.freeBagSpace) : L"?";
        std::wstring dead = (g.validMask & ValidLifeState) ? (g.dead ? L"YES" : L"NO") : L"?";
        std::wstring riding = (g.validMask & ValidRideState) ? (g.riding ? L"YES" : L"NO") : L"?";
        std::wstring moving = (g.validMask & ValidMovingState) ? (g.moving ? L"YES" : L"NO") : L"?";
        std::wstring autoFight = (g.validMask & ValidAutoFightState) ? (g.autoFight ? L"ON" : L"OFF") : L"?";
        std::wstring autoPath = (g.validMask & ValidAutoPathState) ? (g.autoPathing ? L"ON" : L"OFF") : L"?";
        std::wstring mapReady = (g.validMask & ValidMapReadyState) ? (g.mapReady ? L"YES" : L"NO") : L"?";
        std::wstring waitMap = (g.validMask & ValidMapTransitionState) ? (g.waitingChangeMap ? L"YES" : L"NO") : L"?";
        std::wstring detail = session->Detail();

        ListView_SetItemText(g_list, row, 1, title.data());
        ListView_SetItemText(g_list, row, 2, const_cast<wchar_t*>(StateName(session->State())));
        ListView_SetItemText(g_list, row, 3, bridge.data());
        ListView_SetItemText(g_list, row, 4, mainThread.data());
        ListView_SetItemText(g_list, row, 5, role.data());
        ListView_SetItemText(g_list, row, 6, map.data());
        ListView_SetItemText(g_list, row, 7, pos.data());
        ListView_SetItemText(g_list, row, 8, hp.data());
        ListView_SetItemText(g_list, row, 9, bag.data());
        ListView_SetItemText(g_list, row, 10, dead.data());
        ListView_SetItemText(g_list, row, 11, riding.data());
        ListView_SetItemText(g_list, row, 12, moving.data());
        ListView_SetItemText(g_list, row, 13, autoFight.data());
        ListView_SetItemText(g_list, row, 14, autoPath.data());
        ListView_SetItemText(g_list, row, 15, mapReady.data());
        ListView_SetItemText(g_list, row, 16, waitMap.data());
        ListView_SetItemText(g_list, row, 17, detail.data());
        if (checked.count(pid)) ListView_SetCheckState(g_list, row, TRUE);
        ++row;
    }
}

std::vector<DWORD> CheckedPids() {
    std::vector<DWORD> out;
    const int n = ListView_GetItemCount(g_list);
    for (int i = 0; i < n; ++i) {
