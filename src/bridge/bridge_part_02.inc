        if (gameAssembly && resolved == required) return true;
        gameAssembly = GetModuleHandleW(L"GameAssembly.dll");
        if (!gameAssembly) {
            CopyText(detail, cap, L"GameAssembly.dll chưa được load trong client");
            return false;
        }
        resolved = 0;
#define RESOLVE_COUNT(name) do { if (!ResolveProcAddress(gameAssembly, "il2cpp_" #name, name)) { \
            CopyText(detail, cap, L"Thiếu một IL2CPP metadata export bắt buộc"); return false; } ++resolved; } while (0)
        RESOLVE_COUNT(domain_get);
        RESOLVE_COUNT(domain_assembly_open);
        RESOLVE_COUNT(assembly_get_image);
        RESOLVE_COUNT(class_from_name);
        RESOLVE_COUNT(class_get_methods);
        RESOLVE_COUNT(method_get_name);
        RESOLVE_COUNT(method_get_param_count);
        RESOLVE_COUNT(method_get_param);
        RESOLVE_COUNT(method_get_flags);
        RESOLVE_COUNT(type_get_name);
#undef RESOLVE_COUNT
        if (!ResolveProcAddress(gameAssembly, "il2cpp_free", free_fn)) {
            CopyText(detail, cap, L"Thiếu export: il2cpp_free"); return false;
        }
        ++resolved;
#define RESOLVE_COUNT2(name) do { if (!ResolveProcAddress(gameAssembly, "il2cpp_" #name, name)) { \
            CopyText(detail, cap, L"Thiếu một IL2CPP metadata export bắt buộc"); return false; } ++resolved; } while (0)
        RESOLVE_COUNT2(class_get_fields);
        RESOLVE_COUNT2(field_get_name);
        RESOLVE_COUNT2(field_get_type);
        RESOLVE_COUNT2(thread_current);
        RESOLVE_COUNT2(get_corlib);
        RESOLVE_COUNT2(class_get_method_from_name);
        RESOLVE_COUNT2(runtime_invoke);
        RESOLVE_COUNT2(object_get_class);
        RESOLVE_COUNT2(class_get_name);
        RESOLVE_COUNT2(class_get_namespace);
        RESOLVE_COUNT2(object_unbox);
        RESOLVE_COUNT2(class_get_parent);
        RESOLVE_COUNT2(class_get_field_from_name);
        RESOLVE_COUNT2(field_get_value);
        RESOLVE_COUNT2(method_get_return_type);
        RESOLVE_COUNT2(string_length);
        RESOLVE_COUNT2(string_chars);
        RESOLVE_COUNT2(class_from_type);
        RESOLVE_COUNT2(class_is_valuetype);
#undef RESOLVE_COUNT2
        CopyText(detail, cap, L"IL2CPP foundation exports OK");
        return true;
    }
};

Il2CppApi g_api;

void AppendTypeName(wchar_t* out, std::size_t cap, const Il2CppType* type) {
    if (!type || !g_api.type_get_name) { AppendText(out, cap, L"?"); return; }
    char* raw = g_api.type_get_name(type);
    if (!raw) { AppendText(out, cap, L"?"); return; }
    AppendAscii(out, cap, raw);
    g_api.free_fn(raw);
}

const Il2CppImage* AssemblyCSharpImage() {
    Il2CppDomain* domain = g_api.domain_get ? g_api.domain_get() : nullptr;
    if (!domain) return nullptr;
    const Il2CppAssembly* assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp");
    if (!assembly) assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp.dll");
    return assembly ? g_api.assembly_get_image(assembly) : nullptr;
}

bool NativeValidate(FoundationSnapshot& snap, wchar_t* detail, std::size_t cap) {
    snap.hookThreadId = GetCurrentThreadId();
    snap.windowThreadId = g_shared ? g_shared->targetWindowThreadId : 0;
    if (!g_shared || g_shared->targetPid != GetCurrentProcessId()) {
        CopyText(detail, cap, L"Shared mapping/PID không khớp"); return false;
    }
    if (!snap.windowThreadId || snap.hookThreadId != snap.windowThreadId) {
        CopyText(detail, cap, L"Hook không chạy trên thread sở hữu cửa sổ game"); return false;
    }
    snap.validMask |= ValidHookThread;

    wchar_t apiDetail[256]{};
    if (!g_api.Load(apiDetail, _countof(apiDetail))) {
        snap.resolvedExports = g_api.resolved;
        snap.requiredExports = Il2CppApi::required;
        CopyText(detail, cap, apiDetail);
        return false;
    }
    snap.resolvedExports = g_api.resolved;
    snap.requiredExports = Il2CppApi::required;
    snap.validMask |= ValidIl2CppExports;

    ClearText(detail, cap);
    AppendText(detail, cap, L"HOOK PASS TID="); AppendUInt(detail, cap, snap.hookThreadId);
    AppendText(detail, cap, L"; IL2CPP metadata exports "); AppendUInt(detail, cap, snap.resolvedExports);
    AppendText(detail, cap, L"/"); AppendUInt(detail, cap, snap.requiredExports);
    AppendText(detail, cap, L"; runtime_invoke CHỈ được dùng ở probe read-only kế tiếp; Unity main thread CHƯA chứng minh");
    return true;
}

bool DescribeType(const char* nameSpace, const char* className,
                  std::uint32_t validBit,
                  std::uint32_t& methodCountOut,
                  std::uint32_t& fieldCountOut,
                  FoundationSnapshot& snap,
                  wchar_t* detail, std::size_t cap) {
