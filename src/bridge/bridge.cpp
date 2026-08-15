#include <windows.h>
#include <cstdint>
#include <cstddef>
#include "../common/protocol.h"

using namespace tlcore;

namespace {

HANDLE g_mapping = nullptr;
SharedBlock* g_shared = nullptr;

using Il2CppDomain = void;
using Il2CppAssembly = void;
using Il2CppImage = void;
using Il2CppClass = void;
using MethodInfo = void;
using FieldInfo = void;
using Il2CppType = void;
using Il2CppObject = void;
using Il2CppThread = void;
using Il2CppString = void;

template <typename T>
bool ResolveProcAddress(HMODULE module, const char* name, T& out) {
    out = nullptr;
    FARPROC raw = GetProcAddress(module, name);
    if (!raw) return false;
    static_assert(sizeof(raw) == sizeof(out), "Windows function pointer size mismatch");
    const unsigned char* src = reinterpret_cast<const unsigned char*>(&raw);
    unsigned char* dst = reinterpret_cast<unsigned char*>(&out);
    for (std::size_t i = 0; i < sizeof(out); ++i) dst[i] = src[i];
    return out != nullptr;
}

void ClearText(wchar_t* out, std::size_t cap) {
    if (out && cap) out[0] = L'\0';
}


void CopyText(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || cap == 0) return;
    std::size_t i = 0;
    if (text) {
        while (i + 1 < cap && text[i]) { out[i] = text[i]; ++i; }
    }
    out[i] = L'\0';
}

void AppendText(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || cap == 0 || !text) return;
    std::size_t used = 0;
    while (used + 1 < cap && out[used]) ++used;
    std::size_t i = 0;
    while (used + 1 < cap && text[i]) out[used++] = text[i++];
    out[used] = L'\0';
}

void AppendAscii(wchar_t* out, std::size_t cap, const char* text) {
    if (!text) { AppendText(out, cap, L"?"); return; }
    wchar_t tmp[256]{};
    std::size_t i = 0;
    while (i + 1 < _countof(tmp) && text[i]) {
        tmp[i] = static_cast<unsigned char>(text[i]);
        ++i;
    }
    tmp[i] = L'\0';
    AppendText(out, cap, tmp);
}


bool AsciiEquals(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) { if (*a != *b) return false; ++a; ++b; }
    return *a == *b;
}

void AppendUInt(wchar_t* out, std::size_t cap, std::uint32_t value) {
    wchar_t tmp[32]{};
    wsprintfW(tmp, L"%lu", static_cast<unsigned long>(value));
    AppendText(out, cap, tmp);
}

