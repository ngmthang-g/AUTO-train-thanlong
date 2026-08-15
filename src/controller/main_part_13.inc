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
