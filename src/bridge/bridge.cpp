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
    std::uint32_t resolved = 0;
    static constexpr std::uint32_t required = 22;

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
        default:
            CopyText(detail, _countof(detail), L"Command không hợp lệ"); response.errorCode = 2003; break;
    }

    response.ok = ok ? 1 : 0;
    response.snapshot = snap;
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
