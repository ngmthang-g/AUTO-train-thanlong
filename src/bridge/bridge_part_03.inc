    wchar_t nativeDetail[512]{};
    if (!NativeValidate(snap, nativeDetail, _countof(nativeDetail))) {
        CopyText(detail, cap, nativeDetail); return false;
    }
    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) { CopyText(detail, cap, L"Không resolve được Assembly-CSharp bằng metadata API"); return false; }
    Il2CppClass* klass = g_api.class_from_name(image, nameSpace, className);
    if (!klass) {
        ClearText(detail, cap); AppendText(detail, cap, L"Không tìm thấy type ");
        if (nameSpace && *nameSpace) { AppendAscii(detail, cap, nameSpace); AppendText(detail, cap, L"."); }
        AppendAscii(detail, cap, className); return false;
    }

    constexpr std::uint32_t METHOD_ATTRIBUTE_STATIC = 0x0010;
    ClearText(detail, cap);
    if (nameSpace && *nameSpace) { AppendAscii(detail, cap, nameSpace); AppendText(detail, cap, L"."); }
    AppendAscii(detail, cap, className);

    void* iter = nullptr;
    std::uint32_t shownMethods = 0;
    wchar_t methodText[700]{};
    while (const MethodInfo* method = g_api.class_get_methods(klass, &iter)) {
        ++methodCountOut;
        if (shownMethods >= 16) continue;
        if (shownMethods++) AppendText(methodText, _countof(methodText), L"; ");
        std::uint32_t iflags = 0;
        const bool isStatic = (g_api.method_get_flags(method, &iflags) & METHOD_ATTRIBUTE_STATIC) != 0;
        if (isStatic) AppendText(methodText, _countof(methodText), L"static ");
        AppendAscii(methodText, _countof(methodText), g_api.method_get_name(method));
        AppendText(methodText, _countof(methodText), L"(");
        const std::uint32_t argc = g_api.method_get_param_count(method);
        for (std::uint32_t i = 0; i < argc; ++i) {
            if (i) AppendText(methodText, _countof(methodText), L", ");
            AppendTypeName(methodText, _countof(methodText), g_api.method_get_param(method, i));
        }
        AppendText(methodText, _countof(methodText), L")");
    }

    iter = nullptr;
    std::uint32_t shownFields = 0;
    wchar_t fieldText[420]{};
    while (FieldInfo* field = g_api.class_get_fields(klass, &iter)) {
        ++fieldCountOut;
        if (shownFields >= 12) continue;
        if (shownFields++) AppendText(fieldText, _countof(fieldText), L"; ");
        AppendTypeName(fieldText, _countof(fieldText), g_api.field_get_type(field));
        AppendText(fieldText, _countof(fieldText), L" ");
        AppendAscii(fieldText, _countof(fieldText), g_api.field_get_name(field));
    }

    snap.validMask |= validBit;
    AppendText(detail, cap, L": methods="); AppendUInt(detail, cap, methodCountOut);
    AppendText(detail, cap, L", fields="); AppendUInt(detail, cap, fieldCountOut);
    if (methodText[0]) { AppendText(detail, cap, L" | M: "); AppendText(detail, cap, methodText); }
    if (fieldText[0]) { AppendText(detail, cap, L" | F: "); AppendText(detail, cap, fieldText); }
    return true;
}


bool InvokeInt32Getter(const MethodInfo* method, void* instance, std::int32_t& value, wchar_t* detail, std::size_t cap) {
    if (!method) { CopyText(detail, cap, L"Thiếu method getter cần cho main-thread proof"); return false; }
    void* exception = nullptr;
    Il2CppObject* boxed = g_api.runtime_invoke(method, instance, nullptr, &exception);
    if (exception) { CopyText(detail, cap, L"Managed getter ném exception; fail-closed"); return false; }
    if (!boxed) { CopyText(detail, cap, L"Managed getter trả null thay vì Int32"); return false; }
    void* raw = g_api.object_unbox(boxed);
    if (!raw) { CopyText(detail, cap, L"Không unbox được Int32 từ managed getter"); return false; }
    value = *reinterpret_cast<const std::int32_t*>(raw);
    return true;
}

bool ProveUnityMainThread(FoundationSnapshot& snap, wchar_t* detail, std::size_t cap) {
    wchar_t nativeDetail[512]{};
    if (!NativeValidate(snap, nativeDetail, _countof(nativeDetail))) {
        CopyText(detail, cap, nativeDetail);
        return false;
    }

    // Fail closed: tuyệt đối không attach một worker lạ vào IL2CPP ở đây.
    // Hook phải vốn đã chạy trên một managed IL2CPP thread của game.
    if (!g_api.thread_current || !g_api.thread_current()) {
        CopyText(detail, cap, L"Hook/window thread không phải IL2CPP managed thread hiện tại; không chứng minh main thread");
        return false;
    }

    const Il2CppImage* corlib = g_api.get_corlib ? g_api.get_corlib() : nullptr;
    if (!corlib) {
        CopyText(detail, cap, L"Không lấy được corlib để kiểm tra SynchronizationContext");
        return false;
    }

    Il2CppClass* syncBase = g_api.class_from_name(corlib, "System.Threading", "SynchronizationContext");
    if (!syncBase) {
