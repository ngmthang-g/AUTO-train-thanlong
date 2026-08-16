#include <windows.h>
#include <cstdint>
#include <cstddef>
#include <climits>
#include <algorithm>
#include <array>
#include <cwctype>
#include <string>
#include <vector>
#include "protocol.h"

using namespace cleanroute;

namespace {

using Il2CppDomain = void;
using Il2CppAssembly = void;
using Il2CppImage = void;
using Il2CppClass = void;
using MethodInfo = void;
using FieldInfo = void;
using Il2CppType = void;
using Il2CppObject = void;
using Il2CppString = void;

HANDLE g_mapping = nullptr;
SharedBlock* g_shared = nullptr;

template <typename T>
bool Resolve(HMODULE module, const char* name, T& out) {
    out = nullptr;
    FARPROC p = GetProcAddress(module, name);
    if (!p) return false;
    static_assert(sizeof(p) == sizeof(out), "pointer-size mismatch");
    const unsigned char* s = reinterpret_cast<const unsigned char*>(&p);
    unsigned char* d = reinterpret_cast<unsigned char*>(&out);
    for (std::size_t i = 0; i < sizeof(out); ++i) d[i] = s[i];
    return out != nullptr;
}

bool Eq(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) { if (*a++ != *b++) return false; }
    return *a == *b;
}

void SetText(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || cap == 0) return;
    std::size_t i = 0;
    if (text) while (i + 1 < cap && text[i]) { out[i] = text[i]; ++i; }
    out[i] = 0;
}

void Append(wchar_t* out, std::size_t cap, const wchar_t* text) {
    if (!out || !text || cap == 0) return;
    std::size_t n = 0; while (n + 1 < cap && out[n]) ++n;
    std::size_t i = 0; while (n + 1 < cap && text[i]) out[n++] = text[i++];
    out[n] = 0;
}

void AppendInt(wchar_t* out, std::size_t cap, int value) {
    wchar_t tmp[32]{}; wsprintfW(tmp, L"%d", value); Append(out, cap, tmp);
}

struct Api {
    HMODULE module = nullptr;
    Il2CppDomain* (__cdecl* domain_get)() = nullptr;
    const Il2CppAssembly* (__cdecl* domain_assembly_open)(Il2CppDomain*, const char*) = nullptr;
    const Il2CppImage* (__cdecl* assembly_get_image)(const Il2CppAssembly*) = nullptr;
    Il2CppClass* (__cdecl* class_from_name)(const Il2CppImage*, const char*, const char*) = nullptr;
    const MethodInfo* (__cdecl* class_get_method_from_name)(Il2CppClass*, const char*, int) = nullptr;
    Il2CppClass* (__cdecl* class_get_parent)(Il2CppClass*) = nullptr;
    bool (__cdecl* class_is_assignable_from)(Il2CppClass*, Il2CppClass*) = nullptr;
    std::uint32_t (__cdecl* method_get_flags)(const MethodInfo*, std::uint32_t*) = nullptr;
    std::uint32_t (__cdecl* method_get_param_count)(const MethodInfo*) = nullptr;
    const Il2CppType* (__cdecl* method_get_param)(const MethodInfo*, std::uint32_t) = nullptr;
    const Il2CppType* (__cdecl* method_get_return_type)(const MethodInfo*) = nullptr;
    char* (__cdecl* type_get_name)(const Il2CppType*) = nullptr;
    void (__cdecl* free_fn)(void*) = nullptr;
    Il2CppObject* (__cdecl* runtime_invoke)(const MethodInfo*, void*, void**, void**) = nullptr;
    void* (__cdecl* object_unbox)(Il2CppObject*) = nullptr;
    Il2CppClass* (__cdecl* object_get_class)(Il2CppObject*) = nullptr;
    FieldInfo* (__cdecl* class_get_field_from_name)(Il2CppClass*, const char*) = nullptr;
    const Il2CppType* (__cdecl* field_get_type)(FieldInfo*) = nullptr;
    void (__cdecl* field_get_value)(Il2CppObject*, FieldInfo*, void*) = nullptr;
    void (__cdecl* field_static_get_value)(FieldInfo*, void*) = nullptr;
    Il2CppClass* (__cdecl* class_from_type)(const Il2CppType*) = nullptr;
    bool (__cdecl* class_is_valuetype)(const Il2CppClass*) = nullptr;
    std::int32_t (__cdecl* string_length)(Il2CppString*) = nullptr;
    const wchar_t* (__cdecl* string_chars)(Il2CppString*) = nullptr;
    Il2CppString* (__cdecl* string_new)(const char*) = nullptr;
    std::uint32_t (__cdecl* gchandle_new)(Il2CppObject*, bool) = nullptr;
    Il2CppObject* (__cdecl* gchandle_get_target)(std::uint32_t) = nullptr;

    bool Load(wchar_t* detail, std::size_t cap) {
        if (module) return true;
        module = GetModuleHandleW(L"GameAssembly.dll");
        if (!module) { SetText(detail, cap, L"GameAssembly.dll chưa sẵn sàng"); return false; }
#define NEED(symbol) do { if (!Resolve(module, "il2cpp_" #symbol, symbol)) { module = nullptr; SetText(detail, cap, L"Thiếu IL2CPP export bắt buộc"); return false; } } while (0)
        NEED(domain_get); NEED(domain_assembly_open); NEED(assembly_get_image); NEED(class_from_name);
        NEED(class_get_method_from_name); NEED(class_get_parent); NEED(method_get_flags);
        NEED(method_get_param_count); NEED(method_get_param); NEED(method_get_return_type);
        NEED(type_get_name); NEED(runtime_invoke); NEED(object_unbox); NEED(object_get_class);
        NEED(class_get_field_from_name); NEED(field_get_type); NEED(field_get_value);
        NEED(class_from_type); NEED(class_is_valuetype); NEED(string_length); NEED(string_chars);
#undef NEED
        if (!Resolve(module, "il2cpp_free", free_fn)) { module = nullptr; SetText(detail, cap, L"Thiếu il2cpp_free"); return false; }
        // Optional donor-only UI exports. Route core must still work if a different
        // client lacks them; in that case ValidConfirmUi stays unset and actions fail closed.
        (void)Resolve(module, "il2cpp_class_is_assignable_from", class_is_assignable_from);
        (void)Resolve(module, "il2cpp_field_static_get_value", field_static_get_value);
        (void)Resolve(module, "il2cpp_string_new", string_new);
        (void)Resolve(module, "il2cpp_gchandle_new", gchandle_new);
        (void)Resolve(module, "il2cpp_gchandle_get_target", gchandle_get_target);
        return true;
    }
};

Api g_api;

const Il2CppImage* Image() {
    Il2CppDomain* domain = g_api.domain_get ? g_api.domain_get() : nullptr;
    if (!domain) return nullptr;
    const Il2CppAssembly* assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp");
    if (!assembly) assembly = g_api.domain_assembly_open(domain, "Assembly-CSharp.dll");
    return assembly ? g_api.assembly_get_image(assembly) : nullptr;
}

