    AppendText(detail, cap, L". Chỉ chứng minh đường dispatch; CHƯA gọi action game.");
    return true;
}


const MethodInfo* FindMethodInHierarchy(Il2CppClass* klass, const char* name, int argc) {
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        const MethodInfo* method = g_api.class_get_method_from_name(current, name, argc);
        if (method) return method;
    }
    return nullptr;
}

FieldInfo* FindFieldInHierarchy(Il2CppClass* klass, const char* name) {
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        FieldInfo* field = g_api.class_get_field_from_name(current, name);
        if (field) return field;
    }
    return nullptr;
}

bool IsStaticMethod(const MethodInfo* method) {
    if (!method) return false;
    constexpr std::uint32_t METHOD_ATTRIBUTE_STATIC = 0x0010;
    std::uint32_t iflags = 0;
    return (g_api.method_get_flags(method, &iflags) & METHOD_ATTRIBUTE_STATIC) != 0;
}

bool InvokeObject(const MethodInfo* method, void* instance, Il2CppObject*& value,
                  wchar_t* detail, std::size_t cap) {
    value = nullptr;
    if (!method) { CopyText(detail, cap, L"Thiếu read-only getter"); return false; }
    void* exception = nullptr;
    value = g_api.runtime_invoke(method, instance, nullptr, &exception);
    if (exception) { CopyText(detail, cap, L"Read-only getter ném managed exception"); return false; }
    return true;
}

bool InvokeScalar(const MethodInfo* method, void* instance, std::int64_t& value,
                  wchar_t* detail, std::size_t cap) {
    value = 0;
    if (!method) { CopyText(detail, cap, L"Thiếu scalar getter read-only"); return false; }
    const Il2CppType* returnType = g_api.method_get_return_type(method);
    if (!returnType) { CopyText(detail, cap, L"Không đọc được return type của getter"); return false; }
    char* typeName = g_api.type_get_name(returnType);
    if (!typeName) { CopyText(detail, cap, L"Không lấy được tên return type"); return false; }

    void* exception = nullptr;
    Il2CppObject* boxed = g_api.runtime_invoke(method, instance, nullptr, &exception);
    if (exception || !boxed) {
        g_api.free_fn(typeName);
        CopyText(detail, cap, exception ? L"Scalar getter ném managed exception" : L"Scalar getter trả null");
        return false;
    }
    void* raw = g_api.object_unbox(boxed);
    if (!raw) {
        g_api.free_fn(typeName);
        CopyText(detail, cap, L"Không unbox được scalar getter");
        return false;
    }

    bool supported = true;
    if (AsciiEquals(typeName, "System.Boolean")) value = *reinterpret_cast<const std::uint8_t*>(raw) ? 1 : 0;
    else if (AsciiEquals(typeName, "System.Byte")) value = *reinterpret_cast<const std::uint8_t*>(raw);
    else if (AsciiEquals(typeName, "System.SByte")) value = *reinterpret_cast<const std::int8_t*>(raw);
    else if (AsciiEquals(typeName, "System.Int16")) value = *reinterpret_cast<const std::int16_t*>(raw);
    else if (AsciiEquals(typeName, "System.UInt16")) value = *reinterpret_cast<const std::uint16_t*>(raw);
    else if (AsciiEquals(typeName, "System.Int32")) value = *reinterpret_cast<const std::int32_t*>(raw);
    else if (AsciiEquals(typeName, "System.UInt32")) value = *reinterpret_cast<const std::uint32_t*>(raw);
    else if (AsciiEquals(typeName, "System.Int64")) value = *reinterpret_cast<const std::int64_t*>(raw);
    else if (AsciiEquals(typeName, "System.UInt64")) value = static_cast<std::int64_t>(*reinterpret_cast<const std::uint64_t*>(raw));
    else supported = false;

    if (!supported) {
        ClearText(detail, cap);
        AppendText(detail, cap, L"Return type chưa hỗ trợ: ");
        AppendAscii(detail, cap, typeName);
    }
    g_api.free_fn(typeName);
    return supported;
}

bool ReadScalarGetter(Il2CppClass* klass, const char* name, void* instance,
                      std::int32_t& value, wchar_t* detail, std::size_t cap) {
    const MethodInfo* method = FindMethodInHierarchy(klass, name, 0);
    if (!method) {
        ClearText(detail, cap); AppendText(detail, cap, L"Không resolve được getter: "); AppendAscii(detail, cap, name);
        return false;
    }
    std::int64_t wide = 0;
    if (!InvokeScalar(method, instance, wide, detail, cap)) return false;
    if (wide < static_cast<std::int64_t>(INT32_MIN) || wide > static_cast<std::int64_t>(INT32_MAX)) {
        CopyText(detail, cap, L"Giá trị getter vượt Int32; fail-closed"); return false;
    }
    value = static_cast<std::int32_t>(wide);
    return true;
}

bool ReadStaticScalar(Il2CppClass* klass, const char* name, std::int32_t& value,
                      wchar_t* detail, std::size_t cap) {