struct Il2CppApi {
    HMODULE gameAssembly = nullptr;
    Il2CppDomain* (__cdecl* domain_get)() = nullptr;
    const Il2CppAssembly* (__cdecl* domain_assembly_open)(Il2CppDomain*, const char*) = nullptr;
    const Il2CppImage* (__cdecl* assembly_get_image)(const Il2CppAssembly*) = nullptr;
    Il2CppClass* (__cdecl* class_from_name)(const Il2CppImage*, const char*, const char*) = nullptr;
    const MethodInfo* (__cdecl* class_get_methods)(Il2CppClass*, void**) = nullptr;
    const char* (__cdecl* method_get_name)(const MethodInfo*) = nullptr;
    std::uint32_t (__cdecl* method_get_param_count)(const MethodInfo*) = nullptr;
    const Il2CppType* (__cdecl* method_get_param)(const MethodInfo*, std::uint32_t) = nullptr;
    std::uint32_t (__cdecl* method_get_flags)(const MethodInfo*, std::uint32_t*) = nullptr;
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    FieldInfo* (__cdecl* class_get_fields)(Il2CppClass*, void**) = nullptr;
    const char* (__cdecl* field_get_name)(FieldInfo*) = nullptr;
    const Il2CppType* (__cdecl* field_get_type)(FieldInfo*) = nullptr;
    Il2CppThread* (__cdecl* thread_current)() = nullptr;
    const Il2CppImage* (__cdecl* get_corlib)() = nullptr;
    const MethodInfo* (__cdecl* class_get_method_from_name)(Il2CppClass*, const char*, int) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, void**) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    const char* (__cdecl* class_get_name)(Il2CppClass*) = nullptr;
    const char* (__cdecl* class_get_namespace)(Il2CppClass*) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppClass* (__cdecl* class_get_parent)(Il2CppClass*) = nullptr;
    FieldInfo* (__cdecl* class_get_field_from_name)(Il2CppClass*, const char*) = nullptr;
    void (__cdecl* field_get_value)(Il2CppObject*, FieldInfo*, void*) = nullptr;
    const Il2CppType* (__cdecl* method_get_return_type)(const MethodInfo*) = nullptr;
    std::int32_t (__cdecl* string_length)(Il2CppString*) = nullptr;
    const wchar_t* (__cdecl* string_chars)(Il2CppString*) = nullptr;
    Il2CppClass* (__cdecl* class_from_type)(const Il2CppType*) = nullptr;
    bool (__cdecl* class_is_valuetype)(const Il2CppClass*) = nullptr;
    std::uint32_t resolved = 0;
    static constexpr std::uint32_t required = 30;

    bool Load(wchar_t* detail, std::size_t cap) {
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

    Il2CppClass* sharedClass = g_api.class_from_name(image, "FGStudio.LuaSystem", "LuaSystemSharedData");
    if (!sharedClass) { CopyText(detail, cap, L"Thiếu FGStudio.LuaSystem.LuaSystemSharedData"); return false; }
    const MethodInfo* getLeader = FindMethodInHierarchy(sharedClass, "get_LeaderRoleData", 0);
    if (!getLeader || !IsStaticMethod(getLeader)) {
        CopyText(detail, cap, L"Thiếu static LuaSystemSharedData.get_LeaderRoleData()"); return false;
    }
    Il2CppObject* leader = nullptr;
    if (!InvokeObject(getLeader, nullptr, leader, detail, cap)) return false;
    if (!leader) { CopyText(detail, cap, L"LeaderRoleData=null: hãy đăng nhập nhân vật và vào map"); return false; }
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
    game.dead = dead ? 1 : 0; game.validMask |= ValidLifeState;
    if (!ReadScalarGetter(leaderClass, "get_IsRiding", leader, riding, detail, cap)) return false;
    game.riding = riding ? 1 : 0; game.validMask |= ValidRideState;

    const MethodInfo* getName = FindMethodInHierarchy(leaderClass, "get_Name", 0);
    if (getName) {
        Il2CppObject* nameObject = nullptr;
        if (InvokeObject(getName, leader, nameObject, detail, cap) && nameObject &&
            CopyManagedString(reinterpret_cast<Il2CppString*>(nameObject), game.characterName, _countof(game.characterName))) {
            game.validMask |= ValidCharacterName;
        }
    }

    // Position is intentionally discovered through metadata/managed fields instead of donor offsets.
    // First try inherited getters; if unavailable, inspect the managed roleData backing object.
    std::int32_t x = 0, y = 0;
    bool positionOk = ReadScalarGetter(leaderClass, "get_PosX", leader, x, detail, cap) &&
                      ReadScalarGetter(leaderClass, "get_PosY", leader, y, detail, cap);
    if (!positionOk) {
        FieldInfo* roleField = FindFieldInHierarchy(leaderClass, "roleData");
        Il2CppObject* roleBacking = nullptr;
        if (roleField) {
            const Il2CppType* roleType = g_api.field_get_type(roleField);
            Il2CppClass* roleFieldClass = roleType ? g_api.class_from_type(roleType) : nullptr;
            // Never copy an unknown/value-type field into a pointer-sized buffer. Donor tells us
            // roleData used to be a reference, but NewCore verifies that assumption at runtime.
            if (roleFieldClass && !g_api.class_is_valuetype(roleFieldClass)) {
                g_api.field_get_value(leader, roleField, &roleBacking);
            }
        }
        if (roleBacking) {
            Il2CppClass* backingClass = g_api.object_get_class(roleBacking);
            if (backingClass) {
                positionOk = ReadScalarGetter(backingClass, "get_PosX", roleBacking, x, detail, cap) &&
                             ReadScalarGetter(backingClass, "get_PosY", roleBacking, y, detail, cap);
            }
        }
    }
    if (positionOk) {
        game.x = x; game.y = y; game.validMask |= ValidPosition;
    }

    Il2CppClass* gameApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    if (!gameApi) { CopyText(detail, cap, L"Thiếu FGStudio.LuaSystem.API.LuaSystemAPI_Game"); return false; }
    std::int32_t enableAutoF1 = 0, freeBag = -1, mapReady = 0, moving = 0;
    if (!ReadStaticScalar(gameApi, "get_EnableAutoF1", enableAutoF1, detail, cap)) return false;
    game.autoFight = enableAutoF1 ? 0 : 1; game.validMask |= ValidAutoFightState;
    if (!ReadStaticScalar(gameApi, "GetFreeBagSpace", freeBag, detail, cap) || freeBag < 0 || freeBag > 10000) return false;
    game.freeBagSpace = freeBag; game.validMask |= ValidBagSpace;
    if (!ReadStaticScalar(gameApi, "IsMapReady", mapReady, detail, cap)) return false;
    game.mapReady = mapReady ? 1 : 0; game.validMask |= ValidMapReadyState;
    if (ReadStaticScalar(gameApi, "IsMoving", moving, detail, cap)) {
        game.moving = moving ? 1 : 0; game.validMask |= ValidMovingState;
    }

    Il2CppClass* sessionData = g_api.class_from_name(image, "FGStudio.Game.Logic", "SessionData");
    if (sessionData) {
        std::int32_t waiting = 0;
        if (ReadStaticScalar(sessionData, "get_WaitingChangeMap", waiting, detail, cap)) {
            game.waitingChangeMap = waiting ? 1 : 0;
            game.validMask |= ValidMapTransitionState;
        }
    }

    static std::uint32_t sequence = 0;
    game.sequence = ++sequence;
    if ((game.validMask & kRequiredGameCoreMask) != kRequiredGameCoreMask) {
        CopyText(detail, cap, L"Snapshot thiếu core field bắt buộc; fail-closed");
        return false;
    }

    ClearText(detail, cap);
    AppendText(detail, cap, L"SNAPSHOT PASS role="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.roleID));
    AppendText(detail, cap, L" map="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.mapID));
    if (game.validMask & ValidPosition) {
        AppendText(detail, cap, L" pos="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.x));
        AppendText(detail, cap, L","); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.y));
    } else {
        AppendText(detail, cap, L" pos=? (không dùng donor offset)");
    }
    AppendText(detail, cap, L" HP="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.hp));
    AppendText(detail, cap, L"/"); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.maxHP));
    AppendText(detail, cap, L" bag="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.freeBagSpace));
    AppendText(detail, cap, L" dead="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.dead));
    AppendText(detail, cap, L" auto="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.autoFight));
    AppendText(detail, cap, L". READ-ONLY; chưa có action.");
    return true;
}