bool StaticMethod(const MethodInfo* method) {
    if (!method) return false;
    constexpr std::uint32_t StaticFlag = 0x0010;
    std::uint32_t iflags = 0;
    return (g_api.method_get_flags(method, &iflags) & StaticFlag) != 0;
}

const MethodInfo* FindMethod(Il2CppClass* klass, const char* name, int argc) {
    for (Il2CppClass* c = klass; c; c = g_api.class_get_parent(c)) {
        if (const MethodInfo* m = g_api.class_get_method_from_name(c, name, argc)) return m;
    }
    return nullptr;
}

bool ParamType(const MethodInfo* m, std::uint32_t index, const char* expected) {
    if (!m || index >= g_api.method_get_param_count(m)) return false;
    const Il2CppType* t = g_api.method_get_param(m, index);
    char* n = t ? g_api.type_get_name(t) : nullptr;
    if (!n) return false;
    bool ok = Eq(n, expected);
    g_api.free_fn(n);
    return ok;
}

const MethodInfo* ExactMethod(Il2CppClass* klass, const char* name, int argc, bool isStatic,
                              const char* p0 = nullptr, const char* p1 = nullptr, const char* p2 = nullptr) {
    const MethodInfo* m = FindMethod(klass, name, argc);
    if (!m || StaticMethod(m) != isStatic) return nullptr;
    if (argc > 0 && p0 && !ParamType(m, 0, p0)) return nullptr;
    if (argc > 1 && p1 && !ParamType(m, 1, p1)) return nullptr;
    if (argc > 2 && p2 && !ParamType(m, 2, p2)) return nullptr;
    return m;
}

bool InvokeObjectArgs(const MethodInfo* method, void* instance, void** args,
                      Il2CppObject*& out, wchar_t* detail, std::size_t cap) {
    out = nullptr;
    if (!method) { SetText(detail, cap, L"Method object chưa resolve"); return false; }
    void* exc = nullptr;
    out = g_api.runtime_invoke(method, instance, args, &exc);
    if (exc) { SetText(detail, cap, L"Managed exception ở object getter"); return false; }
    return true;
}

bool InvokeObject(const MethodInfo* method, void* instance, Il2CppObject*& out, wchar_t* detail, std::size_t cap) {
    return InvokeObjectArgs(method, instance, nullptr, out, detail, cap);
}

bool InvokeScalar(const MethodInfo* method, void* instance, std::int64_t& out, wchar_t* detail, std::size_t cap) {
    out = 0;
    if (!method) { SetText(detail, cap, L"Scalar method chưa resolve"); return false; }
    const Il2CppType* rt = g_api.method_get_return_type(method);
    char* tn = rt ? g_api.type_get_name(rt) : nullptr;
    if (!tn) { SetText(detail, cap, L"Không đọc được return type"); return false; }
    void* exc = nullptr;
    Il2CppObject* boxed = g_api.runtime_invoke(method, instance, nullptr, &exc);
    if (exc || !boxed) { g_api.free_fn(tn); SetText(detail, cap, L"Scalar getter lỗi/null"); return false; }
    void* raw = g_api.object_unbox(boxed);
    if (!raw) { g_api.free_fn(tn); SetText(detail, cap, L"Không unbox scalar"); return false; }
    bool ok = true;
    if (Eq(tn, "System.Boolean")) out = *reinterpret_cast<const std::uint8_t*>(raw) ? 1 : 0;
    else if (Eq(tn, "System.Int32")) out = *reinterpret_cast<const std::int32_t*>(raw);
    else if (Eq(tn, "System.UInt32")) out = *reinterpret_cast<const std::uint32_t*>(raw);
    else if (Eq(tn, "System.Int64")) out = *reinterpret_cast<const std::int64_t*>(raw);
    else ok = false;
    g_api.free_fn(tn);
    if (!ok) SetText(detail, cap, L"Return type scalar chưa hỗ trợ");
    return ok;
}

bool ScalarGetter(Il2CppClass* klass, const char* name, void* instance, std::int32_t& out,
                  wchar_t* detail, std::size_t cap) {
    std::int64_t value = 0;
    if (!InvokeScalar(FindMethod(klass, name, 0), instance, value, detail, cap)) return false;
    if (value < INT32_MIN || value > INT32_MAX) { SetText(detail, cap, L"Scalar vượt Int32"); return false; }
    out = static_cast<std::int32_t>(value);
    return true;
}

bool StaticScalar(Il2CppClass* klass, const char* name, std::int32_t& out,
                  wchar_t* detail, std::size_t cap) {
    const MethodInfo* m = FindMethod(klass, name, 0);
    if (!m || !StaticMethod(m)) { SetText(detail, cap, L"Static getter chưa resolve"); return false; }
    return ScalarGetter(klass, name, nullptr, out, detail, cap);
}

bool InvokeVoid(const MethodInfo* method, void* instance, void** args,
                wchar_t* detail, std::size_t cap) {
    if (!method) { SetText(detail, cap, L"Action method chưa resolve"); return false; }
    void* exc = nullptr;
    (void)g_api.runtime_invoke(method, instance, args, &exc);
    if (exc) { SetText(detail, cap, L"Action ném managed exception"); return false; }
    return true;
}

bool CopyString(Il2CppString* value, wchar_t* out, std::size_t cap) {
    if (!value || !out || cap == 0) return false;
    const int len = g_api.string_length(value);
    const wchar_t* chars = g_api.string_chars(value);
    if (len < 0 || len > 4096 || !chars) return false;
    std::size_t n = static_cast<std::size_t>(len);
    if (n + 1 > cap) n = cap - 1;
    for (std::size_t i = 0; i < n; ++i) out[i] = chars[i];
    out[n] = 0;
    return true;
}


// v0.8.7 donor UI path. These RVAs are used ONLY for the two runtime-proven
// UIButton actions requested by the user: map Confirm and death-overlay Revive.
// Every raw call is gated by the donor PE identity + byte signatures so a different
// GameAssembly build fails closed instead of calling a wrong address.
namespace donor087 {
constexpr std::uint64_t LuaFindUI = 0x6A5DF0;
constexpr std::uint64_t LuaMainFindUI = 0x6A5F90;
constexpr std::uint64_t UIObjectGetName = 0x530240;
constexpr std::uint64_t UIObjectActiveInHierarchy = 0x52F7D0;
constexpr std::uint64_t UIObjectCoreChildren = 0x52FB80;
constexpr std::uint64_t UIButtonGetInteractable = 0x52E120;
constexpr std::uint64_t UIButtonGetText = 0x52E230;
constexpr std::uint64_t UIButtonGetClickHandler = 0x52DF50;
constexpr std::uint64_t UIButtonHandleClick = 0x52D140;
constexpr std::uint32_t TimeDateStamp = 0x6A410C14u;
constexpr std::uint32_t SizeOfImage = 0x03DCB000u;
constexpr std::uintptr_t DictionaryEntries = 0x18;
constexpr std::uintptr_t DictionaryCount = 0x20;
constexpr std::uintptr_t ArrayLength = 0x18;
constexpr std::uintptr_t ArrayData = 0x20;
constexpr std::uintptr_t EntrySize = 0x18;
constexpr std::uintptr_t EntryValue = 0x10;
}

