// NewCore v1.2.1 - Revive anti-diss hotfix.
// IMPORTANT: WH_GETMESSAGE is only a short command ingress. It must never synchronously
// execute UIButton.HandleClickEvent(). The button is fresh-resolved with a strict time budget,
// wrapped as System.Action, then queued into FGStudio.Engine.Utilities.MainThread.Execute().

using FieldStaticGetValueFn = void (__cdecl*)(FieldInfo*, void*);
using ClassGetTypeFn = const Il2CppType* (__cdecl*)(Il2CppClass*);
using TypeGetObjectFn = Il2CppObject* (__cdecl*)(const Il2CppType*);
using MethodGetObjectFn = Il2CppObject* (__cdecl*)(const MethodInfo*, Il2CppClass*);

constexpr ULONGLONG kReviveResolveBudgetMs = 120;
constexpr int kReviveResolveHardEntryCap = 4096;

void PublishReviveProgress(std::int32_t stage,
                           std::int32_t scannedObjects = 0,
                           std::int32_t scannedButtons = 0,
                           std::int32_t elapsedMs = 0) {
    if (!g_shared) return;
    g_shared->response.reviveAction.progressStage = stage;
    g_shared->response.reviveAction.scannedObjects = scannedObjects;
    g_shared->response.reviveAction.scannedButtons = scannedButtons;
    g_shared->response.reviveAction.elapsedMs = elapsedMs;
    MemoryBarrier();
}

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

int ScoreExactReviveText(const wchar_t* text) {
    wchar_t key[160]{};
    CompactReviveText(text, key, _countof(key));
    if (WideEqualsLiteral(key, L"dauthai")) return 1000;
    if (WideEqualsLiteral(key, L"hoisinh")) return 930;
    if (WideEqualsLiteral(key, L"revive") || WideEqualsLiteral(key, L"relive") ||
        WideEqualsLiteral(key, L"respawn")) return 900;
    return 0;
}

