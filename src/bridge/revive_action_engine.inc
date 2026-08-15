// NewCore v1.2.0 - first real gameplay mutation: Revive / Đầu thai only.
// Runs only from the proven WH_GETMESSAGE / Unity-main-thread callback.
// Every invocation re-resolves UIObject.instances and the selected UIButton; no UI pointer is cached.

using FieldStaticGetValueFn = void (__cdecl*)(FieldInfo*, void*);

wchar_t FoldVietnameseReviveChar(wchar_t ch) {
    if (ch >= L'A' && ch <= L'Z') return static_cast<wchar_t>(ch - L'A' + L'a');
    if (ch >= L'a' && ch <= L'z') return ch;
    if (ch >= L'0' && ch <= L'9') return ch;
    switch (static_cast<unsigned int>(ch)) {
        case 0x0110: case 0x0111: return L'd';
        case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x00C4:
        case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x00E4:
        case 0x0102: case 0x0103:
        case 0x1EA0: case 0x1EA1: case 0x1EA2: case 0x1EA3: case 0x1EA4: case 0x1EA5:
        case 0x1EA6: case 0x1EA7: case 0x1EA8: case 0x1EA9: case 0x1EAA: case 0x1EAB:
        case 0x1EAC: case 0x1EAD: case 0x1EAE: case 0x1EAF: case 0x1EB0: case 0x1EB1:
        case 0x1EB2: case 0x1EB3: case 0x1EB4: case 0x1EB5: case 0x1EB6: case 0x1EB7:
            return L'a';
        case 0x00C8: case 0x00C9: case 0x00CA: case 0x00CB:
        case 0x00E8: case 0x00E9: case 0x00EA: case 0x00EB:
        case 0x1EB8: case 0x1EB9: case 0x1EBA: case 0x1EBB: case 0x1EBC: case 0x1EBD:
        case 0x1EBE: case 0x1EBF: case 0x1EC0: case 0x1EC1: case 0x1EC2: case 0x1EC3:
        case 0x1EC4: case 0x1EC5: case 0x1EC6: case 0x1EC7:
            return L'e';
        case 0x00CC: case 0x00CD: case 0x00CE: case 0x00CF:
        case 0x00EC: case 0x00ED: case 0x00EE: case 0x00EF:
        case 0x1EC8: case 0x1EC9: case 0x0128: case 0x0129: case 0x1ECA: case 0x1ECB:
            return L'i';
        case 0x00D2: case 0x00D3: case 0x00D4: case 0x00D5: case 0x00D6:
        case 0x00F2: case 0x00F3: case 0x00F4: case 0x00F5: case 0x00F6:
        case 0x01A0: case 0x01A1:
        case 0x1ECC: case 0x1ECD: case 0x1ECE: case 0x1ECF: case 0x1ED0: case 0x1ED1:
        case 0x1ED2: case 0x1ED3: case 0x1ED4: case 0x1ED5: case 0x1ED6: case 0x1ED7:
        case 0x1ED8: case 0x1ED9: case 0x1EDA: case 0x1EDB: case 0x1EDC: case 0x1EDD:
        case 0x1EDE: case 0x1EDF: case 0x1EE0: case 0x1EE1: case 0x1EE2: case 0x1EE3:
            return L'o';
        case 0x00D9: case 0x00DA: case 0x00DB: case 0x00DC:
        case 0x00F9: case 0x00FA: case 0x00FB: case 0x00FC:
        case 0x01AF: case 0x01B0:
        case 0x1EE4: case 0x1EE5: case 0x1EE6: case 0x1EE7: case 0x1EE8: case 0x1EE9:
        case 0x1EEA: case 0x1EEB: case 0x1EEC: case 0x1EED: case 0x1EEE: case 0x1EEF:
        case 0x1EF0: case 0x1EF1:
            return L'u';
        case 0x00DD: case 0x00FD: case 0x0178:
        case 0x1EF2: case 0x1EF3: case 0x1EF4: case 0x1EF5: case 0x1EF6: case 0x1EF7:
        case 0x1EF8: case 0x1EF9:
            return L'y';
        default: return 0;
    }
}