bool Readable(const void* ptr, std::size_t bytes) {
    if (!ptr || bytes == 0) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)) != sizeof(mbi)) return false;
    if (mbi.State != MEM_COMMIT || (mbi.Protect & PAGE_GUARD) || (mbi.Protect & PAGE_NOACCESS)) return false;
    const auto begin = reinterpret_cast<std::uintptr_t>(ptr);
    const auto region = reinterpret_cast<std::uintptr_t>(mbi.BaseAddress);
    return begin >= region && begin + bytes >= begin && begin + bytes <= region + mbi.RegionSize;
}

template <typename T>
bool ReadLocal(std::uintptr_t address, T& out) {
    const void* p = reinterpret_cast<const void*>(address);
    if (!Readable(p, sizeof(T))) return false;
    out = *reinterpret_cast<const T*>(p);
    return true;
}

using DonorRaw5 = std::uint64_t (*)(std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t, std::uint64_t);

std::uint64_t DonorCall(std::uint64_t rva, std::uint64_t a0 = 0, std::uint64_t a1 = 0,
                        std::uint64_t a2 = 0, std::uint64_t a3 = 0, std::uint64_t a4 = 0) {
    const auto base = reinterpret_cast<std::uintptr_t>(g_api.module);
    const auto fn = reinterpret_cast<DonorRaw5>(base + static_cast<std::uintptr_t>(rva));
    return fn(a0, a1, a2, a3, a4);
}

bool ValidateDonorUi(wchar_t* detail, std::size_t cap) {
    static int cached = 0; // 1 pass, -1 fail
    if (cached != 0) {
        if (cached < 0) SetText(detail, cap, L"UI donor v0.8.7 không khớp GameAssembly; chặn action nội bộ");
        return cached > 0;
    }
    // Module/IL2CPP readiness can be transient during client startup. Do not cache
    // that as a permanent donor mismatch; simply fail this action and retry later.
    if (!g_api.Load(detail, cap) || !g_api.module) return false;
    const auto base = reinterpret_cast<std::uintptr_t>(g_api.module);
    IMAGE_DOS_HEADER dos{};
    if (!ReadLocal(base, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE) {
        SetText(detail, cap, L"GameAssembly PE chưa đọc ổn định; chặn action nội bộ lần này");
        return false;
    }
    IMAGE_NT_HEADERS64 nt{};
    if (!ReadLocal(base + static_cast<std::uintptr_t>(dos.e_lfanew), nt)) {
        SetText(detail, cap, L"GameAssembly NT header chưa readable; chặn action nội bộ lần này");
        return false;
    }
    if (nt.Signature != IMAGE_NT_SIGNATURE || nt.FileHeader.TimeDateStamp != donor087::TimeDateStamp ||
        nt.OptionalHeader.SizeOfImage != donor087::SizeOfImage) {
        cached = -1;
        SetText(detail, cap, L"Sai build GameAssembly so với donor v0.8.7; không gọi UIButton RVA");
        return false;
    }
    struct Sig { std::uint64_t rva; std::array<unsigned char, 12> bytes; };
    static constexpr Sig sigs[] = {
        {donor087::LuaFindUI, {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xBF,0x17,0x12,0x03}},
        {donor087::LuaMainFindUI, {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x1F,0x16,0x12,0x03}},
        {donor087::UIObjectGetName, {0x48,0x83,0xEC,0x28,0x48,0x8B,0x49,0x30,0x48,0x85,0xC9,0x74}},
        {donor087::UIObjectActiveInHierarchy, {0x48,0x83,0xEC,0x28,0x48,0x8B,0x49,0x30,0x48,0x85,0xC9,0x74}},
        {donor087::UIObjectCoreChildren, {0x48,0x89,0x5C,0x24,0x10,0x56,0x57,0x41,0x54,0x41,0x56,0x41}},
        {donor087::UIButtonGetInteractable, {0x48,0x83,0xEC,0x28,0x48,0x8B,0x81,0xE0,0x00,0x00,0x00,0x48}},
        {donor087::UIButtonGetText, {0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83,0xEC,0x20,0x80,0x3D}},
        {donor087::UIButtonGetClickHandler, {0x48,0x8B,0x81,0x00,0x01,0x00,0x00,0xC3,0xCC,0xCC,0xCC,0xCC}},
        {donor087::UIButtonHandleClick, {0x40,0x53,0x48,0x83,0xEC,0x30,0x80,0x3D,0x4B,0xA4,0x29,0x03}},
    };
    for (const auto& sig : sigs) {
        std::array<unsigned char, 12> actual{};
        const void* p = reinterpret_cast<const void*>(base + static_cast<std::uintptr_t>(sig.rva));
        if (!Readable(p, actual.size())) {
            SetText(detail, cap, L"GameAssembly signature chưa readable; chặn action nội bộ lần này");
            return false;
        }
        std::copy_n(reinterpret_cast<const unsigned char*>(p), actual.size(), actual.begin());
        if (actual != sig.bytes) {
            cached = -1;
            SetText(detail, cap, L"Chữ ký UIButton donor v0.8.7 không khớp; chặn action để tránh diss");
            return false;
        }
    }
    cached = 1;
    return true;
}

wchar_t FoldVietnameseChar(wchar_t c) {
    switch (c) {
        case L'À': case L'Á': case L'Ạ': case L'Ả': case L'Ã': case L'Â': case L'Ầ': case L'Ấ': case L'Ậ': case L'Ẩ': case L'Ẫ': case L'Ă': case L'Ằ': case L'Ắ': case L'Ặ': case L'Ẳ': case L'Ẵ':
        case L'à': case L'á': case L'ạ': case L'ả': case L'ã': case L'â': case L'ầ': case L'ấ': case L'ậ': case L'ẩ': case L'ẫ': case L'ă': case L'ằ': case L'ắ': case L'ặ': case L'ẳ': case L'ẵ': return L'a';
        case L'È': case L'É': case L'Ẹ': case L'Ẻ': case L'Ẽ': case L'Ê': case L'Ề': case L'Ế': case L'Ệ': case L'Ể': case L'Ễ':
        case L'è': case L'é': case L'ẹ': case L'ẻ': case L'ẽ': case L'ê': case L'ề': case L'ế': case L'ệ': case L'ể': case L'ễ': return L'e';
        case L'Ì': case L'Í': case L'Ị': case L'Ỉ': case L'Ĩ': case L'ì': case L'í': case L'ị': case L'ỉ': case L'ĩ': return L'i';
        case L'Ò': case L'Ó': case L'Ọ': case L'Ỏ': case L'Õ': case L'Ô': case L'Ồ': case L'Ố': case L'Ộ': case L'Ổ': case L'Ỗ': case L'Ơ': case L'Ờ': case L'Ớ': case L'Ợ': case L'Ở': case L'Ỡ':
        case L'ò': case L'ó': case L'ọ': case L'ỏ': case L'õ': case L'ô': case L'ồ': case L'ố': case L'ộ': case L'ổ': case L'ỗ': case L'ơ': case L'ờ': case L'ớ': case L'ợ': case L'ở': case L'ỡ': return L'o';
        case L'Ù': case L'Ú': case L'Ụ': case L'Ủ': case L'Ũ': case L'Ư': case L'Ừ': case L'Ứ': case L'Ự': case L'Ử': case L'Ữ':
        case L'ù': case L'ú': case L'ụ': case L'ủ': case L'ũ': case L'ư': case L'ừ': case L'ứ': case L'ự': case L'ử': case L'ữ': return L'u';
        case L'Ỳ': case L'Ý': case L'Ỵ': case L'Ỷ': case L'Ỹ': case L'ỳ': case L'ý': case L'ỵ': case L'ỷ': case L'ỹ': return L'y';
        case L'Đ': case L'đ': case L'Ð': case L'ð': return L'd';
        default:
            if (c >= 0x0300 && c <= 0x036F) return 0;
            return static_cast<wchar_t>(towlower(c));
    }
}

std::wstring DonorTrim(std::wstring value) {
    while (!value.empty() && iswspace(value.front())) value.erase(value.begin());
    while (!value.empty() && iswspace(value.back())) value.pop_back();
    return value;
}

std::wstring DonorLower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return value;
}