bool FindFreshReviveButtonBounded(Il2CppObject*& selected,
                                  Il2CppClass*& uiButtonClass,
                                  const MethodInfo*& handleClick,
                                  int& selectedScore,
                                  wchar_t* selectedLabel, std::size_t selectedLabelCap,
                                  std::int32_t& scannedObjects,
                                  std::int32_t& scannedButtons,
                                  std::int32_t& elapsedMs,
                                  wchar_t* detail, std::size_t cap) {
    selected = nullptr;
    uiButtonClass = nullptr;
    handleClick = nullptr;
    selectedScore = 0;
    scannedObjects = 0;
    scannedButtons = 0;
    elapsedMs = 0;
    if (selectedLabel && selectedLabelCap) selectedLabel[0] = L'\0';
    const ULONGLONG started = GetTickCount64();

    PublishReviveProgress(ReviveStageResolveTypes);
    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: Assembly-CSharp unavailable"); return false; }
    Il2CppClass* uiObjectClass = g_api.class_from_name(image, "FGStudio.LuaSystem.Base", "UIObject");
    uiButtonClass = g_api.class_from_name(image, "FGStudio.LuaSystem.GUI", "UIButton");
    if (!uiObjectClass || !uiButtonClass) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIObject/UIButton type missing"); return false;
    }

    FieldInfo* instancesField = FindFieldInHierarchy(uiObjectClass, "instances");
    if (!instancesField) { CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIObject.instances missing"); return false; }
    FieldStaticGetValueFn fieldStaticGetValue = nullptr;
    if (!ResolveProcAddress(g_api.gameAssembly, "il2cpp_field_static_get_value", fieldStaticGetValue)) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: il2cpp_field_static_get_value unavailable"); return false;
    }

    PublishReviveProgress(ReviveStageInstances);
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

    PublishReviveProgress(ReviveStageEnumerator);
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
    const MethodInfo* interactableMethod = FindMethodInHierarchy(uiButtonClass, "get_Interactable", 0);
    const MethodInfo* textMethod = FindMethodInHierarchy(uiButtonClass, "get_Text", 0);
    handleClick = FindMethodInHierarchy(uiButtonClass, "HandleClickEvent", 0);
    if (!activeMethod || !interactableMethod || !textMethod || !handleClick || IsStaticMethod(handleClick)) {
        CopyText(detail, cap, L"REVIVE RESOLVE FAIL: UIButton getter/callback signature incomplete"); return false;
    }

    PublishReviveProgress(ReviveStageScanButtons);
    for (int i = 0; i < kReviveResolveHardEntryCap; ++i) {
        const ULONGLONG elapsed = GetTickCount64() - started;
        elapsedMs = static_cast<std::int32_t>(elapsed > 0x7fffffffULL ? 0x7fffffffULL : elapsed);
        if (elapsed > kReviveResolveBudgetMs) {
            PublishReviveProgress(ReviveStageScanButtons, scannedObjects, scannedButtons, elapsedMs);
            ClearText(detail, cap);
            AppendText(detail, cap, L"REVIVE RESOLVE BLOCK: bounded UI scan exceeded ");
            AppendUInt(detail, cap, static_cast<std::uint32_t>(kReviveResolveBudgetMs));
            AppendText(detail, cap, L"ms after objects=");
            AppendUInt(detail, cap, static_cast<std::uint32_t>(scannedObjects));
            AppendText(detail, cap, L" buttons=");
            AppendUInt(detail, cap, static_cast<std::uint32_t>(scannedButtons));
            AppendText(detail, cap, L"; no action queued");
            return false;
        }

        std::int64_t hasNext = 0;
        if (!InvokeScalar(moveNext, enumerator, hasNext, detail, cap)) return false;
        if (!hasNext) break;
        ++scannedObjects;

        Il2CppObject* object = nullptr;
        if (!InvokeObject(getCurrent, enumerator, object, detail, cap)) return false;
        if (!object) continue;
        Il2CppClass* actual = g_api.object_get_class(object);
        if (!ClassIsOrDerivesFrom(actual, uiButtonClass)) continue;
        ++scannedButtons;

        wchar_t text[128]{};
        if (!InvokeReadOnlyString(textMethod, object, text, _countof(text), detail, cap)) continue;
        const int score = ScoreExactReviveText(text);
        if (score < 900) continue; // exact visible labels only in the hotfix; no fuzzy/name scan.

        bool active = false, interactable = false;
        if (!InvokeReadOnlyBool(activeMethod, object, active, detail, cap) || !active) continue;
        if (!InvokeReadOnlyBool(interactableMethod, object, interactable, detail, cap) || !interactable) continue;

        selected = object;
        selectedScore = score;
        CopyText(selectedLabel, selectedLabelCap, text);
        elapsedMs = static_cast<std::int32_t>(GetTickCount64() - started);
        PublishReviveProgress(ReviveStageCandidate, scannedObjects, scannedButtons, elapsedMs);
        return true;
    }

    elapsedMs = static_cast<std::int32_t>(GetTickCount64() - started);
    PublishReviveProgress(ReviveStageScanButtons, scannedObjects, scannedButtons, elapsedMs);
    ClearText(detail, cap);
    AppendText(detail, cap, L"REVIVE RESOLVE BLOCK: no exact active+interactable Đầu thai/Hồi sinh button within bounded scan; objects=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(scannedObjects));
    AppendText(detail, cap, L" buttons=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(scannedButtons));
    AppendText(detail, cap, L" elapsedMs=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(elapsedMs));
    return false;
}

bool CreateManagedSystemAction(Il2CppObject* target,
                               Il2CppClass* targetClass,
                               const MethodInfo* targetMethod,
                               Il2CppObject*& actionDelegate,
                               wchar_t* detail, std::size_t cap) {
    actionDelegate = nullptr;
    PublishReviveProgress(ReviveStageDelegateReflection);
    if (!target || !targetClass || !targetMethod) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: invalid delegate target"); return false;
    }

    ClassGetTypeFn classGetType = nullptr;
    TypeGetObjectFn typeGetObject = nullptr;
    MethodGetObjectFn methodGetObject = nullptr;
    if (!ResolveProcAddress(g_api.gameAssembly, "il2cpp_class_get_type", classGetType) ||
        !ResolveProcAddress(g_api.gameAssembly, "il2cpp_type_get_object", typeGetObject) ||
        !ResolveProcAddress(g_api.gameAssembly, "il2cpp_method_get_object", methodGetObject)) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: reflection exports class_get_type/type_get_object/method_get_object unavailable");
        return false;
    }

    const Il2CppImage* corlib = g_api.get_corlib ? g_api.get_corlib() : nullptr;
    if (!corlib) { CopyText(detail, cap, L"REVIVE DISPATCH FAIL: corlib unavailable"); return false; }
    Il2CppClass* actionClass = g_api.class_from_name(corlib, "System", "Action");
    Il2CppClass* delegateClass = g_api.class_from_name(corlib, "System", "Delegate");
    if (!actionClass || !delegateClass) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: System.Action/Delegate metadata missing"); return false;
    }

    const MethodInfo* createDelegate = FindMetadataMethod3(
        delegateClass, "CreateDelegate", true,
        "System.Type", "System.Object", "System.Reflection.MethodInfo");
    if (!createDelegate) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: Delegate.CreateDelegate(Type,Object,MethodInfo) missing");
        return false;
    }

    const Il2CppType* actionType = classGetType(actionClass);
    Il2CppObject* actionTypeObject = actionType ? typeGetObject(actionType) : nullptr;
    Il2CppObject* methodObject = methodGetObject(targetMethod, targetClass);
    if (!actionTypeObject || !methodObject) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: reflection Type/MethodInfo object unavailable");
        return false;
    }

    PublishReviveProgress(ReviveStageCreateDelegate);
    void* args[3] = {actionTypeObject, target, methodObject};
    void* exception = nullptr;
    actionDelegate = g_api.runtime_invoke(createDelegate, nullptr, args, &exception);
    if (exception || !actionDelegate) {
        CopyText(detail, cap, exception
            ? L"REVIVE DISPATCH FAIL: Delegate.CreateDelegate threw managed exception"
            : L"REVIVE DISPATCH FAIL: Delegate.CreateDelegate returned null");
        return false;
    }
    Il2CppClass* actualDelegateClass = g_api.object_get_class(actionDelegate);
    if (!ClassIsOrDerivesFrom(actualDelegateClass, actionClass)) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: created delegate is not System.Action");
        return false;
    }
    return true;
}