void CompactReviveText(const wchar_t* input, wchar_t* output, std::size_t cap) {
    if (!output || cap == 0) return;
    output[0] = L'\0';
    if (!input) return;
    std::size_t used = 0;
    for (std::size_t i = 0; input[i] && used + 1 < cap; ++i) {
        const wchar_t folded = FoldVietnameseReviveChar(input[i]);
        if (!folded) continue;
        output[used++] = folded;
    }
    output[used] = L'\0';
}

bool WideEqualsLiteral(const wchar_t* a, const wchar_t* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (*a != *b) return false;
        ++a; ++b;
    }
    return *a == *b;
}

bool WideContainsLiteral(const wchar_t* text, const wchar_t* needle) {
    if (!text || !needle || !*needle) return false;
    for (std::size_t i = 0; text[i]; ++i) {
        std::size_t j = 0;
        while (needle[j] && text[i + j] == needle[j]) ++j;
        if (!needle[j]) return true;
    }
    return false;
}

bool ClassIsOrDerivesFrom(Il2CppClass* actual, Il2CppClass* target) {
    if (!actual || !target) return false;
    for (Il2CppClass* current = actual; current; current = g_api.class_get_parent(current)) {
        if (current == target) return true;
    }
    return false;
}

bool InvokeReadOnlyString(const MethodInfo* method, void* instance,
                          wchar_t* output, std::size_t outputCap,
                          wchar_t* detail, std::size_t detailCap) {
    if (!output || outputCap == 0) return false;
    output[0] = L'\0';
    Il2CppObject* object = nullptr;
    if (!InvokeObject(method, instance, object, detail, detailCap) || !object) return false;
    return CopyManagedString(reinterpret_cast<Il2CppString*>(object), output, outputCap);
}

bool InvokeReadOnlyBool(const MethodInfo* method, void* instance, bool& value,
                        wchar_t* detail, std::size_t cap) {
    value = false;
    std::int64_t raw = 0;
    if (!InvokeScalar(method, instance, raw, detail, cap)) return false;
    value = raw != 0;
    return true;
}

int ScoreReviveButton(const wchar_t* name, const wchar_t* text) {
    wchar_t nameKey[160]{};
    wchar_t textKey[160]{};
    CompactReviveText(name, nameKey, _countof(nameKey));
    CompactReviveText(text, textKey, _countof(textKey));

    if (WideEqualsLiteral(textKey, L"dauthai")) return 1000;
    if (WideEqualsLiteral(textKey, L"hoisinh")) return 930;
    if (WideEqualsLiteral(textKey, L"revive") || WideEqualsLiteral(textKey, L"relive") ||
        WideEqualsLiteral(textKey, L"respawn")) return 900;

    if (WideContainsLiteral(nameKey, L"dauthai")) return 820;
    if (WideContainsLiteral(nameKey, L"hoisinh")) return 790;
    if (WideContainsLiteral(nameKey, L"revive") || WideContainsLiteral(nameKey, L"relive") ||
        WideContainsLiteral(nameKey, L"respawn")) return 760;

    if (WideContainsLiteral(textKey, L"dauthai")) return 740;
    if (WideContainsLiteral(textKey, L"hoisinh")) return 710;
    return 0;
}