bool DonorContainsAny(const std::wstring& value, const std::vector<std::wstring>& needles) {
    for (const auto& needle : needles)
        if (value.find(needle) != std::wstring::npos) return true;
    return false;
}

std::wstring CompactMatch(const std::wstring& value) {
    std::wstring out;
    out.reserve(value.size());
    for (wchar_t original : value) {
        const wchar_t c = FoldVietnameseChar(original);
        if ((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9')) out.push_back(c);
    }
    return out;
}

std::wstring DonorString(std::uint64_t pointer) {
    wchar_t buffer[256]{};
    if (!pointer || !CopyString(reinterpret_cast<Il2CppString*>(pointer), buffer, _countof(buffer))) return {};
    return buffer;
}

struct DonorUiRuntime {
    Il2CppClass* uiObject = nullptr;
    Il2CppClass* uiButton = nullptr;
    FieldInfo* instances = nullptr;
};

bool ResolveDonorUiRuntime(DonorUiRuntime& ui, wchar_t* detail, std::size_t cap) {
    if (!ValidateDonorUi(detail, cap)) return false;
    if (!g_api.class_is_assignable_from || !g_api.field_static_get_value) {
        SetText(detail, cap, L"Thiếu IL2CPP export donor UI; action nội bộ fail-closed");
        return false;
    }
    const Il2CppImage* image = Image();
    if (!image) { SetText(detail, cap, L"Không mở được Assembly-CSharp cho UI donor"); return false; }
    ui.uiObject = g_api.class_from_name(image, "FGStudio.LuaSystem.Base", "UIObject");
    ui.uiButton = g_api.class_from_name(image, "FGStudio.LuaSystem.GUI", "UIButton");
    if (!ui.uiObject || !ui.uiButton) { SetText(detail, cap, L"Không resolve được UIObject/UIButton donor"); return false; }
    ui.instances = g_api.class_get_field_from_name(ui.uiObject, "instances");
    if (!ui.instances) { SetText(detail, cap, L"Không resolve được UIObject.instances"); return false; }
    return true;
}

bool IsUiButton(const DonorUiRuntime& ui, std::uint64_t object) {
    // Match the v0.8.7 donor's stale-object guard before touching object_get_class.
    // UIObject.instances can retain disposed entries; calling a virtual/runtime API on
    // one of those is exactly the kind of mismatch that can destabilize the client.
    std::uint8_t disposed = 1;
    if (!object || !ReadLocal(static_cast<std::uintptr_t>(object) + 0x60u, disposed) || disposed != 0) return false;
    Il2CppClass* klass = nullptr;
    if (!ReadLocal(static_cast<std::uintptr_t>(object), klass) || !klass) return false;
    return klass == ui.uiButton || g_api.class_is_assignable_from(ui.uiButton, klass);
}

bool InspectDonorButton(std::uint64_t button, std::wstring& name, std::wstring& text) {
    name.clear(); text.clear();
    if (!button) return false;
    if ((DonorCall(donor087::UIObjectActiveInHierarchy, button) & 0xFFu) == 0) return false;
    if ((DonorCall(donor087::UIButtonGetInteractable, button) & 0xFFu) == 0) return false;
    name = DonorString(DonorCall(donor087::UIObjectGetName, button));
    text = DonorString(DonorCall(donor087::UIButtonGetText, button));
    return true;
}

struct DonorButtonInfo {
    std::uint64_t object = 0;
    std::wstring name;
    std::wstring text;
    std::wstring descendants;
    std::wstring label;
    std::wstring nameKey;
    std::wstring textKey;
    std::wstring allKey;
};

void CollectDonorDescendantLabels(std::uint64_t root, std::wstring& output) {
    output.clear();
    DonorUiRuntime ui{};
    wchar_t ignored[96]{};
    if (!ResolveDonorUiRuntime(ui, ignored, _countof(ignored))) return;
    std::vector<std::uint64_t> pending{root};
    std::vector<std::uint64_t> visited;
    while (!pending.empty() && visited.size() < 96) {
        const std::uint64_t current = pending.back();
        pending.pop_back();
        if (!current || std::find(visited.begin(), visited.end(), current) != visited.end()) continue;
        visited.push_back(current);
        if (current != root) {
            std::wstring item = DonorString(DonorCall(donor087::UIObjectGetName, current));
            // The donor only adds control text for descendant buttons/toggles. We only
            // need UIButton parity for the Revive role, so read text when this object
            // responds as an active UIButton; non-button child names are still useful.
            if (IsUiButton(ui, current)) {
                const std::wstring childText = DonorString(DonorCall(donor087::UIButtonGetText, current));
                if (!childText.empty()) item += (item.empty() ? L"" : L" ") + childText;
            }
            if (!item.empty() && output.find(item) == std::wstring::npos) {
                if (!output.empty()) output += L" ";
                output += item;
            }
        }
        const std::uintptr_t array = static_cast<std::uintptr_t>(DonorCall(donor087::UIObjectCoreChildren, current));
        if (!array) continue;
        std::uint64_t length = 0;
        if (!ReadLocal(array + donor087::ArrayLength, length) || length > 128) continue;
        for (std::uint64_t i = 0; i < length; ++i) {
            std::uintptr_t child = 0;
            if (ReadLocal(array + donor087::ArrayData + i * sizeof(std::uintptr_t), child) && child)
                pending.push_back(child);
        }
    }
}

bool DescribeDonorButton(std::uint64_t button, bool includeDescendants, DonorButtonInfo& info) {
    info = {};
    if (!InspectDonorButton(button, info.name, info.text)) return false;
    info.object = button;
    if (includeDescendants) CollectDonorDescendantLabels(button, info.descendants);
    if (!info.name.empty()) info.label = L"Name=“" + info.name + L"”";
    if (!info.text.empty()) {
        if (!info.label.empty()) info.label += L" • ";
        info.label += L"Text=“" + info.text + L"”";
    }
    if (!info.descendants.empty()) {
        if (!info.label.empty()) info.label += L" • ";
        info.label += L"Child=“" + info.descendants + L"”";
    }
    if (info.label.empty()) info.label = L"UIButton không có nhãn";
    info.nameKey = CompactMatch(info.name);
    info.textKey = CompactMatch(info.text);
    info.allKey = CompactMatch(info.name + L" " + info.text + L" " + info.descendants);
    return true;
}

bool ReadAllDonorButtons(std::vector<std::uint64_t>& buttons, wchar_t* detail, std::size_t cap) {
    buttons.clear();
    DonorUiRuntime ui{};
    if (!ResolveDonorUiRuntime(ui, detail, cap)) return false;
    void* dictionaryRaw = nullptr;
    g_api.field_static_get_value(ui.instances, &dictionaryRaw);
    const std::uintptr_t dictionary = reinterpret_cast<std::uintptr_t>(dictionaryRaw);
    if (!dictionary) { SetText(detail, cap, L"UIObject.instances=null"); return false; }
    std::uintptr_t entries = 0;
    std::int32_t count = 0;
    if (!ReadLocal(dictionary + donor087::DictionaryEntries, entries) || !entries ||
        !ReadLocal(dictionary + donor087::DictionaryCount, count) || count < 0 || count > 32768) {
        SetText(detail, cap, L"UIObject.instances dictionary không hợp lệ"); return false;
    }
    for (std::int32_t i = 0; i < count; ++i) {
        std::uintptr_t object = 0;
        const std::uintptr_t entry = entries + donor087::ArrayData +
            static_cast<std::uintptr_t>(i) * donor087::EntrySize;
        if (ReadLocal(entry + donor087::EntryValue, object) && IsUiButton(ui, object)) buttons.push_back(object);
    }
    return true;
}

bool FindMessageBox(std::uint64_t& root, wchar_t* detail, std::size_t cap) {
    root = 0;
    if (!ValidateDonorUi(detail, cap)) return false;
    if (!g_api.string_new || !g_api.gchandle_new || !g_api.gchandle_get_target) {
        SetText(detail, cap, L"Thiếu IL2CPP string/GCHandle cho MessageBox donor v0.8.7");
        return false;
    }

    // v0.8.7 keeps one rooted managed "MessageBox" string instead of allocating a
    // new IL2CPP string on every ReadState poll. Preserve that behavior: it is both
    // faster and avoids long-running GC pressure in multi-client sessions.
    static std::uint32_t messageBoxNameHandle = 0;
    if (messageBoxNameHandle == 0) {
        Il2CppString* created = g_api.string_new("MessageBox");
        if (!created) { SetText(detail, cap, L"Không tạo được managed string MessageBox"); return false; }
        messageBoxNameHandle = g_api.gchandle_new(reinterpret_cast<Il2CppObject*>(created), false);
        if (messageBoxNameHandle == 0) {
            SetText(detail, cap, L"Không tạo được GCHandle MessageBox donor"); return false;
        }
    }
    Il2CppObject* target = g_api.gchandle_get_target(messageBoxNameHandle);
    if (!target) { SetText(detail, cap, L"GCHandle MessageBox mất target"); return false; }
    const auto name = reinterpret_cast<std::uint64_t>(target);
    // Exact v0.8.7 portal detector: MainFindUI("MessageBox") only. Do not widen
    // this to LuaFindUI, because a non-main UI with the same name is not donor parity.
    root = DonorCall(donor087::LuaMainFindUI, name);
    return true;
}

bool CollectTreeButtons(std::uint64_t root, std::vector<std::uint64_t>& buttons,
                        wchar_t* detail, std::size_t cap) {
    buttons.clear();
    DonorUiRuntime ui{};
    if (!ResolveDonorUiRuntime(ui, detail, cap)) return false;
    if (!root) return true;
    std::vector<std::uint64_t> pending{root};
    std::vector<std::uint64_t> visited;
    while (!pending.empty() && visited.size() < 2048) {
        const std::uint64_t current = pending.back(); pending.pop_back();
        if (!current || std::find(visited.begin(), visited.end(), current) != visited.end()) continue;
        visited.push_back(current);
        if (IsUiButton(ui, current)) buttons.push_back(current);
        const std::uintptr_t array = static_cast<std::uintptr_t>(DonorCall(donor087::UIObjectCoreChildren, current));
        if (!array) continue;
        std::uint64_t length = 0;
        if (!ReadLocal(array + donor087::ArrayLength, length) || length > 512) {
            SetText(detail, cap, L"Cây MessageBox children không hợp lệ"); return false;
        }
        for (std::uint64_t i = 0; i < length; ++i) {
            std::uintptr_t child = 0;
            if (!ReadLocal(array + donor087::ArrayData + i * sizeof(std::uintptr_t), child)) return false;
            if (child) pending.push_back(child);
        }
    }
    if (visited.size() >= 2048) { SetText(detail, cap, L"Cây MessageBox vượt giới hạn an toàn"); return false; }
    return true;
}

bool ClickDonorButton(std::uint64_t button) {
    if (!button) return false;
    (void)DonorCall(donor087::UIButtonHandleClick, button);
    return true;
}

bool ReadConfirmVisible(std::int32_t& visible, wchar_t* detail, std::size_t cap) {
    visible = 0;
    std::uint64_t root = 0;
    if (!FindMessageBox(root, detail, cap)) return false;
    visible = root ? 1 : 0;
    return true;
}

struct Classes {
    Il2CppClass* gameApi = nullptr;
    Il2CppClass* guiApi = nullptr; // optional observer surface; route core must remain usable if unavailable
    Il2CppClass* session = nullptr;
    Il2CppClass* shared = nullptr;
    Il2CppClass* autoPath = nullptr;
};

bool ResolveClasses(Classes& c, wchar_t* detail, std::size_t cap) {
    if (!g_api.Load(detail, cap)) return false;
    const Il2CppImage* image = Image();
    if (!image) { SetText(detail, cap, L"Không mở được Assembly-CSharp"); return false; }
    c.gameApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_Game");
    c.guiApi = g_api.class_from_name(image, "FGStudio.LuaSystem.API", "LuaSystemAPI_GUI");
    c.session = g_api.class_from_name(image, "FGStudio.Game.Logic", "SessionData");
    c.shared = g_api.class_from_name(image, "FGStudio.LuaSystem", "LuaSystemSharedData");
    c.autoPath = g_api.class_from_name(image, "FGStudio.Engine.Logic", "AutoPathManager");
    if (!c.gameApi || !c.session || !c.shared || !c.autoPath) {
        SetText(detail, cap, L"Thiếu class route bắt buộc trên client này");
        return false;
    }
    return true;
}

bool Transition(const Classes& c, int& mapReady, int& waiting, wchar_t* detail, std::size_t cap) {
    if (!StaticScalar(c.gameApi, "IsMapReady", mapReady, detail, cap)) return false;
    if (!StaticScalar(c.session, "get_WaitingChangeMap", waiting, detail, cap)) return false;
    return true;
}

bool GetLeader(const Classes& c, Il2CppObject*& leader, Il2CppClass*& leaderClass,
               wchar_t* detail, std::size_t cap) {
    const MethodInfo* getLeader = ExactMethod(c.shared, "get_LeaderRoleData", 0, true);
    if (!getLeader || !InvokeObject(getLeader, nullptr, leader, detail, cap) || !leader) {
        SetText(detail, cap, L"LeaderRoleData chưa sẵn sàng"); return false;
    }
    leaderClass = g_api.object_get_class(leader);
    if (!leaderClass) { SetText(detail, cap, L"Không lấy được class LeaderRoleData"); return false; }
    return true;
}

bool ReadPosition(Il2CppObject* leader, Il2CppClass* leaderClass, int& x, int& y,
                  wchar_t* detail, std::size_t cap) {
    if (ScalarGetter(leaderClass, "get_PosX", leader, x, detail, cap) &&
        ScalarGetter(leaderClass, "get_PosY", leader, y, detail, cap)) return true;
    FieldInfo* field = nullptr;
    for (Il2CppClass* c = leaderClass; c; c = g_api.class_get_parent(c)) {
        field = g_api.class_get_field_from_name(c, "roleData");
        if (field) break;
    }
    if (!field) { SetText(detail, cap, L"Không resolve được PosX/PosY"); return false; }
    const Il2CppType* ft = g_api.field_get_type(field);
    Il2CppClass* fc = ft ? g_api.class_from_type(ft) : nullptr;
    if (!fc || g_api.class_is_valuetype(fc)) { SetText(detail, cap, L"roleData backing không hợp lệ"); return false; }
    Il2CppObject* backing = nullptr;
    g_api.field_get_value(leader, field, &backing);
    if (!backing) { SetText(detail, cap, L"roleData backing=null"); return false; }
    Il2CppClass* bc = g_api.object_get_class(backing);
    return bc && ScalarGetter(bc, "get_PosX", backing, x, detail, cap) &&
                 ScalarGetter(bc, "get_PosY", backing, y, detail, cap);
}

bool AutoPathInstance(const Classes& c, Il2CppObject*& instance, Il2CppClass*& actual,
                      wchar_t* detail, std::size_t cap) {
    const MethodInfo* getInstance = ExactMethod(c.autoPath, "get_Instance", 0, true);
    if (!getInstance || !InvokeObject(getInstance, nullptr, instance, detail, cap) || !instance) {
        SetText(detail, cap, L"AutoPathManager.Instance chưa sẵn sàng"); return false;
    }
    actual = g_api.object_get_class(instance);
    if (!actual) { SetText(detail, cap, L"Không lấy được class AutoPathManager"); return false; }
    return true;
}

bool ReadState(Snapshot& s, wchar_t* detail, std::size_t cap) {
    Classes c{};
    if (!ResolveClasses(c, detail, cap)) return false;
    static std::uint32_t seq = 0;
    s = {}; s.sequence = ++seq;
    int ready = 0, waiting = 0;
    if (!Transition(c, ready, waiting, detail, cap)) return false;
    s.mapReady = ready ? 1 : 0;
    s.waitingChangeMap = waiting ? 1 : 0;
    s.validMask |= ValidMapTransition;
    if (!ready || waiting) {
        SetText(detail, cap, L"Đang chuyển map; không đọc object sâu và không gửi action");
        return true;
    }

    Il2CppObject* leader = nullptr; Il2CppClass* lc = nullptr;
    if (!GetLeader(c, leader, lc, detail, cap)) return false;
    int role = 0, map = 0, x = 0, y = 0, riding = 0;
    if (!ScalarGetter(lc, "get_RoleID", leader, role, detail, cap) || role <= 0) return false;
    if (!ScalarGetter(lc, "get_MapID", leader, map, detail, cap) || map <= 0) return false;
    if (!ReadPosition(leader, lc, x, y, detail, cap)) return false;
    if (!ScalarGetter(lc, "get_IsRiding", leader, riding, detail, cap)) return false;
    s.roleID = role; s.mapID = map; s.x = x; s.y = y; s.riding = riding ? 1 : 0;
    s.validMask |= ValidIdentity | ValidMap | ValidPosition | ValidRiding;

    int hp = 0, maxHP = 0, dead = 0;
    wchar_t optionalDetail[160]{};
    if (ScalarGetter(lc, "get_HP", leader, hp, optionalDetail, _countof(optionalDetail)) &&
        ScalarGetter(lc, "get_MaxHP", leader, maxHP, optionalDetail, _countof(optionalDetail)) &&
        hp >= 0 && maxHP > 0) {
        s.hp = hp; s.maxHP = maxHP; s.validMask |= ValidVitals;
    }
    optionalDetail[0] = 0;
    if (ScalarGetter(lc, "get_IsDeath", leader, dead, optionalDetail, _countof(optionalDetail))) {
        s.dead = dead ? 1 : 0; s.validMask |= ValidLifeState;
    }

    int moving = 0;
    optionalDetail[0] = 0;
    if (StaticScalar(c.gameApi, "IsMoving", moving, optionalDetail, _countof(optionalDetail))) {
        s.moving = moving ? 1 : 0; s.validMask |= ValidMoving;
    }

    int enableAutoF1 = 0;
    optionalDetail[0] = 0;
    if (StaticScalar(c.gameApi, "get_EnableAutoF1", enableAutoF1, optionalDetail, _countof(optionalDetail))) {
        // Existing client semantic verified by the read-only NewCore donor: EnableAutoF1=false means auto-fight is ON.
        s.autoFight = enableAutoF1 ? 0 : 1; s.validMask |= ValidAutoFight;
    }

    int freeBagSpace = -1;
    optionalDetail[0] = 0;
    if (StaticScalar(c.gameApi, "GetFreeBagSpace", freeBagSpace, optionalDetail, _countof(optionalDetail)) && freeBagSpace >= 0) {
        s.freeBagSpace = freeBagSpace; s.validMask |= ValidBagSpace;
    }

    // v1.5.10: restore the v0.8.7 semantic MessageBox observer. This is read-only;
    // if the donor UI surface cannot be proven on this exact GameAssembly build,
    // leave ValidConfirmUi unset so the controller fails closed and never clicks blind.
    std::int32_t confirmVisible = 0;
    optionalDetail[0] = 0;
    if (ReadConfirmVisible(confirmVisible, optionalDetail, _countof(optionalDetail))) {
        s.confirmUiVisible = confirmVisible ? 1 : 0;
        s.validMask |= ValidConfirmUi;
    }

    Il2CppObject* ap = nullptr; Il2CppClass* ac = nullptr;
    if (!AutoPathInstance(c, ap, ac, detail, cap)) return false;
    int pathing = 0;
    if (!ScalarGetter(ac, "get_IsAutoPathing", ap, pathing, detail, cap)) return false;
    s.autoPathing = pathing ? 1 : 0; s.validMask |= ValidAutoPath;

    const MethodInfo* getName = FindMethod(lc, "get_Name", 0);
    if (getName) {
        Il2CppObject* no = nullptr;
        wchar_t ignored[128]{};
        if (InvokeObject(getName, leader, no, ignored, _countof(ignored)) && no)
            (void)CopyString(reinterpret_cast<Il2CppString*>(no), s.characterName, _countof(s.characterName));
    }

    SetText(detail, cap, L"STATE map="); AppendInt(detail, cap, s.mapID);
    Append(detail, cap, L" pos="); AppendInt(detail, cap, s.x); Append(detail, cap, L","); AppendInt(detail, cap, s.y);
    Append(detail, cap, L" riding="); AppendInt(detail, cap, s.riding);
    Append(detail, cap, L" autoPath="); AppendInt(detail, cap, s.autoPathing);
    if (s.validMask & ValidLifeState) { Append(detail, cap, L" dead="); AppendInt(detail, cap, s.dead); }
    if (s.validMask & ValidVitals) { Append(detail, cap, L" HP="); AppendInt(detail, cap, s.hp); Append(detail, cap, L"/"); AppendInt(detail, cap, s.maxHP); }
    if (s.validMask & ValidAutoFight) { Append(detail, cap, L" autoFight="); AppendInt(detail, cap, s.autoFight); }
    if (s.validMask & ValidBagSpace) { Append(detail, cap, L" freeBag="); AppendInt(detail, cap, s.freeBagSpace); }
    return true;
}

bool SafeForAction(const Classes& c, wchar_t* detail, std::size_t cap) {
    int ready = 0, waiting = 0;
    if (!Transition(c, ready, waiting, detail, cap)) return false;
    if (!ready || waiting) { SetText(detail, cap, L"Action bị chặn: đang chuyển map"); return false; }
    return true;
}

bool ClickInternalConfirm(wchar_t* detail, std::size_t cap) {
    Classes c{};
    if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    std::uint64_t root = 0;
    if (!FindMessageBox(root, detail, cap) || !root) {
        SetText(detail, cap, L"Không bấm Confirm: MessageBox không tồn tại"); return false;
    }
    std::vector<std::uint64_t> buttons;
    if (!CollectTreeButtons(root, buttons, detail, cap)) return false;
    struct Candidate { std::uint64_t object; int score; std::wstring label; };
    std::vector<Candidate> candidates;

    // Keep v0.8.7's Confirm matcher literally accent-aware here. Do not replace
    // this with CompactMatch(): "Đồng ý" -> dongy while "Đóng" -> dong, which
    // would make the negative token a substring of the positive Vietnamese label.
    const std::vector<std::wstring> positive{
        L"đồng ý", L"dong y", L"xác nhận", L"xac nhan", L"confirm",
        L"buttonok", L"btnok", L"buttonyes", L"btnyes", L"yes"};
    const std::vector<std::wstring> negative{
        L"hủy", L"huy", L"không", L"khong", L"cancel", L"buttonno",
        L"btnno", L"đóng", L"close", L"thoát", L"thoat"};
    for (std::uint64_t button : buttons) {
        std::wstring name, text;
        if (!InspectDonorButton(button, name, text)) continue;
        const std::wstring label = DonorTrim(name + L" " + text);
        const std::wstring normalized = DonorLower(label);
        int score = DonorContainsAny(normalized, positive) ? 20 : 0;
        if (DonorContainsAny(normalized, negative)) score -= 100;
        const std::wstring exactName = DonorTrim(DonorLower(name));
        const std::wstring exactText = DonorTrim(DonorLower(text));
        if (exactName == L"ok" || exactText == L"ok" || exactText == L"có" ||
            exactText == L"yes") score += 20;
        candidates.push_back({button, score, label});
    }
    if (candidates.empty()) {
        SetText(detail, cap, L"MessageBox không có UIButton đang hoạt động"); return false;
    }
    std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b){ return a.score > b.score; });
    const bool unambiguous = candidates.size() == 1
        ? candidates.front().score >= 0
        : candidates.front().score > 0 && candidates.front().score > candidates[1].score;
    if (!unambiguous) {
        SetText(detail, cap, L"Không xác định duy nhất nút đồng ý trong MessageBox; không bấm mù"); return false;
    }
    if (!ClickDonorButton(candidates.front().object)) return false;
    SetText(detail, cap, L"v0.8.7 UIButton.HandleClickEvent Confirm PASS: ");
    Append(detail, cap, candidates.front().label.c_str());
    return true;
}

