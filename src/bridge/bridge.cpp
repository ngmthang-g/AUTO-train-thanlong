#include <windows.h>
#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>
#include <algorithm>
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
using Il2CppArray = void;
using Il2CppChar = std::uint16_t;

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
    const char* (__cdecl* class_get_name)(Il2CppClass*) = nullptr;
    Il2CppClass* (__cdecl* class_get_parent)(Il2CppClass*) = nullptr;
    const MethodInfo* (__cdecl* class_get_methods)(Il2CppClass*, void**) = nullptr;
    const MethodInfo* (__cdecl* class_get_method_from_name)(Il2CppClass*, const char*, int) = nullptr;
    const char* (__cdecl* method_get_name)(const MethodInfo*) = nullptr;
    std::uint32_t (__cdecl* method_get_param_count)(const MethodInfo*) = nullptr;
    std::uint32_t (__cdecl* method_get_flags)(const MethodInfo*, std::uint32_t*) = nullptr;
    const Il2CppType* (__cdecl* method_get_param)(const MethodInfo*, std::uint32_t) = nullptr;
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, Il2CppObject**) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    const Il2CppImage* (__cdecl* get_corlib)() = nullptr;
    Il2CppClass* (__cdecl* class_from_name)(const Il2CppImage*, const char*, const char*) = nullptr;
    FieldInfo* (__cdecl* class_get_field_from_name)(Il2CppClass*, const char*) = nullptr;
    Il2CppObject* (__cdecl* string_new)(const char*) = nullptr;
    std::int32_t (__cdecl* string_length)(Il2CppObject*) = nullptr;
    const Il2CppChar* (__cdecl* string_chars)(Il2CppObject*) = nullptr;
    std::uint32_t (__cdecl* gchandle_new)(Il2CppObject*, bool) = nullptr;
    void (__cdecl* gchandle_free)(std::uint32_t) = nullptr;
    Il2CppArray* (__cdecl* array_new)(Il2CppClass*, std::size_t) = nullptr;
    Il2CppObject* (__cdecl* value_box)(Il2CppClass*, void*) = nullptr;
    Il2CppObject* (__cdecl* field_get_value_object)(FieldInfo*, Il2CppObject*) = nullptr;

    bool Load() {
        if (gameAssembly) return true;
        gameAssembly = GetModuleHandleW(L"GameAssembly.dll");
        if (!gameAssembly) return false;
#define RESOLVE(name) if (!ResolveProcAddress(gameAssembly, "il2cpp_" #name, name)) return false
        RESOLVE(domain_get);
        RESOLVE(domain_assembly_open);
        RESOLVE(assembly_get_image);
        RESOLVE(class_get_name);
        RESOLVE(class_get_parent);
        RESOLVE(class_get_methods);
        RESOLVE(class_get_method_from_name);
        RESOLVE(method_get_name);
        RESOLVE(method_get_param_count);
        RESOLVE(method_get_flags);
        RESOLVE(method_get_param);
        RESOLVE(type_get_name);
        if (!ResolveProcAddress(gameAssembly, "il2cpp_free", free_fn)) return false;
        RESOLVE(runtime_invoke);
        RESOLVE(object_unbox);
        RESOLVE(object_get_class);
        RESOLVE(get_corlib);
        RESOLVE(class_from_name);
        RESOLVE(class_get_field_from_name);
        RESOLVE(string_new);
        RESOLVE(string_length);
        RESOLVE(string_chars);
        RESOLVE(gchandle_new);
        RESOLVE(gchandle_free);
        RESOLVE(array_new);
        RESOLVE(value_box);
        RESOLVE(field_get_value_object);
#undef RESOLVE
        return true;
    }
};

Il2CppApi g_api;

class ManagedRoot {
public:
    ManagedRoot() = default;
    explicit ManagedRoot(Il2CppObject* object) { Reset(object); }
    ManagedRoot(const ManagedRoot&) = delete;
    ManagedRoot& operator=(const ManagedRoot&) = delete;
    ~ManagedRoot() { Reset(nullptr); }
    bool Reset(Il2CppObject* object) {
        if (handle_) { g_api.gchandle_free(handle_); handle_ = 0; }
        object_ = object;
        if (object_) handle_ = g_api.gchandle_new(object_, false);
        return !object_ || handle_ != 0;
    }
    Il2CppObject* Get() const { return object_; }
private:
    Il2CppObject* object_ = nullptr;
    std::uint32_t handle_ = 0;
};