bool DescribeReviveButton(Il2CppObject* object, Il2CppClass* uiButtonClass,
                          const MethodInfo* activeMethod,
                          const MethodInfo* interactableMethod,
                          const MethodInfo* nameMethod,
                          const MethodInfo* textMethod,
                          int& score, wchar_t* label, std::size_t labelCap,
                          wchar_t* detail, std::size_t detailCap) {
    score = 0;
    if (!object || !uiButtonClass) return false;
    Il2CppClass* actual = g_api.object_get_class(object);
    if (!ClassIsOrDerivesFrom(actual, uiButtonClass)) return false;

    bool active = false, interactable = false;
    if (!InvokeReadOnlyBool(activeMethod, object, active, detail, detailCap) || !active) return false;
    if (!InvokeReadOnlyBool(interactableMethod, object, interactable, detail, detailCap) || !interactable) return false;

    wchar_t name[128]{};
    wchar_t text[128]{};
    if (nameMethod) InvokeReadOnlyString(nameMethod, object, name, _countof(name), detail, detailCap);
    if (!InvokeReadOnlyString(textMethod, object, text, _countof(text), detail, detailCap)) return false;

    score = ScoreReviveButton(name, text);
    if (score <= 0) return false;

    if (label && labelCap) {
        ClearText(label, labelCap);
        if (text[0]) CopyText(label, labelCap, text);
        else CopyText(label, labelCap, name);
    }
    return true;
}

bool FindFreshReviveButton(Il2CppObject*& selected, int& selectedScore,
                           wchar_t* selectedLabel, std::size_t selectedLabelCap,
                           const MethodInfo*& handleClick,
                           wchar_t* detail, std::size_t cap) {
    selected = nullptr;
    selectedScore = 0;
    handleClick = nullptr;
    if (selectedLabel && selectedLabelCap) selectedLabel[0] = L'\0';

    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: Assembly-CSharp unavailable"); return false; }
    Il2CppClass* uiObjectClass = g_api.class_from_name(image, "FGStudio.LuaSystem.Base", "UIObject");
    Il2CppClass* uiButtonClass = g_api.class_from_name(image, "FGStudio.LuaSystem.GUI", "UIButton");
    if (!uiObjectClass || !uiButtonClass) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIObject/UIButton type missing"); return false;
    }

    FieldInfo* instancesField = FindFieldInHierarchy(uiObjectClass, "instances");
    if (!instancesField) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIObject.instances missing"); return false; }

    FieldStaticGetValueFn fieldStaticGetValue = nullptr;
    if (!ResolveProcAddress(g_api.gameAssembly, "il2cpp_field_static_get_value", fieldStaticGetValue)) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: il2cpp_field_static_get_value unavailable"); return false;
    }

    Il2CppObject* instances = nullptr;
    fieldStaticGetValue(instancesField, &instances);
    if (!instances) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIObject.instances=null"); return false; }

    Il2CppClass* dictionaryClass = g_api.object_get_class(instances);
    if (!dictionaryClass) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: instances class unavailable"); return false; }
    const MethodInfo* getValues = FindMethodInHierarchy(dictionaryClass, "get_Values", 0);
    if (!getValues) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: instances.get_Values missing"); return false; }

    Il2CppObject* values = nullptr;
    if (!InvokeObject(getValues, instances, values, detail, cap) || !values) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: instances.Values unavailable"); return false;
    }
    Il2CppClass* valuesClass = g_api.object_get_class(values);
    if (!valuesClass) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: Values class unavailable"); return false; }
    const MethodInfo* getEnumerator = FindMethodInHierarchy(valuesClass, "GetEnumerator", 0);
    if (!getEnumerator) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: Values.GetEnumerator missing"); return false; }

    Il2CppObject* enumerator = nullptr;
    if (!InvokeObject(getEnumerator, values, enumerator, detail, cap) || !enumerator) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: Values enumerator unavailable"); return false;
    }
    Il2CppClass* enumeratorClass = g_api.object_get_class(enumerator);
    if (!enumeratorClass) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: enumerator class unavailable"); return false; }
    const MethodInfo* moveNext = FindMethodInHierarchy(enumeratorClass, "MoveNext", 0);
    const MethodInfo* getCurrent = FindMethodInHierarchy(enumeratorClass, "get_Current", 0);
    if (!moveNext || !getCurrent) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: enumerator MoveNext/Current missing"); return false;
    }

    const MethodInfo* activeMethod = FindMethodInHierarchy(uiObjectClass, "get_ActiveInHierarchy", 0);
    const MethodInfo* nameMethod = FindMethodInHierarchy(uiObjectClass, "get_Name", 0);
    const MethodInfo* interactableMethod = FindMethodInHierarchy(uiButtonClass, "get_Interactable", 0);
    const MethodInfo* textMethod = FindMethodInHierarchy(uiButtonClass, "get_Text", 0);
    handleClick = FindMethodInHierarchy(uiButtonClass, "HandleClickEvent", 0);
    if (!activeMethod || !interactableMethod || !textMethod || !handleClick || IsStaticMethod(handleClick)) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIButton getter/callback signature incomplete"); return false;
    }

    int tiesAtBest = 0;
    for (int i = 0; i < 32768; ++i) {
        std::int64_t hasNext = 0;
        if (!InvokeScalar(moveNext, enumerator, hasNext, detail, cap)) return false;
        if (!hasNext) break;

        Il2CppObject* object = nullptr;
        if (!InvokeObject(getCurrent, enumerator, object, detail, cap)) return false;
        if (!object) continue;

        int score = 0;
        wchar_t label[96]{};
        if (!DescribeReviveButton(object, uiButtonClass, activeMethod, interactableMethod,
                                  nameMethod, textMethod, score, label, _countof(label), detail, cap)) {
            continue;
        }
        if (score > selectedScore) {
            selected = object;
            selectedScore = score;
            tiesAtBest = 1;
            CopyText(selectedLabel, selectedLabelCap, label);
        } else if (score == selectedScore && object != selected) {
            ++tiesAtBest;
        }
    }

    if (!selected || selectedScore <= 0) {
        CopyText(detail, cap, L"REVIVE RESOLVE BLOCK: không thấy UIButton Đầu thai/Hồi sinh active+interactable");
        return false;
    }
    if (tiesAtBest != 1) {
        CopyText(detail, cap, L"REVIVE RESOLVE BLOCK: nhiều UIButton revive đồng điểm; không click mù");
        return false;
    }
    return true;
}