bool ClickInternalRevive(wchar_t* detail, std::size_t cap) {
    Classes c{};
    if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    Il2CppObject* leader = nullptr; Il2CppClass* lc = nullptr;
    if (!GetLeader(c, leader, lc, detail, cap)) return false;
    std::int32_t dead = 0;
    if (!ScalarGetter(lc, "get_IsDeath", leader, dead, detail, cap) || !dead) {
        SetText(detail, cap, L"Không bấm Đầu thai vì get_IsDeath=false/không authoritative"); return false;
    }

    std::vector<std::uint64_t> buttons;
    if (!ReadAllDonorButtons(buttons, detail, cap)) return false;
    struct Candidate { DonorButtonInfo info; int score; };
    auto choose = [&](bool descendants, Candidate& selected) -> bool {
        std::vector<Candidate> candidates;
        for (std::uint64_t button : buttons) {
            DonorButtonInfo info;
            if (!DescribeDonorButton(button, descendants, info)) continue;
            int score = 0;
            if (info.textKey == L"dauthai") score = 600;
            else if (info.allKey.find(L"dauthai") != std::wstring::npos) score = 520;
            if (info.allKey.find(L"cancel") != std::wstring::npos ||
                info.allKey.find(L"close") != std::wstring::npos ||
                info.allKey.find(L"huy") != std::wstring::npos ||
                info.allKey.find(L"thoat") != std::wstring::npos) score -= 1000;
            if (score > 0) candidates.push_back({std::move(info), score});
        }
        std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b){
            return a.score > b.score;
        });
        if (candidates.empty()) return false;
        if (candidates.size() > 1 && candidates[0].score == candidates[1].score) return false;
        selected = std::move(candidates.front());
        return true;
    };

    Candidate selected{};
    if (!choose(false, selected) && !choose(true, selected)) {
        SetText(detail, cap, L"IsDeath=true nhưng chưa định vị duy nhất được UIButton Đầu thai");
        return false;
    }
    if (!ClickDonorButton(selected.info.object)) {
        SetText(detail, cap, L"UIButton.HandleClickEvent Đầu thai không phản hồi");
        return false;
    }
    SetText(detail, cap, L"v0.8.7 UIButton.HandleClickEvent Đầu thai PASS: ");
    Append(detail, cap, selected.info.label.c_str());
    return true;
}