bool QueueSystemActionOnFgMainThread(Il2CppObject* actionDelegate,
                                     wchar_t* detail, std::size_t cap) {
    if (!actionDelegate) { CopyText(detail, cap, L"REVIVE DISPATCH FAIL: action delegate=null"); return false; }
    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) { CopyText(detail, cap, L"REVIVE DISPATCH FAIL: Assembly-CSharp unavailable for MainThread"); return false; }
    Il2CppClass* mainThreadClass =
        g_api.class_from_name(image, "FGStudio.Engine.Utilities", "MainThread");
    if (!mainThreadClass) { CopyText(detail, cap, L"REVIVE DISPATCH FAIL: MainThread type missing"); return false; }

    const MethodInfo* getInstance = FindMetadataMethod(mainThreadClass, "get_Instance", 0, true);
    const MethodInfo* execute = FindMetadataMethod(mainThreadClass, "Execute", 1, false, "System.Action");
    if (!getInstance || !execute) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: MainThread.get_Instance/Execute(System.Action) missing");
        return false;
    }

    PublishReviveProgress(ReviveStageMainThreadInstance);
    Il2CppObject* mainThread = nullptr;
    if (!InvokeObject(getInstance, nullptr, mainThread, detail, cap) || !mainThread) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: MainThread.Instance unavailable"); return false;
    }

    PublishReviveProgress(ReviveStageMainThreadEnqueue);
    void* args[1] = {actionDelegate};
    void* exception = nullptr;
    g_api.runtime_invoke(execute, mainThread, args, &exception);
    if (exception) {
        CopyText(detail, cap, L"REVIVE DISPATCH FAIL: MainThread.Execute(System.Action) threw managed exception");
        return false;
    }
    PublishReviveProgress(ReviveStageQueued);
    return true;
}

