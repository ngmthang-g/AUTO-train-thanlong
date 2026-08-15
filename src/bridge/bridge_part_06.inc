    const MethodInfo* method = FindMethodInHierarchy(klass, name, 0);
    if (!method || !IsStaticMethod(method)) {
        ClearText(detail, cap); AppendText(detail, cap, L"Không resolve được static query: "); AppendAscii(detail, cap, name);
        return false;
    }
    return ReadScalarGetter(klass, name, nullptr, value, detail, cap);
}

bool CopyManagedString(Il2CppString* value, wchar_t* out, std::size_t cap) {
    if (!out || cap == 0) return false;
    out[0] = L'\0';
    if (!value) return false;
    const std::int32_t length = g_api.string_length(value);
    const wchar_t* chars = g_api.string_chars(value);
    if (length < 0 || length > 4096 || !chars) return false;
    std::size_t n = static_cast<std::size_t>(length);
    if (n + 1 > cap) n = cap - 1;
    for (std::size_t i = 0; i < n; ++i) out[i] = chars[i];
    out[n] = L'\0';
    return true;
}

bool ReadGameSnapshot(FoundationSnapshot& foundation, GameSnapshot& game,
                      wchar_t* detail, std::size_t cap) {
    wchar_t proof[512]{};
    if (!ProveUnityMainThread(foundation, proof, _countof(proof))) {
        CopyText(detail, cap, proof);
        return false;
    }

    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) { CopyText(detail, cap, L"Không resolve được Assembly-CSharp cho snapshot"); return false; }

    // Donor v0.9.0 lesson retained semantically: map-transition state must be read FIRST.
    // While Unity is rebuilding a map, do not touch LeaderRoleData/AutoPath objects that may be
    // absent or in flux. This is still metadata-based and read-only; no donor RVA is executed.
    Il2CppClass* gameApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    if (!gameApi) { CopyText(detail, cap, L"Thiếu FGStudio.LuaSystem.API.LuaSystemAPI_Game"); return false; }
    Il2CppClass* sessionData = g_api.class_from_name(image, "FGStudio.Game.Logic", "SessionData");
    if (!sessionData) { CopyText(detail, cap, L"Thiếu FGStudio.Game.Logic.SessionData"); return false; }

    std::int32_t mapReady = 0, waiting = 0;
    if (!ReadStaticScalar(gameApi, "IsMapReady", mapReady, detail, cap)) return false;
    game.mapReady = mapReady ? 1 : 0;
    game.validMask |= ValidMapReadyState;
    if (!ReadStaticScalar(sessionData, "get_WaitingChangeMap", waiting, detail, cap)) return false;
    game.waitingChangeMap = waiting ? 1 : 0;
    game.validMask |= ValidMapTransitionState;

    static std::uint32_t sequence = 0;
    game.sequence = ++sequence;
    if (!game.mapReady || game.waitingChangeMap) {
        ClearText(detail, cap);
        AppendText(detail, cap, L"SNAPSHOT TRANSITION seq="); AppendUInt(detail, cap, game.sequence);
        AppendText(detail, cap, L" mapReady="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.mapReady));
        AppendText(detail, cap, L" waitingChangeMap="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.waitingChangeMap));
        AppendText(detail, cap, L". Donor guard: bỏ qua Leader/AutoPath trong lúc đổi map. READ-ONLY.");
        return true;
    }

    Il2CppClass* sharedClass = g_api.class_from_name(image, "FGStudio.LuaSystem", "LuaSystemSharedData");
    if (!sharedClass) { CopyText(detail, cap, L"Thiếu FGStudio.LuaSystem.LuaSystemSharedData"); return false; }
    const MethodInfo* getLeader = FindMethodInHierarchy(sharedClass, "get_LeaderRoleData", 0);
    if (!getLeader || !IsStaticMethod(getLeader)) {
        CopyText(detail, cap, L"Thiếu static LuaSystemSharedData.get_LeaderRoleData()"); return false;
    }
    Il2CppObject* leader = nullptr;
    if (!InvokeObject(getLeader, nullptr, leader, detail, cap)) return false;
    if (!leader) { CopyText(detail, cap, L"LeaderRoleData=null dù map đã ready"); return false; }
    Il2CppClass* leaderClass = g_api.object_get_class(leader);
    if (!leaderClass) { CopyText(detail, cap, L"Không lấy được class của LeaderRoleData"); return false; }

    std::int32_t roleID = 0, mapID = 0, hp = 0, maxHP = 0, dead = 0, riding = 0;
    if (!ReadScalarGetter(leaderClass, "get_RoleID", leader, roleID, detail, cap) || roleID <= 0) return false;
    game.roleID = roleID; game.validMask |= ValidRoleIdentity;
    if (!ReadScalarGetter(leaderClass, "get_MapID", leader, mapID, detail, cap) || mapID <= 0) return false;
    game.mapID = mapID; game.validMask |= ValidMapId;
    if (!ReadScalarGetter(leaderClass, "get_HP", leader, hp, detail, cap) || hp < 0) return false;
    if (!ReadScalarGetter(leaderClass, "get_MaxHP", leader, maxHP, detail, cap) || maxHP <= 0) return false;
    game.hp = hp; game.maxHP = maxHP; game.validMask |= ValidVitals;
    if (!ReadScalarGetter(leaderClass, "get_IsDeath", leader, dead, detail, cap)) return false;
