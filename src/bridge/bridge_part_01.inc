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