const Il2CppImage* ResolveImage(const char* assemblyName) {
    if (!g_api.Load() || !assemblyName) return nullptr;
    Il2CppDomain* domain = g_api.domain_get();
    if (!domain) return nullptr;
    const Il2CppAssembly* assembly = g_api.domain_assembly_open(domain, assemblyName);
    if (!assembly) {
        std::string dllName = std::string(assemblyName) + ".dll";
        assembly = g_api.domain_assembly_open(domain, dllName.c_str());
    }
    return assembly ? g_api.assembly_get_image(assembly) : nullptr;
}

const Il2CppImage* AssemblyCSharpImage() { return ResolveImage("Assembly-CSharp"); }

Il2CppClass* ResolveClass(const char* nameSpace, const char* className) {
    const Il2CppImage* image = AssemblyCSharpImage();
    return image ? g_api.class_from_name(image, nameSpace, className) : nullptr;
}

std::string TypeName(const Il2CppType* type) {
    if (!type || !g_api.type_get_name) return {};
    char* raw = g_api.type_get_name(type);
    if (!raw) return {};
    std::string result(raw);
    g_api.free_fn(raw);
    return result;
}

const MethodInfo* FindClassMethodByParam(Il2CppClass* klass, const char* methodName,
                                         int argc, int paramIndex,
                                         const char* paramTypeContains) {
    if (!klass) return nullptr;
    const MethodInfo* found = nullptr;
    int matches = 0;
    void* iter = nullptr;
    while (const MethodInfo* m = g_api.class_get_methods(klass, &iter)) {
        const char* mn = g_api.method_get_name(m);
        if (!mn || std::strcmp(mn, methodName) != 0) continue;
        if (static_cast<int>(g_api.method_get_param_count(m)) != argc) continue;
        if (paramIndex >= 0) {
            const std::string tn = TypeName(g_api.method_get_param(m, static_cast<std::uint32_t>(paramIndex)));
            if (!paramTypeContains || tn.find(paramTypeContains) == std::string::npos) continue;
        }
        found = m;
        ++matches;
    }
    return matches == 1 ? found : nullptr;
}

bool IsStatic(const MethodInfo* method) {
    if (!method) return false;
    std::uint32_t iflags = 0;
    constexpr std::uint32_t METHOD_ATTRIBUTE_STATIC = 0x0010;
    return (g_api.method_get_flags(method, &iflags) & METHOD_ATTRIBUTE_STATIC) != 0;
}

Il2CppObject* Invoke(const MethodInfo* method, Il2CppObject* instance, void** args,
                     Il2CppObject** exception = nullptr) {
    if (!method || !g_api.runtime_invoke) return nullptr;
    Il2CppObject* localException = nullptr;
    Il2CppObject* result = g_api.runtime_invoke(method, instance, args,
                                                exception ? exception : &localException);
    if (exception ? *exception : localException) return nullptr;
    return result;
}

template <typename T>
bool Unbox(Il2CppObject* object, T& value) {
    if (!object || !g_api.object_unbox) return false;
    void* p = g_api.object_unbox(object);
    if (!p) return false;
    value = *reinterpret_cast<T*>(p);
    return true;
}

bool InvokeStaticBool(const MethodInfo* method, bool& value) {
    value = false;
    if (!method || !IsStatic(method)) return false;
    Il2CppObject* result = Invoke(method, nullptr, nullptr);
    std::uint8_t raw = 0;
    if (!Unbox(result, raw)) return false;
    value = raw != 0;
    return true;
}

bool InvokeStaticInt(const MethodInfo* method, std::int32_t& value) {
    value = 0;
    if (!method || !IsStatic(method)) return false;
    return Unbox(Invoke(method, nullptr, nullptr), value);
}

Il2CppObject* InvokeStaticObject(const MethodInfo* method) {
    if (!method || !IsStatic(method)) return nullptr;
    return Invoke(method, nullptr, nullptr);
}

const MethodInfo* ClassMethodAny(Il2CppClass* klass, const char* const* names, int count, int argc) {
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        for (int i = 0; i < count; ++i) {
            if (const MethodInfo* m = g_api.class_get_method_from_name(current, names[i], argc)) return m;
        }
    }
    return nullptr;
}

