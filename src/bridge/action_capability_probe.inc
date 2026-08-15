// Read-only action/dispatcher capability resolver.
// Metadata inspection only: no managed method invocation, no live gameplay/UI instance,
// no gameplay command. Donor names and prior runtime signatures are evidence inputs only.

bool MetadataParamEquals(const MethodInfo* method, std::uint32_t index, const char* expected) {
    if (!method || !expected || index >= g_api.method_get_param_count(method)) return false;
    const Il2CppType* type = g_api.method_get_param(method, index);
    if (!type) return false;
    char* raw = g_api.type_get_name(type);
    if (!raw) return false;
    const bool same = AsciiEquals(raw, expected);
    g_api.free_fn(raw);
    return same;
}

const MethodInfo* FindMetadataMethod(Il2CppClass* klass, const char* name,
                                     std::uint32_t argc, bool requireStatic,
                                     const char* param0 = nullptr) {
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        void* iter = nullptr;
        while (const MethodInfo* method = g_api.class_get_methods(current, &iter)) {
            const char* candidate = g_api.method_get_name(method);
            if (!candidate || !AsciiEquals(candidate, name)) continue;
            if (g_api.method_get_param_count(method) != argc) continue;
            if (IsStaticMethod(method) != requireStatic) continue;
            if (argc > 0 && param0 && !MetadataParamEquals(method, 0, param0)) continue;
            return method;
        }
    }
    return nullptr;
}

const MethodInfo* FindMetadataMethod3(Il2CppClass* klass, const char* name,
                                      bool requireStatic,
                                      const char* param0, const char* param1,
                                      const char* param2) {
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        void* iter = nullptr;
        while (const MethodInfo* method = g_api.class_get_methods(current, &iter)) {
            const char* candidate = g_api.method_get_name(method);
            if (!candidate || !AsciiEquals(candidate, name)) continue;
            if (g_api.method_get_param_count(method) != 3) continue;
            if (IsStaticMethod(method) != requireStatic) continue;
            if (!MetadataParamEquals(method, 0, param0) ||
                !MetadataParamEquals(method, 1, param1) ||
                !MetadataParamEquals(method, 2, param2)) continue;
            return method;
        }
    }
    return nullptr;
}

const MethodInfo* FindMetadataMethodByName(Il2CppClass* klass, const char* name) {
    if (!klass || !name) return nullptr;
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        void* iter = nullptr;
        while (const MethodInfo* method = g_api.class_get_methods(current, &iter)) {
            const char* candidate = g_api.method_get_name(method);
            if (candidate && AsciiEquals(candidate, name)) return method;
        }
    }
    return nullptr;
}

void AppendMethodSignature(wchar_t* out, std::size_t cap,
                           Il2CppClass* klass, const char* name) {
    const MethodInfo* method = FindMetadataMethodByName(klass, name);
    if (!method) {
        AppendAscii(out, cap, name);
        AppendText(out, cap, L"(?)");
        return;
    }
    if (IsStaticMethod(method)) AppendText(out, cap, L"static ");
    AppendAscii(out, cap, g_api.method_get_name(method));
    AppendText(out, cap, L"(");
    const std::uint32_t argc = g_api.method_get_param_count(method);
    for (std::uint32_t i = 0; i < argc; ++i) {
        if (i) AppendText(out, cap, L",");
        AppendTypeName(out, cap, g_api.method_get_param(method, i));
    }
    AppendText(out, cap, L")");
}

struct OptionalClassEnumerator {
    using GetCountFn = std::size_t (__cdecl*)(const Il2CppImage*);
    using GetClassFn = Il2CppClass* (__cdecl*)(const Il2CppImage*, std::size_t);
    GetCountFn getCount = nullptr;
    GetClassFn getClass = nullptr;

    bool Load() {
        if (!g_api.gameAssembly) return false;
        return ResolveProcAddress(g_api.gameAssembly, "il2cpp_image_get_class_count", getCount) &&
               ResolveProcAddress(g_api.gameAssembly, "il2cpp_image_get_class", getClass);
    }
};

Il2CppClass* FindUniqueClassByName(const Il2CppImage* image,
                                   const OptionalClassEnumerator& enumerator,
                                   const char* className,
                                   std::uint32_t& matches) {
    matches = 0;
    Il2CppClass* found = nullptr;
    if (!image || !enumerator.getCount || !enumerator.getClass || !className) return nullptr;
    const std::size_t count = enumerator.getCount(image);
    for (std::size_t i = 0; i < count; ++i) {
        Il2CppClass* klass = enumerator.getClass(image, i);
        if (!klass) continue;
        const char* name = g_api.class_get_name(klass);
        if (!name || !AsciiEquals(name, className)) continue;
        ++matches;
        found = klass;
    }
    return matches == 1 ? found : nullptr;
}