bool InvokeReviveButtonAction(const BridgeRequest& request,
                              FoundationSnapshot& foundation,
                              ReviveActionSnapshot& action,
                              GameSnapshot& freshGame,
                              wchar_t* detail, std::size_t cap) {
    action = {};
    freshGame = {};
    if (request.u64Arg0 == 0 || request.intArg0 <= 0 || request.intArg1 <= 0) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: invalid token/identity PRE");
        return false;
    }

    wchar_t snapshotDetail[768]{};
    if (!ReadGameSnapshot(foundation, freshGame, snapshotDetail, _countof(snapshotDetail))) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: fresh PRE snapshot unavailable");
        return false;
    }
    if ((freshGame.validMask & (ValidRoleIdentity | ValidMapId | ValidLifeState |
                                ValidMapReadyState | ValidMapTransitionState)) !=
        (ValidRoleIdentity | ValidMapId | ValidLifeState |
         ValidMapReadyState | ValidMapTransitionState)) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: fresh PRE snapshot partial");
        return false;
    }
    if (freshGame.roleID != request.intArg0 || freshGame.mapID != request.intArg1) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: identity drift before UI resolve");
        return false;
    }
    if (!freshGame.dead || !freshGame.mapReady || freshGame.waitingChangeMap) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: requires dead=1, mapReady=1, waitingMap=0");
        return false;
    }

    Il2CppObject* button = nullptr;
    const MethodInfo* handleClick = nullptr;
    int score = 0;
    wchar_t label[96]{};
    if (!FindFreshReviveButton(button, score, label, _countof(label), handleClick, detail, cap)) {
        return false;
    }

    // Re-read actual game PRE immediately before the one allowed gameplay callback.
    FoundationSnapshot immediateFoundation{};
    GameSnapshot immediateGame{};
    if (!ReadGameSnapshot(immediateFoundation, immediateGame, snapshotDetail, _countof(snapshotDetail))) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: immediate PRE snapshot unavailable");
        return false;
    }
    if (immediateGame.roleID != request.intArg0 || immediateGame.mapID != request.intArg1 ||
        !immediateGame.dead || !immediateGame.mapReady || immediateGame.waitingChangeMap) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: PRE changed while resolving button; no callback");
        return false;
    }

    Il2CppClass* actualClass = g_api.object_get_class(button);
    const Il2CppImage* image = AssemblyCSharpImage();
    Il2CppClass* uiButtonClass = image
        ? g_api.class_from_name(image, "FGStudio.LuaSystem.GUI", "UIButton") : nullptr;
    const MethodInfo* activeMethod = uiButtonClass
        ? FindMethodInHierarchy(uiButtonClass, "get_ActiveInHierarchy", 0) : nullptr;
    if (!activeMethod && image) {
        Il2CppClass* uiObjectClass = g_api.class_from_name(image, "FGStudio.LuaSystem.Base", "UIObject");
        activeMethod = uiObjectClass ? FindMethodInHierarchy(uiObjectClass, "get_ActiveInHierarchy", 0) : nullptr;
    }
    const MethodInfo* interactableMethod = uiButtonClass
        ? FindMethodInHierarchy(uiButtonClass, "get_Interactable", 0) : nullptr;
    const MethodInfo* textMethod = uiButtonClass
        ? FindMethodInHierarchy(uiButtonClass, "get_Text", 0) : nullptr;
    if (!actualClass || !ClassIsOrDerivesFrom(actualClass, uiButtonClass) ||
        !activeMethod || !interactableMethod || !textMethod) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: selected UIButton became invalid; no callback");
        return false;
    }
    bool active = false, interactable = false;
    if (!InvokeReadOnlyBool(activeMethod, button, active, detail, cap) || !active ||
        !InvokeReadOnlyBool(interactableMethod, button, interactable, detail, cap) || !interactable) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: selected UIButton no longer active/interactable");
        return false;
    }
    wchar_t finalText[128]{};
    if (!InvokeReadOnlyString(textMethod, button, finalText, _countof(finalText), detail, cap) ||
        ScoreReviveButton(L"", finalText) <= 0) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: selected UIButton label changed; no callback");
        return false;
    }

    void* exception = nullptr;
    g_api.runtime_invoke(handleClick, button, nullptr, &exception); // exactly one gameplay mutation
    if (exception) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: UIButton.HandleClickEvent() threw managed exception");
        return false;
    }

    foundation = immediateFoundation;
    freshGame = immediateGame;
    action.token = request.u64Arg0;
    action.callbackThreadId = GetCurrentThreadId();
    action.currentManagedThreadId = immediateFoundation.currentManagedThreadId;
    action.unityMainManagedThreadId = immediateFoundation.unityMainManagedThreadId;
    action.preRoleID = immediateGame.roleID;
    action.preMapID = immediateGame.mapID;
    action.preDead = immediateGame.dead;
    action.buttonScore = score;
    action.invoked = 1;
    CopyText(action.buttonLabel, _countof(action.buttonLabel), label);

    ClearText(detail, cap);
    AppendText(detail, cap, L"REVIVE ACTION DISPATCHED • fresh UIButton=“");
    AppendText(detail, cap, action.buttonLabel);
    AppendText(detail, cap, L"” • HandleClickEvent() called ONCE • role=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(action.preRoleID));
    AppendText(detail, cap, L" map=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(action.preMapID));
    AppendText(detail, cap, L" • callbackTID=");
    AppendUInt(detail, cap, action.callbackThreadId);
    AppendText(detail, cap, L" • now WAIT real dead=1→0 POST; no retry spam");
    return true;
}