FieldInfo* ClassFieldAny(Il2CppClass* klass, const char* name) {
    if (!name) return nullptr;
    for (Il2CppClass* current = klass; current; current = g_api.class_get_parent(current)) {
        if (FieldInfo* field = g_api.class_get_field_from_name(current, name)) return field;
    }
    return nullptr;
}

bool ObjectBoolGetter(Il2CppObject* object, const char* const* names, int count, bool& value) {
    if (!object) return false;
    Il2CppClass* klass = g_api.object_get_class(object);
    const MethodInfo* m = ClassMethodAny(klass, names, count, 0);
    if (!m || IsStatic(m)) return false;
    std::uint8_t raw = 0;
    if (!Unbox(Invoke(m, object, nullptr), raw)) return false;
    value = raw != 0;
    return true;
}

bool ObjectIntGetter(Il2CppObject* object, const char* const* names, int count, std::int32_t& value) {
    if (!object) return false;
    Il2CppClass* klass = g_api.object_get_class(object);
    const MethodInfo* m = ClassMethodAny(klass, names, count, 0);
    if (!m || IsStatic(m)) return false;
    return Unbox(Invoke(m, object, nullptr), value);
}

bool ObjectStringGetter(Il2CppObject* object, const char* const* names, int count,
                        wchar_t* output, std::size_t capacity) {
    if (!object || !output || capacity == 0) return false;
    output[0] = L'\0';
    Il2CppClass* klass = g_api.object_get_class(object);
    const MethodInfo* m = ClassMethodAny(klass, names, count, 0);
    if (!m || IsStatic(m)) return false;
    Il2CppObject* text = Invoke(m, object, nullptr);
    if (!text) return false;
    const std::int32_t length = g_api.string_length(text);
    const Il2CppChar* chars = g_api.string_chars(text);
    if (!chars || length < 0 || length > 4096) return false;
    const std::size_t copy = std::min<std::size_t>(static_cast<std::size_t>(length), capacity - 1);
    for (std::size_t i = 0; i < copy; ++i) output[i] = static_cast<wchar_t>(chars[i]);
    output[copy] = L'\0';
    return true;
}

bool VerifyUnityMainThread(GameSnapshot& snap, std::wstring& detail) {
    snap.windowThreadId = g_shared ? g_shared->targetWindowThreadId : 0;
    snap.mainThreadId = GetCurrentThreadId();
    if (!g_shared || snap.mainThreadId != g_shared->targetWindowThreadId) {
        detail = L"Bridge không chạy trên thread của cửa sổ game";
        return false;
    }

    // Strict proof only. Do not infer that a window thread is Unity's main thread.
    const Il2CppImage* corlib = g_api.get_corlib();
    if (!corlib) { detail = L"Không đọc được corlib để xác minh main thread"; return false; }
    Il2CppClass* sync = g_api.class_from_name(corlib, "System.Threading", "SynchronizationContext");
    if (!sync) { detail = L"Không tìm thấy SynchronizationContext"; return false; }
    const MethodInfo* getCurrent = g_api.class_get_method_from_name(sync, "get_Current", 0);
    if (!getCurrent || !IsStatic(getCurrent)) {
        detail = L"Không resolve SynchronizationContext.Current"; return false;
    }
    Il2CppObject* current = Invoke(getCurrent, nullptr, nullptr);
    if (!current) { detail = L"SynchronizationContext.Current=null; fail-closed"; return false; }
    Il2CppClass* currentClass = g_api.object_get_class(current);
    const char* cn = currentClass ? g_api.class_get_name(currentClass) : nullptr;
    if (!cn || std::strstr(cn, "UnitySynchronizationContext") == nullptr) {
        detail = L"Thread cửa sổ không có UnitySynchronizationContext; fail-closed";
        return false;
    }
    snap.mainThreadProof = 1;
    snap.validMask |= ValidMainThread;
    detail = L"Main thread: UnitySynchronizationContext";
    return true;
}

