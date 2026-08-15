        CopyText(detail, cap, L"Không resolve được System.Threading.SynchronizationContext");
        return false;
    }
    const MethodInfo* getCurrent = g_api.class_get_method_from_name(syncBase, "get_Current", 0);
    if (!getCurrent) {
        CopyText(detail, cap, L"Không resolve được SynchronizationContext.get_Current()");
        return false;
    }

    void* exception = nullptr;
    Il2CppObject* syncContext = g_api.runtime_invoke(getCurrent, nullptr, nullptr, &exception);
    if (exception) {
        CopyText(detail, cap, L"SynchronizationContext.get_Current() ném exception; fail-closed");
        return false;
    }
    if (!syncContext) {
        CopyText(detail, cap, L"SynchronizationContext.Current = null trên hook thread; chưa phải Unity main thread");
        return false;
    }

    Il2CppClass* syncClass = g_api.object_get_class(syncContext);
    if (!syncClass) {
        CopyText(detail, cap, L"Không lấy được class của SynchronizationContext.Current");
        return false;
    }
    const char* syncName = g_api.class_get_name(syncClass);
    const char* syncNs = g_api.class_get_namespace(syncClass);
    if (!AsciiEquals(syncName, "UnitySynchronizationContext")) {
        ClearText(detail, cap);
        AppendText(detail, cap, L"SynchronizationContext.Current không phải UnitySynchronizationContext: ");
        if (syncNs && *syncNs) { AppendAscii(detail, cap, syncNs); AppendText(detail, cap, L"."); }
        AppendAscii(detail, cap, syncName ? syncName : "?");
        return false;
    }
    snap.validMask |= ValidUnitySyncContext;

    const MethodInfo* getUnityMainId = g_api.class_get_method_from_name(syncClass, "get_MainThreadId", 0);
    if (!getUnityMainId) {
        CopyText(detail, cap, L"UnitySynchronizationContext có mặt nhưng thiếu get_MainThreadId()");
        return false;
    }
    std::int32_t unityMainManagedId = 0;
    if (!InvokeInt32Getter(getUnityMainId, syncContext, unityMainManagedId, detail, cap)) return false;

    Il2CppClass* threadClass = g_api.class_from_name(corlib, "System.Threading", "Thread");
    if (!threadClass) {
        CopyText(detail, cap, L"Không resolve được System.Threading.Thread");
        return false;
    }
    const MethodInfo* getCurrentThread = g_api.class_get_method_from_name(threadClass, "get_CurrentThread", 0);
    if (!getCurrentThread) {
        CopyText(detail, cap, L"Không resolve được Thread.get_CurrentThread()");
        return false;
    }
    exception = nullptr;
    Il2CppObject* currentThread = g_api.runtime_invoke(getCurrentThread, nullptr, nullptr, &exception);
    if (exception || !currentThread) {
        CopyText(detail, cap, L"Không đọc được Thread.CurrentThread; fail-closed");
        return false;
    }
    Il2CppClass* currentThreadClass = g_api.object_get_class(currentThread);
    if (!currentThreadClass) {
        CopyText(detail, cap, L"Không lấy được class của Thread.CurrentThread");
        return false;
    }
    const MethodInfo* getManagedThreadId = g_api.class_get_method_from_name(currentThreadClass, "get_ManagedThreadId", 0);
    if (!getManagedThreadId) {
        CopyText(detail, cap, L"Không resolve được Thread.get_ManagedThreadId()");
        return false;
    }
    std::int32_t currentManagedId = 0;
    if (!InvokeInt32Getter(getManagedThreadId, currentThread, currentManagedId, detail, cap)) return false;

    snap.currentManagedThreadId = currentManagedId;
    snap.unityMainManagedThreadId = unityMainManagedId;
    if (currentManagedId <= 0 || unityMainManagedId <= 0 || currentManagedId != unityMainManagedId) {
        ClearText(detail, cap);
        AppendText(detail, cap, L"UnitySynchronizationContext tồn tại nhưng ManagedThreadId lệch: current=");
        AppendUInt(detail, cap, static_cast<std::uint32_t>(currentManagedId));
        AppendText(detail, cap, L", unityMain=");
        AppendUInt(detail, cap, static_cast<std::uint32_t>(unityMainManagedId));
        AppendText(detail, cap, L"; action vẫn KHÓA");
        return false;
    }

    snap.validMask |= ValidUnityMainThread;
    ClearText(detail, cap);
    AppendText(detail, cap, L"MAINTHREAD PROVEN: hook TID=");
    AppendUInt(detail, cap, snap.hookThreadId);
    AppendText(detail, cap, L"; SynchronizationContext=");
    if (syncNs && *syncNs) { AppendAscii(detail, cap, syncNs); AppendText(detail, cap, L"."); }
    AppendAscii(detail, cap, syncName);
    AppendText(detail, cap, L"; managed current/main=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(currentManagedId));
    AppendText(detail, cap, L"/");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(unityMainManagedId));
