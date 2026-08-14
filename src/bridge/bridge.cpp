#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>
#include <sstream>
#include <vector>
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

template <typename T>
bool ResolveProcAddress(HMODULE module, const char* name, T& out) {
    out = nullptr;
    FARPROC raw = GetProcAddress(module, name);
    if (!raw) return false;
    static_assert(sizeof(raw) == sizeof(out), "Windows function pointer size mismatch");
    std::memcpy(&out, &raw, sizeof(out));
    return out != nullptr;
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
    std::uint32_t resolved = 0;
    static constexpr std::uint32_t required = 14;

    bool Load(std::wstring& detail) {
        if (gameAssembly && resolved == required) return true;
        gameAssembly = GetModuleHandleW(L"GameAssembly.dll");
        if (!gameAssembly) {
            detail = L"GameAssembly.dll chưa được load trong client";
            return false;
        }
        resolved = 0;
#define RESOLVE_COUNT(name) do { if (!ResolveProcAddress(gameAssembly, "il2cpp_" #name, name)) { \
            detail = L"Thiếu một IL2CPP metadata export bắt buộc"; return false; } ++resolved; } while (0)
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
        if (!ResolveProcAddress(gameAssembly, "il2cpp_free", free_fn)) {
            detail = L"Thiếu export: il2cpp_free";
            return false;
        }
        ++resolved;
        RESOLVE_COUNT(class_get_fields);
        RESOLVE_COUNT(field_get_name);
        RESOLVE_COUNT(field_get_type);
#undef RESOLVE_COUNT
        detail = L"IL2CPP metadata exports OK";
        return true;
    }
};

Il2CppApi g_api;

std::wstring WidenAscii(const char* text) {
    if (!text) return L"?";
    std::wstring out;
    while (*text) out.push_back(static_cast<unsigned char>(*text++));
    return out;
}

std::wstring TypeName(const Il2CppType* type) {
    if (!type || !g_api.type_get_name) return L"?";
    char* raw = g_api.type_get_name(type);
    if (!raw) return L"?";
    std::wstring out = WidenAscii(raw);
    g_api.free_fn(raw);
    return out;
}

const Il2CppImage* AssemblyCSharpImage() {
    Il2CppDomain* domain = g_api.domain_get ? g_api.domain_get() : nullptr;
    if (!domain) return nullptr;
    const Il2CppAssembly* assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp");
    if (!assembly) assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp.dll");
    return assembly ? g_api.assembly_get_image(assembly) : nullptr;
}

bool NativeValidate(FoundationSnapshot& snap, std::wstring& detail) {
    snap.hookThreadId = GetCurrentThreadId();
    snap.windowThreadId = g_shared ? g_shared->targetWindowThreadId : 0;
    if (!g_shared || g_shared->targetPid != GetCurrentProcessId()) {
        detail = L"Shared mapping/PID không khớp";
        return false;
    }
    if (!snap.windowThreadId || snap.hookThreadId != snap.windowThreadId) {
        detail = L"Hook không chạy trên thread sở hữu cửa sổ game";
        return false;
    }
    snap.validMask |= ValidHookThread;

    std::wstring apiDetail;
    if (!g_api.Load(apiDetail)) {
        snap.resolvedExports = g_api.resolved;
        snap.requiredExports = Il2CppApi::required;
        detail = apiDetail;
        return false;
    }
    snap.resolvedExports = g_api.resolved;
    snap.requiredExports = Il2CppApi::required;
    snap.validMask |= ValidIl2CppExports;

    std::wstringstream ss;
    ss << L"HOOK PASS TID=" << snap.hookThreadId
       << L"; IL2CPP metadata exports " << snap.resolvedExports << L"/" << snap.requiredExports
       << L"; CHƯA gọi managed action invoke; CHƯA xác minh Unity main thread";
    detail = ss.str();
    return true;
}