bool ToggleRide(bool desiredRiding, wchar_t* detail, std::size_t cap) {
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    Il2CppObject* leader = nullptr; Il2CppClass* lc = nullptr;
    if (!GetLeader(c, leader, lc, detail, cap)) return false;
    int riding = 0;
    if (!ScalarGetter(lc, "get_IsRiding", leader, riding, detail, cap)) return false;
    if ((riding != 0) == desiredRiding) { SetText(detail, cap, L"Ride state đã đúng; không toggle lại"); return true; }

    const MethodInfo* getSlot = ExactMethod(c.gameApi, "get_CurrentMountSlot", 0, true);
    const MethodInfo* toggle = ExactMethod(c.gameApi, "SendToggleRideState", 1, true, "System.Int32");
    if (!getSlot || !toggle) { SetText(detail, cap, L"Không resolve được API lên/xuống ngựa"); return false; }
    std::int64_t slot64 = 0;
    if (!InvokeScalar(getSlot, nullptr, slot64, detail, cap) || slot64 < 0 || slot64 > INT32_MAX) return false;
    std::int32_t slot = static_cast<std::int32_t>(slot64);
    void* args[] = { &slot };
    if (!InvokeVoid(toggle, nullptr, args, detail, cap)) return false;
    SetText(detail, cap, desiredRiding ? L"Đã gửi lệnh lên ngựa" : L"Đã gửi lệnh xuống ngựa");
    return true;
}