void SetDetail(BridgeResponse& response, const wchar_t* text) {
    CopyText(response.detail, _countof(response.detail), text ? text : L"");
}

bool EnsureShared() {
    if (g_shared) return true;
    wchar_t name[96]{};
    MappingName(GetCurrentProcessId(), name, _countof(name));
    g_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!g_mapping) return false;
    g_shared = reinterpret_cast<SharedBlock*>(MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    if (!g_shared || g_shared->magic != kMagic || g_shared->protocolVersion != kProtocolVersion) {
        if (g_shared) UnmapViewOfFile(g_shared);
        g_shared = nullptr;
        CloseHandle(g_mapping); g_mapping = nullptr;
        return false;
    }
    InterlockedExchange(&g_shared->bridgeLoaded, 1);
    return true;
}

void ProcessRequest() {
    if (!EnsureShared()) return;
    const LONG seq = g_shared->requestSeq;
    if (seq == g_shared->completedSeq || seq <= 0) return;
    if (InterlockedCompareExchange(&g_shared->bridgeBusy, 1, 0) != 0) return;

    BridgeResponse response{};
    FoundationSnapshot snap{};
    GameSnapshot game{};
    wchar_t detail[1024]{};
    bool ok = false;

    const BridgeCommand command = static_cast<BridgeCommand>(g_shared->request.command);
    switch (command) {
        case BridgeCommand::ValidateNative:
            ok = NativeValidate(snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2101; break;
        case BridgeCommand::InspectFgMainThread:
            ok = DescribeType("FGStudio.Engine.Utilities", "MainThread", ValidFgMainThreadType,
                              snap.fgMainThreadMethodCount, snap.fgMainThreadFieldCount,
                              snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2201; break;
        case BridgeCommand::InspectUnityDispatcher:
            ok = DescribeType("", "UnityMainThreadDispatcher", ValidUnityDispatcher,
                              snap.unityDispatcherMethodCount, snap.unityDispatcherFieldCount,
                              snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2301; break;
        case BridgeCommand::ProveUnityMainThread:
            ok = ProveUnityMainThread(snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2401; break;
        case BridgeCommand::ReadGameSnapshot:
            ok = ReadGameSnapshot(snap, game, detail, _countof(detail)); response.errorCode = ok ? 0 : 2501; break;
        default:
            CopyText(detail, _countof(detail), L"Command không hợp lệ"); response.errorCode = 2003; break;
    }

    response.ok = ok ? 1 : 0;
    response.snapshot = snap;
    response.gameSnapshot = game;
    SetDetail(response, detail);
    g_shared->response = response;
    MemoryBarrier();
    InterlockedExchange(&g_shared->completedSeq, seq);
    InterlockedExchange(&g_shared->bridgeBusy, 0);
}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK TlGetMessageHook(int code, WPARAM wParam, LPARAM lParam) {
    (void)wParam;
    if (code >= 0 && lParam) {
        const MSG* msg = reinterpret_cast<const MSG*>(lParam);
        if (msg->message == kWakeMessage) ProcessRequest();
    }
    return CallNextHookEx(nullptr, code, wParam, lParam);
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);
    } else if (reason == DLL_PROCESS_DETACH) {
        if (g_shared) UnmapViewOfFile(g_shared);
        if (g_mapping) CloseHandle(g_mapping);
        g_shared = nullptr;
        g_mapping = nullptr;
    }
    return TRUE;
}