void AppendClassIdentity(wchar_t* out, std::size_t cap, Il2CppClass* klass) {
    if (!klass) { AppendText(out, cap, L"?"); return; }
    const char* nameSpace = g_api.class_get_namespace(klass);
    if (nameSpace && *nameSpace) {
        AppendAscii(out, cap, nameSpace);
        AppendText(out, cap, L".");
    }
    AppendAscii(out, cap, g_api.class_get_name(klass));
}

bool HasActionMetadata(std::uint32_t mask, std::uint32_t required) {
    return (mask & required) == required;
}

bool ProbeActionCapabilities(ActionCapabilitySnapshot& out, wchar_t* detail, std::size_t cap) {
    out = {};
    out.probeVersion = 3;

    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) {
        CopyText(detail, cap, L"ACTION META PROBE: Assembly-CSharp unavailable; mutation LOCKED");
        return false;
    }

    Il2CppClass* uiObject = g_api.class_from_name(image, "FGStudio.LuaSystem.Base", "UIObject");
    if (uiObject) {
        out.metadataMask |= CapUiObjectType;
        if (FindFieldInHierarchy(uiObject, "instances")) out.metadataMask |= CapUiObjectInstances;
        if (FindMetadataMethod(uiObject, "get_ActiveInHierarchy", 0, false))
            out.metadataMask |= CapUiObjectActive;
        if (FindMetadataMethod(uiObject, "get_CoreChildren", 0, false))
            out.metadataMask |= CapUiObjectChildren;
    }

    Il2CppClass* uiButton = g_api.class_from_name(image, "FGStudio.LuaSystem.GUI", "UIButton");
    if (uiButton) {
        out.metadataMask |= CapUiButtonType;
        if (FindMetadataMethod(uiButton, "get_Interactable", 0, false))
            out.metadataMask |= CapUiButtonInteractable;
        if (FindMetadataMethod(uiButton, "get_Text", 0, false))
            out.metadataMask |= CapUiButtonText;
        if (FindMetadataMethod(uiButton, "HandleClickEvent", 0, false))
            out.metadataMask |= CapUiButtonHandleClick;
    }

    Il2CppClass* uiToggle = g_api.class_from_name(image, "FGStudio.LuaSystem.GUI", "UIToggle");
    if (uiToggle) {
        out.metadataMask |= CapUiToggleType;
        if (FindMetadataMethod(uiToggle, "get_Selected", 0, false))
            out.metadataMask |= CapUiToggleSelected;
        if (FindMetadataMethod(uiToggle, "set_Selected", 1, false, "System.Boolean"))
            out.metadataMask |= CapUiToggleSetSelected;
        if (FindMetadataMethod(uiToggle, "HandleSelectEvent", 1, false, "System.Boolean"))
            out.metadataMask |= CapUiToggleHandleSelect;
    }

    Il2CppClass* gameApi =
        g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    if (gameApi) {
        out.metadataMask |= CapGameApiType;
        if (FindMetadataMethod(gameApi, "ClickNPC", 1, true, "System.Int32"))
            out.metadataMask |= CapClickNpc;
        if (FindMetadataMethod(gameApi, "GetNearestNPC", 1, true, "System.Int32"))
            out.metadataMask |= CapGetNearestNpc;
        if (FindMetadataMethod(gameApi, "GetFreeBagSpace", 0, true))
            out.metadataMask |= CapBagFreeSpace;
        if (FindMetadataMethod(gameApi, "GetItemsAtSite", 1, true, "System.Int32"))
            out.metadataMask |= CapGetItemsAtSite;
        if (FindMetadataMethod(gameApi, "GetItemData", 1, true, "System.Int32"))
            out.metadataMask |= CapGetItemData;
        if (FindMetadataMethod(gameApi, "GetItemType", 1, true, "System.Int32"))
            out.metadataMask |= CapGetItemType;
        if (FindMetadataMethod(gameApi, "GetEquipType", 1, true, "System.Int32"))
            out.metadataMask |= CapGetEquipType;
        if (FindMetadataMethod(gameApi, "IsItemSellable", 1, true, "System.Int32"))
            out.metadataMask |= CapIsItemSellable;
    }

    Il2CppClass* mainThread =
        g_api.class_from_name(image, "FGStudio.Engine.Utilities", "MainThread");
    if (mainThread) {
        out.metadataMask |= CapMainThreadType;
        if (FindMetadataMethod(mainThread, "get_Instance", 0, true))
            out.metadataMask |= CapMainThreadInstanceGetter;
        if (FindMetadataMethod(mainThread, "Execute", 1, false, "System.Action"))
            out.metadataMask |= CapMainThreadExecuteAction;
    }

    ClearText(detail, cap);
    AppendText(detail, cap, L"ACTION META PROBE v3 mask=");
    AppendUInt(detail, cap, out.metadataMask);
    AppendText(detail, cap, L" • revive=");
    AppendText(detail, cap, HasActionMetadata(out.metadataMask, kReviveMetadataSupportMask) ? L"READY" : L"PARTIAL");
    AppendText(detail, cap, L" • npc=");
    AppendText(detail, cap, HasActionMetadata(out.metadataMask, kNpcMetadataSupportMask) ? L"READY" : L"PARTIAL");
    AppendText(detail, cap, L" • inventory=");
    AppendText(detail, cap, HasActionMetadata(out.metadataMask, kInventoryMetadataSupportMask) ? L"READY" : L"PARTIAL");
    AppendText(detail, cap, L" • mainThread=");
    AppendText(detail, cap,
               HasActionMetadata(out.metadataMask, kMainThreadDispatcherMetadataSupportMask) ? L"READY{" : L"PARTIAL{");
    AppendMethodSignature(detail, cap, mainThread, "get_Instance");
    AppendText(detail, cap, L";");
    AppendMethodSignature(detail, cap, mainThread, "Execute");
    AppendText(detail, cap, L"}");

    OptionalClassEnumerator enumerator{};
    if (!enumerator.Load()) {
        AppendText(detail, cap, L" • classEnum=UNAVAILABLE");
    } else {
        std::uint32_t executorMatches = 0;
        Il2CppClass* executor =
            FindUniqueClassByName(image, enumerator, "MonoBehaviourExecutor", executorMatches);
        if (executor) {
            out.metadataMask |= CapMonoExecutorType;
            if (FindMetadataMethod(executor, "get_Instance", 0, true))
                out.metadataMask |= CapMonoExecutorInstanceGetter;
            if (FindMetadataMethod3(executor, "ExecuteScriptFunction", false,
                                    "FGStudio.LuaSystem.Base.UIObject",
                                    "System.String", "System.Object[]"))
                out.metadataMask |= CapMonoExecutorExecuteScript;
        }
        AppendText(detail, cap, L" • luaExec=");
        if (executor) AppendClassIdentity(detail, cap, executor);
        else AppendText(detail, cap, executorMatches > 1 ? L"AMBIGUOUS" : L"NOT_FOUND");
        AppendText(detail, cap,
                   HasActionMetadata(out.metadataMask, kLuaExecutorMetadataSupportMask) ? L":READY" : L":PARTIAL");

        std::uint32_t dispatcherMatches = 0;
        Il2CppClass* dispatcher =
            FindUniqueClassByName(image, enumerator, "UnityMainThreadDispatcher", dispatcherMatches);
        if (dispatcher) {
            out.metadataMask |= CapUnityDispatcherType;
            if (FindMetadataMethod(dispatcher, "get_Instance", 0, true))
                out.metadataMask |= CapUnityDispatcherInstanceGetter;
            if (FindMetadataMethod(dispatcher, "Enqueue", 1, false, "System.Action"))
                out.metadataMask |= CapUnityDispatcherEnqueueAction;
            if (FindMetadataMethod(dispatcher, "Dispatch", 1, true, "System.Action"))
                out.metadataMask |= CapUnityDispatcherDispatchAction;
        }
        AppendText(detail, cap, L" • unityDispatcher=");
        if (dispatcher) {
            AppendClassIdentity(detail, cap, dispatcher);
            AppendText(detail, cap,
                       HasActionMetadata(out.metadataMask, kUnityDispatcherMetadataSupportMask) ? L":READY{" : L":PARTIAL{");
            AppendMethodSignature(detail, cap, dispatcher, "get_Instance");
            AppendText(detail, cap, L";");
            AppendMethodSignature(detail, cap, dispatcher, "Enqueue");
            AppendText(detail, cap, L";");
            AppendMethodSignature(detail, cap, dispatcher, "Dispatch");
            AppendText(detail, cap, L"}");
        } else {
            AppendText(detail, cap, dispatcherMatches > 1 ? L"AMBIGUOUS" : L"NOT_FOUND");
        }
    }

    AppendText(detail, cap,
               L" • no live object • no action invoke • mutation LOCKED");
    return true;
}