bool StartPath(int mapID, int x, int y, wchar_t* detail, std::size_t cap) {
    if (mapID <= 0) { SetText(detail, cap, L"MapID đích không hợp lệ"); return false; }
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    Il2CppObject* ap = nullptr; Il2CppClass* ac = nullptr;
    if (!AutoPathInstance(c, ap, ac, detail, cap)) return false;
    const MethodInfo* start = ExactMethod(ac, "StartAutoPath", 3, false,
                                          "System.Int32", "System.Int32", "System.Int32");
    if (!start) { SetText(detail, cap, L"Không resolve đúng StartAutoPath(Int32,Int32,Int32)"); return false; }
    std::int32_t m = mapID, px = x, py = y;
    void* args[] = { &m, &px, &py };
    if (!InvokeVoid(start, ap, args, detail, cap)) return false;
    SetText(detail, cap, L"Đã gửi AutoPath tới map="); AppendInt(detail, cap, mapID);
    Append(detail, cap, L" x="); AppendInt(detail, cap, x); Append(detail, cap, L" y="); AppendInt(detail, cap, y);
    return true;
}

bool StopPath(wchar_t* detail, std::size_t cap) {
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    const MethodInfo* stop = ExactMethod(c.gameApi, "StopAutoPath", 0, true);
    if (!stop) { SetText(detail, cap, L"Không resolve được LuaSystemAPI_Game.StopAutoPath()"); return false; }
    if (!InvokeVoid(stop, nullptr, nullptr, detail, cap)) return false;
    SetText(detail, cap, L"Đã gửi StopAutoPath");
    return true;
}