bool InvokeReviveButtonAction(const BridgeRequest& request,
                              FoundationSnapshot& foundation,
                              ReviveActionSnapshot& action,
                              GameSnapshot& freshGame,
                              wchar_t* detail, std::size_t cap) {
    action = {};
    freshGame = {};
    if (g_shared) {
        g_shared->response.reviveAction = {};
        g_shared->response.reviveAction.token = request.u64Arg0;
        MemoryBarrier();
    }
    if (request.u64Arg0 == 0 || request.intArg0 <= 0 || request.intArg1 <= 0) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: invalid token/identity PRE");
        return false;
    }

    PublishReviveProgress(ReviveStageFreshPre);
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
    Il2CppClass* uiButtonClass = nullptr;
    const MethodInfo* handleClick = nullptr;
    int score = 0;
    wchar_t label[96]{};
    std::int32_t scannedObjects = 0;
    std::int32_t scannedButtons = 0;
    std::int32_t elapsedMs = 0;
    if (!FindFreshReviveButtonBounded(button, uiButtonClass, handleClick, score,
                                      label, _countof(label), scannedObjects,
                                      scannedButtons, elapsedMs, detail, cap)) {
        return false;
    }

    // Re-read actual PRE immediately before delegate creation/queueing.
    FoundationSnapshot immediateFoundation{};
    GameSnapshot immediateGame{};
    if (!ReadGameSnapshot(immediateFoundation, immediateGame, snapshotDetail, _countof(snapshotDetail))) {
        CopyText(detail, cap, L"REVIVE ACTION FAIL: immediate PRE snapshot unavailable");
        return false;
    }
    if (immediateGame.roleID != request.intArg0 || immediateGame.mapID != request.intArg1 ||
        !immediateGame.dead || !immediateGame.mapReady || immediateGame.waitingChangeMap) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: PRE changed while resolving button; no action queued");
        return false;
    }

    Il2CppClass* actualClass = g_api.object_get_class(button);
    const MethodInfo* activeMethod = uiButtonClass
        ? FindMethodInHierarchy(uiButtonClass, "get_ActiveInHierarchy", 0) : nullptr;
    if (!activeMethod) {
        const Il2CppImage* image = AssemblyCSharpImage();
        Il2CppClass* uiObjectClass = image
            ? g_api.class_from_name(image, "FGStudio.LuaSystem.Base", "UIObject") : nullptr;
        activeMethod = uiObjectClass ? FindMethodInHierarchy(uiObjectClass, "get_ActiveInHierarchy", 0) : nullptr;
    }
    const MethodInfo* interactableMethod = uiButtonClass
        ? FindMethodInHierarchy(uiButtonClass, "get_Interactable", 0) : nullptr;
    const MethodInfo* textMethod = uiButtonClass
        ? FindMethodInHierarchy(uiButtonClass, "get_Text", 0) : nullptr;
    if (!actualClass || !ClassIsOrDerivesFrom(actualClass, uiButtonClass) ||
        !activeMethod || !interactableMethod || !textMethod) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: selected UIButton became invalid; no action queued");
        return false;
    }

    bool active = false, interactable = false;
    wchar_t finalText[128]{};
    if (!InvokeReadOnlyBool(activeMethod, button, active, detail, cap) || !active ||
        !InvokeReadOnlyBool(interactableMethod, button, interactable, detail, cap) || !interactable ||
        !InvokeReadOnlyString(textMethod, button, finalText, _countof(finalText), detail, cap) ||
        ScoreExactReviveText(finalText) < 900) {
        CopyText(detail, cap, L"REVIVE ACTION BLOCK: selected UIButton changed before dispatch; no action queued");
        return false;
    }

    Il2CppObject* actionDelegate = nullptr;
    if (!CreateManagedSystemAction(button, actualClass, handleClick, actionDelegate, detail, cap)) {
        return false;
    }
    if (!QueueSystemActionOnFgMainThread(actionDelegate, detail, cap)) {
        return false;
    }

    foundation = immediateFoundation;
    freshGame = immediateGame;
    action.token = request.u64Arg0;
    action.callbackThreadId = GetCurrentThreadId(); // enqueue ingress thread, not UI callback execution.
    action.currentManagedThreadId = immediateFoundation.currentManagedThreadId;
    action.unityMainManagedThreadId = immediateFoundation.unityMainManagedThreadId;
    action.preRoleID = immediateGame.roleID;
    action.preMapID = immediateGame.mapID;
    action.preDead = immediateGame.dead;
    action.buttonScore = score;
    action.queued = 1;
    action.directInvoked = 0;
    action.progressStage = ReviveStageQueued;
    action.scannedObjects = scannedObjects;
    action.scannedButtons = scannedButtons;
    action.elapsedMs = elapsedMs;
    CopyText(action.buttonLabel, _countof(action.buttonLabel), label);

    ClearText(detail, cap);
    AppendText(detail, cap, L"REVIVE ACTION QUEUED • fresh UIButton=“");
    AppendText(detail, cap, action.buttonLabel);
    AppendText(detail, cap, L"” • System.Action -> MainThread.Execute() • direct HandleClickEvent invoke=0 • scanned objects=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(action.scannedObjects));
    AppendText(detail, cap, L" buttons=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(action.scannedButtons));
    AppendText(detail, cap, L" resolveMs=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(action.elapsedMs));
    AppendText(detail, cap, L" • now WAIT real dead=1→0 POST; no retry spam");
    return true;
}