bool DescribeType(const char* nameSpace, const char* className,
                  std::uint32_t validBit,
                  std::uint32_t& methodCountOut,
                  std::uint32_t& fieldCountOut,
                  FoundationSnapshot& snap,
                  std::wstring& detail) {
    std::wstring nativeDetail;
    if (!NativeValidate(snap, nativeDetail)) {
        detail = nativeDetail;
        return false;
    }
    const Il2CppImage* image = AssemblyCSharpImage();
    if (!image) {
        detail = L"Không resolve được Assembly-CSharp bằng metadata API";
        return false;
    }
    Il2CppClass* klass = g_api.class_from_name(image, nameSpace, className);
    if (!klass) {
        std::wstringstream miss;
        miss << L"Không tìm thấy type " << WidenAscii(nameSpace) << L"." << WidenAscii(className);
        detail = miss.str();
        return false;
    }

    constexpr std::uint32_t METHOD_ATTRIBUTE_STATIC = 0x0010;
    std::vector<std::wstring> methods;
    void* iter = nullptr;
    while (const MethodInfo* method = g_api.class_get_methods(klass, &iter)) {
        ++methodCountOut;
        if (methods.size() >= 24) continue;
        const char* mn = g_api.method_get_name(method);
        const std::uint32_t argc = g_api.method_get_param_count(method);
        std::uint32_t iflags = 0;
        const bool isStatic = (g_api.method_get_flags(method, &iflags) & METHOD_ATTRIBUTE_STATIC) != 0;
        std::wstringstream one;
        one << (isStatic ? L"static " : L"") << WidenAscii(mn) << L"(";
        for (std::uint32_t i = 0; i < argc; ++i) {
            if (i) one << L", ";
            one << TypeName(g_api.method_get_param(method, i));
        }
        one << L")";
        methods.push_back(one.str());
    }

    std::vector<std::wstring> fields;
    iter = nullptr;
    while (FieldInfo* field = g_api.class_get_fields(klass, &iter)) {
        ++fieldCountOut;
        if (fields.size() >= 16) continue;
        std::wstringstream one;
        one << TypeName(g_api.field_get_type(field)) << L" " << WidenAscii(g_api.field_get_name(field));
        fields.push_back(one.str());
    }

    snap.validMask |= validBit;
    std::wstringstream ss;
    if (nameSpace && *nameSpace) ss << WidenAscii(nameSpace) << L".";
    ss << WidenAscii(className) << L": methods=" << methodCountOut << L", fields=" << fieldCountOut;
    if (!methods.empty()) {
        ss << L" | M: ";
        for (std::size_t i = 0; i < methods.size(); ++i) {
            if (i) ss << L"; ";
            ss << methods[i];
        }
    }
    if (!fields.empty()) {
        ss << L" | F: ";
        for (std::size_t i = 0; i < fields.size(); ++i) {
            if (i) ss << L"; ";
            ss << fields[i];
        }
    }
    detail = ss.str();
    return true;
}

void SetDetail(BridgeResponse& response, const std::wstring& text) {
    const std::size_t cap = sizeof(response.detail) / sizeof(response.detail[0]);
    std::wcsncpy(response.detail, text.c_str(), cap - 1);
    response.detail[cap - 1] = L'\0';
}

bool EnsureShared() {
    if (g_shared) return true;
    wchar_t name[96]{};
    MappingName(GetCurrentProcessId(), name, _countof(name));
    g_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!g_mapping) return false;
    g_shared = reinterpret_cast<SharedBlock*>(MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0,
                                                            sizeof(SharedBlock)));
    if (!g_shared || g_shared->magic != kMagic || g_shared->protocolVersion != kProtocolVersion) {
        if (g_shared) UnmapViewOfFile(g_shared);
        g_shared = nullptr;
        CloseHandle(g_mapping);
        g_mapping = nullptr;
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
    std::wstring detail;
    bool ok = false;

    const BridgeCommand command = static_cast<BridgeCommand>(g_shared->request.command);
    switch (command) {
        case BridgeCommand::ValidateNative:
            ok = NativeValidate(snap, detail);
            response.errorCode = ok ? 0 : 2101;
            break;
        case BridgeCommand::InspectFgMainThread:
            ok = DescribeType("FGStudio.Engine.Utilities", "MainThread",
                              ValidFgMainThreadType,
                              snap.fgMainThreadMethodCount,
                              snap.fgMainThreadFieldCount,
                              snap, detail);
            response.errorCode = ok ? 0 : 2201;
            break;
        case BridgeCommand::InspectUnityDispatcher:
            ok = DescribeType("", "UnityMainThreadDispatcher",
                              ValidUnityDispatcher,
                              snap.unityDispatcherMethodCount,
                              snap.unityDispatcherFieldCount,
                              snap, detail);
            response.errorCode = ok ? 0 : 2301;
            break;
        default:
            detail = L"Command không hợp lệ";
            response.errorCode = 2003;
            break;
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
