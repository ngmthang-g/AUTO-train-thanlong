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