bool ClickNpc(int npcID, wchar_t* detail, std::size_t cap) {
    if (npcID <= 0) { SetText(detail, cap, L"NPC ID không hợp lệ"); return false; }
    Classes c{}; if (!ResolveClasses(c, detail, cap) || !SafeForAction(c, detail, cap)) return false;
    const MethodInfo* click = ExactMethod(c.gameApi, "ClickNPC", 1, true);
    if (!click) { SetText(detail, cap, L"Không resolve đúng static LuaSystemAPI_Game.ClickNPC(1 arg)"); return false; }
    std::int32_t id = npcID;
    void* args[] = { &id };
    if (!InvokeVoid(click, nullptr, args, detail, cap)) return false;
    SetText(detail, cap, L"Đã gửi ClickNPC id="); AppendInt(detail, cap, npcID);
    return true;
}

bool EnsureShared() {
    if (g_shared) return true;
    wchar_t name[96]{}; MappingName(GetCurrentProcessId(), name, _countof(name));
    g_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!g_mapping) return false;
    g_shared = reinterpret_cast<SharedBlock*>(MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    if (!g_shared || g_shared->magic != kMagic || g_shared->protocolVersion != kProtocolVersion ||
        g_shared->targetPid != GetCurrentProcessId()) {
        if (g_shared) UnmapViewOfFile(g_shared);
        if (g_mapping) CloseHandle(g_mapping);
        g_shared = nullptr; g_mapping = nullptr; return false;
    }
    InterlockedExchange(&g_shared->bridgeLoaded, 1);
    return true;
}

void ProcessRequest() {
    if (!EnsureShared()) return;
    const LONG seq = g_shared->requestSeq;
    if (seq <= 0 || seq == g_shared->completedSeq) return;
    if (InterlockedCompareExchange(&g_shared->bridgeBusy, 1, 0) != 0) return;

    Response r{};
    r.callbackThreadId = GetCurrentThreadId();
    wchar_t detail[512]{};
    bool ok = false;
    if (r.callbackThreadId != g_shared->targetWindowThreadId) {
        SetText(detail, _countof(detail), L"Sai callback thread; action bị chặn");
        r.errorCode = 1001;
    } else {
        const Command cmd = static_cast<Command>(g_shared->request.command);
        switch (cmd) {
            case Command::ReadState:
                ok = ReadState(r.snapshot, detail, _countof(detail)); r.errorCode = ok ? 0 : 1101; break;
            case Command::ToggleRide:
                ok = ToggleRide(g_shared->request.arg0 != 0, detail, _countof(detail)); r.errorCode = ok ? 0 : 1201; break;
            case Command::StartPath:
                ok = StartPath(g_shared->request.arg0, g_shared->request.arg1, g_shared->request.arg2,
                               detail, _countof(detail)); r.errorCode = ok ? 0 : 1301; break;
            case Command::StopPath:
                ok = StopPath(detail, _countof(detail)); r.errorCode = ok ? 0 : 1401; break;
            case Command::ClickNpc:
                ok = ClickNpc(g_shared->request.arg0, detail, _countof(detail));
                r.errorCode = ok ? 0 : 1601; break;
            case Command::ClickInternalConfirm:
                ok = ClickInternalConfirm(detail, _countof(detail));
                r.errorCode = ok ? 0 : 1701; break;
            case Command::ClickInternalRevive:
                ok = ClickInternalRevive(detail, _countof(detail));
                r.errorCode = ok ? 0 : 1801; break;
            default:
                SetText(detail, _countof(detail), L"Command không hợp lệ"); r.errorCode = 1002; break;
        }
    }
    r.ok = ok ? 1 : 0;
    SetText(r.detail, _countof(r.detail), detail);
    g_shared->response = r;
    MemoryBarrier();
    InterlockedExchange(&g_shared->completedSeq, seq);
    InterlockedExchange(&g_shared->bridgeBusy, 0);
}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK TlcGetMessageHook(int code, WPARAM wParam, LPARAM lParam) {
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