struct RuntimeCache {
    bool scannerReady = false;
    bool actionReady = false;
    Il2CppClass* gameApiClass = nullptr;
    Il2CppClass* guiApiClass = nullptr;
    Il2CppClass* executorClass = nullptr;
    const MethodInfo* getRoleData = nullptr;
    const MethodInfo* isMapReady = nullptr;
    const MethodInfo* isMoving = nullptr;
    const MethodInfo* getFreeBagSpace = nullptr;
    const MethodInfo* getEnableAutoF1 = nullptr;
    const MethodInfo* clickNpc = nullptr;
    const MethodInfo* findUi = nullptr;
    const MethodInfo* mainFindUi = nullptr;
    const MethodInfo* executorGetInstance = nullptr;
    const MethodInfo* executorExecuteUiObject = nullptr;
    Il2CppClass* systemObjectClass = nullptr;
    Il2CppClass* systemInt32Class = nullptr;
    Il2CppClass* systemArrayClass = nullptr;
    const MethodInfo* systemArraySetValue = nullptr;

    bool InitScanner(std::wstring& detail) {
        if (scannerReady) return true;
        if (!g_api.Load()) { detail = L"Không resolve được IL2CPP exports"; return false; }
        gameApiClass = ResolveClass("FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
        if (!gameApiClass) { detail = L"Không resolve LuaSystemAPI_Game"; return false; }
        getRoleData = g_api.class_get_method_from_name(gameApiClass, "get_RoleData", 0);
        isMapReady = g_api.class_get_method_from_name(gameApiClass, "IsMapReady", 0);
        isMoving = g_api.class_get_method_from_name(gameApiClass, "IsMoving", 0);
        getFreeBagSpace = g_api.class_get_method_from_name(gameApiClass, "GetFreeBagSpace", 0);
        getEnableAutoF1 = g_api.class_get_method_from_name(gameApiClass, "get_EnableAutoF1", 0);
        scannerReady = getRoleData && isMapReady && getFreeBagSpace && getEnableAutoF1;
        if (!scannerReady) {
            detail = L"Scanner resolver thiếu API bắt buộc; fail-closed";
            return false;
        }
        detail = L"Scanner resolver OK";
        return true;
    }

    bool InitActions(std::wstring& detail) {
        if (actionReady) return true;
        if (!InitScanner(detail)) return false;
        guiApiClass = ResolveClass("FGStudio.LuaSystem.API", "LuaSystemAPI_GUI");
        executorClass = ResolveClass("FGStudio.LuaSystem.Logic", "MonoBehaviourExecutor");
        if (!guiApiClass || !executorClass) {
            detail = L"Không resolve GUI/MonoBehaviourExecutor"; return false;
        }
        clickNpc = g_api.class_get_method_from_name(gameApiClass, "ClickNPC", 1);
        findUi = g_api.class_get_method_from_name(guiApiClass, "FindUI", 1);
        mainFindUi = g_api.class_get_method_from_name(guiApiClass, "MainFindUI", 1);
        const char* instanceNames[] = {"get_Instance", "GetInstance"};
        executorGetInstance = ClassMethodAny(executorClass, instanceNames, 2, 0);
        executorExecuteUiObject = FindClassMethodByParam(executorClass, "ExecuteScriptFunction", 4, 0, "UIObject");
        if (!executorExecuteUiObject)
            executorExecuteUiObject = FindClassMethodByParam(executorClass, "ExecuteScriptFunction", 4, -1, nullptr);
        const Il2CppImage* corlib = g_api.get_corlib();
        if (corlib) {
            systemObjectClass = g_api.class_from_name(corlib, "System", "Object");
            systemInt32Class = g_api.class_from_name(corlib, "System", "Int32");
            systemArrayClass = g_api.class_from_name(corlib, "System", "Array");
            systemArraySetValue = FindClassMethodByParam(systemArrayClass, "SetValue", 2, 1, "Int32");
        }
        actionReady = clickNpc && findUi && executorGetInstance && executorExecuteUiObject &&
                      systemObjectClass && systemInt32Class && systemArraySetValue;
        if (!actionReady) {
            detail = L"Action resolver thiếu API; không dùng RVA cứng để bù";
            return false;
        }
        detail = L"Action resolver OK";
        return true;
    }
};

RuntimeCache g_cache;

Il2CppObject* FindUiObject(const char* name) {
    if (!name || !*name || !g_cache.findUi) return nullptr;
    ManagedRoot managedName(g_api.string_new(name));
    if (!managedName.Get()) return nullptr;
    void* arg0 = managedName.Get();
    void* args[1] = { &arg0 };
    Il2CppObject* result = Invoke(g_cache.findUi, nullptr, args);
    if (!result && g_cache.mainFindUi) result = Invoke(g_cache.mainFindUi, nullptr, args);
    return result;
}

bool InvokeLuaInt(const char* uiName, const char* functionName, std::int32_t value,
                  std::wstring& detail) {
    Il2CppObject* ui = FindUiObject(uiName);
    if (!ui) { detail = L"Lua UI chưa tồn tại"; return false; }
    Il2CppObject* executor = InvokeStaticObject(g_cache.executorGetInstance);
    if (!executor) { detail = L"MonoBehaviourExecutor chưa sẵn sàng"; return false; }
    ManagedRoot uiRoot(ui);
    ManagedRoot function(g_api.string_new(functionName));
    ManagedRoot boxed(g_api.value_box(g_cache.systemInt32Class, &value));
    Il2CppArray* rawArray = g_api.array_new(g_cache.systemObjectClass, 1);
    ManagedRoot arrayRoot(reinterpret_cast<Il2CppObject*>(rawArray));
    if (!uiRoot.Get() || !function.Get() || !boxed.Get() || !rawArray || !arrayRoot.Get()) {
        detail = L"Không tạo/root được Lua args"; return false;
    }
    // Populate Object[] through System.Array.SetValue instead of writing an IL2CPP layout offset.
    Il2CppObject* boxedArg = boxed.Get();
    std::int32_t index = 0;
    void* setArgs[2] = { &boxedArg, &index };
    Il2CppObject* setException = nullptr;
    g_api.runtime_invoke(g_cache.systemArraySetValue, reinterpret_cast<Il2CppObject*>(rawArray),
                         setArgs, &setException);
    if (setException) { detail = L"System.Array.SetValue ném exception"; return false; }
    Il2CppObject* uiArg = uiRoot.Get();
    Il2CppObject* fnArg = function.Get();
    Il2CppArray* arrayArg = rawArray;
    std::uint8_t falseArg = 0;
    void* args[4] = { &uiArg, &fnArg, &arrayArg, &falseArg };
    Il2CppObject* exception = nullptr;
    g_api.runtime_invoke(g_cache.executorExecuteUiObject, executor, args, &exception);
    if (exception) { detail = L"Lua action ném exception"; return false; }
    detail = L"Lua action đã gửi trên main thread";
    return true;
}

bool ClickNpc(std::int32_t resId, std::wstring& detail) {
    if (!g_cache.clickNpc || !IsStatic(g_cache.clickNpc) || resId <= 0) {
        detail = L"ClickNPC chưa resolve hoặc ResID sai";
        return false;
    }
    std::int32_t arg = resId;
    void* args[1] = { &arg };
    Il2CppObject* exception = nullptr;
    g_api.runtime_invoke(g_cache.clickNpc, nullptr, args, &exception);
    if (exception) { detail = L"ClickNPC ném exception"; return false; }
    detail = L"ClickNPC đã gửi trên main thread";
    return true;
}

bool BuildSnapshot(GameSnapshot& snap, std::wstring& detail) {
    snap = {};
    if (!g_cache.InitScanner(detail)) return false;
    if (!VerifyUnityMainThread(snap, detail)) return false;

    bool b = false;
    if (InvokeStaticBool(g_cache.isMapReady, b)) {
        snap.mapReady = b ? 1 : 0;
        snap.validMask |= ValidMapReady;
    }
    if (g_cache.isMoving && InvokeStaticBool(g_cache.isMoving, b)) {
        snap.moving = b ? 1 : 0;
        snap.validMask |= ValidMoving;
    }
    std::int32_t value = 0;
    if (g_cache.getFreeBagSpace && InvokeStaticInt(g_cache.getFreeBagSpace, value)) {
        snap.freeBagSpace = value;
        snap.validMask |= ValidBag;
    }
    bool enableAutoF1 = true;
    if (InvokeStaticBool(g_cache.getEnableAutoF1, enableAutoF1)) {
        // Donor/source analysis proved EnableAutoF1=false means Train is active.
        snap.autoFight = enableAutoF1 ? 0 : 1;
        snap.validMask |= ValidAutoFight;
    }

    Il2CppObject* role = InvokeStaticObject(g_cache.getRoleData);
    if (role) {
        // Read public properties through IL2CPP methods; do not carry object pointers outside this probe.
        const char* roleIdNames[] = {"get_RoleID", "GetRoleID", "get_ID", "GetID"};
        if (ObjectIntGetter(role, roleIdNames, 4, snap.roleId)) snap.validMask |= ValidRole;
        const char* nameNames[] = {"get_Name", "GetName"};
        ObjectStringGetter(role, nameNames, 2, snap.characterName,
                           sizeof(snap.characterName) / sizeof(snap.characterName[0]));

        const char* deadNames[] = {"get_IsDeath", "IsDeath", "GetIsDeath", "get_Dead"};
        if (ObjectBoolGetter(role, deadNames, 4, b)) {
            snap.dead = b ? 1 : 0;
            snap.validMask |= ValidDead;
        }

        const char* ridingNames[] = {"get_IsRiding", "IsRiding"};
        if (ObjectBoolGetter(role, ridingNames, 2, b)) {
            snap.riding = b ? 1 : 0;
            snap.validMask |= ValidRiding;
        }

        const char* mapNames[] = {"get_MapID", "GetMapID"};
        if (ObjectIntGetter(role, mapNames, 2, value) && value > 0 && value < 100000) {
            snap.mapId = value;
            snap.validMask |= ValidMapId;
            // Position is optional: try the nested roleData object by reflected field name,
            // then reflected PosX/PosY accessors. No hardcoded object/field offset is used.
            Il2CppClass* roleClass = g_api.object_get_class(role);
            FieldInfo* roleDataField = roleClass ? ClassFieldAny(roleClass, "roleData") : nullptr;
            Il2CppObject* roleData = roleDataField ? g_api.field_get_value_object(roleDataField, role) : nullptr;
            std::int32_t px = 0, py = 0;
            const char* posXNames[] = {"get_PosX", "GetPosX", "get_X", "GetX"};
            const char* posYNames[] = {"get_PosY", "GetPosY", "get_Y", "GetY"};
            if (roleData && ObjectIntGetter(roleData, posXNames, 4, px) &&
                ObjectIntGetter(roleData, posYNames, 4, py) &&
                px > -10000000 && px < 10000000 && py > -10000000 && py < 10000000) {
                snap.x = px; snap.y = py;
                snap.validMask |= ValidPosition;
            }
        }
    }
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
    std::wstring detail;
    GameSnapshot snap{};
    bool ok = false;

    if (!g_api.Load()) {
        detail = L"Không load được IL2CPP API";
        response.errorCode = 1001;
    } else {
        const BridgeCommand command = static_cast<BridgeCommand>(g_shared->request.command);
        if (command == BridgeCommand::Probe) {
            ok = BuildSnapshot(snap, detail);
            response.errorCode = ok ? 0 : 1101;
        } else if (!VerifyUnityMainThread(snap, detail)) {
            response.errorCode = 1002;
        } else switch (command) {
            case BridgeCommand::StartAutoFight: {
                if (!g_cache.InitActions(detail)) { response.errorCode = 1200; break; }
                std::wstring actionDetail;
                ok = InvokeLuaInt("AutoFight_Main", "StartAutoFight", 1, actionDetail);
                response.errorCode = ok ? 0 : 1201;
                std::wstring snapshotDetail;
                BuildSnapshot(snap, snapshotDetail);
                detail = actionDetail + L" | " + snapshotDetail;
                break;
            }
            case BridgeCommand::StopAutoFight: {
                if (!g_cache.InitActions(detail)) { response.errorCode = 1200; break; }
                std::wstring actionDetail;
                ok = InvokeLuaInt("AutoFight_Main", "StartAutoFight", 0, actionDetail);
                response.errorCode = ok ? 0 : 1202;
                std::wstring snapshotDetail;
                BuildSnapshot(snap, snapshotDetail);
                detail = actionDetail + L" | " + snapshotDetail;
                break;
            }
            case BridgeCommand::ClickNpc: {
                if (!g_cache.InitActions(detail)) { response.errorCode = 1200; break; }
                std::wstring actionDetail;
                ok = ClickNpc(g_shared->request.intArg0, actionDetail);
                response.errorCode = ok ? 0 : 1203;
                std::wstring snapshotDetail;
                BuildSnapshot(snap, snapshotDetail);
                detail = actionDetail + L" | " + snapshotDetail;
                break;
            }
            default:
                detail = L"Command không hợp lệ";
                response.errorCode = 1003;
                break;
        }
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
        g_shared = nullptr; g_mapping = nullptr;
    }
    return TRUE;
}
