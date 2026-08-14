#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <fstream>
#include <initializer_list>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

extern "C" unsigned char RemoteWorkerStart[];
extern "C" unsigned char RemoteWorkerEnd[];

using Clock = std::chrono::steady_clock;
constexpr wchar_t kTitle[] = L"Thần Long Mobile - Auto Train v0.9.0";
constexpr wchar_t kModule[] = L"GameAssembly.dll";

namespace rva {
constexpr std::uint64_t LuaGetRoleData = 0x67B5D0;
constexpr std::uint64_t LuaPlayerGetRoleID = 0x50D0F0;
constexpr std::uint64_t LuaPlayerGetName = 0x50D000;
constexpr std::uint64_t LuaLeaderIsDeath = 0x50C910;
constexpr std::uint64_t LuaIsRiding = 0x678050;
constexpr std::uint64_t LuaCurrentMountSlot = 0x67AE60;
constexpr std::uint64_t LuaToggleRide = 0x679760;
constexpr std::uint64_t LuaGetAutoFightEnabled = 0x67B440;
constexpr std::uint64_t LuaGetFreeBagSpace = 0x6716F0;
constexpr std::uint64_t LuaGetItemsAtSite = 0x672650;
constexpr std::uint64_t LuaGetItemData = 0x672160;
constexpr std::uint64_t LuaGetItemType = 0x672520;
constexpr std::uint64_t LuaIsItemSellable = 0x677B30;
constexpr std::uint64_t LuaItemGetID = 0x41F000;
constexpr std::uint64_t LuaItemGetItemID = 0x4D0A90;
constexpr std::uint64_t LuaItemGetPosition = 0x43F560;
constexpr std::uint64_t LuaGetBuffs = 0x66F020;
constexpr std::uint64_t LuaHasBuff = 0x677610;
constexpr std::uint64_t LuaRequestUsingSkill = 0x6791C0;
constexpr std::uint64_t LuaBuffGetBuffID = 0x41F000;
constexpr std::uint64_t LuaBuffGetDurationTick = 0x41F3F0;
constexpr std::uint64_t LuaGetNearestNPC = 0x673A90;
constexpr std::uint64_t LuaGetNearestNPCByResID = 0x673A80;
constexpr std::uint64_t LuaClickNPC = 0x66ADC0;
constexpr std::uint64_t SessionGetNPCs = 0x6F3DD0;
constexpr std::uint64_t SessionGetMovingNPCs = 0x6F3D80;
constexpr std::uint64_t GNPCGetResID = 0x68EE70;
constexpr std::uint64_t LuaMapObjectGetRoleID = 0x41F000;
constexpr std::uint64_t LuaMapObjectGetName = 0x41F3F0;
constexpr std::uint64_t LuaIsMoving = 0x677F60;
constexpr std::uint64_t LuaGetSkills = 0x675160;
constexpr std::uint64_t LuaGetSkillName = 0x674D40;
constexpr std::uint64_t LuaHasSkill = 0x677840;
constexpr std::uint64_t LuaCanUseSkill = 0x66A8E0;
constexpr std::uint64_t LuaCheckSkillCondition = 0x66AD30;
constexpr std::uint64_t LuaIsSkillCooldown = 0x6782D0;
constexpr std::uint64_t LuaRequestUsingSkillWithTarget = 0x6791B0;
constexpr std::uint64_t LuaGetSelectedTarget = 0x67B610;
constexpr std::uint64_t SelectedTargetGetRoleID = 0x526830;
constexpr std::uint64_t LuaIsSelectTargetDie = 0x678260;
constexpr std::uint64_t LuaIsMapReady = 0x677E60;
constexpr std::uint64_t LuaFindUI = 0x6A5DF0;
constexpr std::uint64_t LuaMainFindUI = 0x6A5F90;
constexpr std::uint64_t UIObjectGetName = 0x530240;
constexpr std::uint64_t UIObjectActiveInHierarchy = 0x52F7D0;
constexpr std::uint64_t UIObjectCoreChildren = 0x52FB80;
constexpr std::uint64_t UIButtonGetInteractable = 0x52E120;
constexpr std::uint64_t UIButtonGetText = 0x52E230;
constexpr std::uint64_t UIButtonHandleClick = 0x52D140;
constexpr std::uint64_t UIToggleGetInteractable = 0x688580;
constexpr std::uint64_t UIToggleGetSelected = 0x6885D0;
constexpr std::uint64_t UIToggleGetText = 0x688710;
constexpr std::uint64_t UIToggleSetSelected = 0x6888E0;
constexpr std::uint64_t UIToggleHandleSelectEvent = 0x687450;
constexpr std::uint64_t MonoBehaviourExecutorGetInstance = 0x523CE0;
constexpr std::uint64_t MonoBehaviourExecutorExecuteUiObject = 0x521B20;
constexpr std::uint64_t SessionWaitingChangeMap = 0x6F4390;
constexpr std::uint64_t AutoPathGetInstance = 0x615980;
constexpr std::uint64_t AutoPathStart = 0x615510;
constexpr std::uint64_t AutoPathStop = 0x615910;
constexpr std::uint64_t AutoPathIsRunning = 0x6159C0;
}

namespace off {
constexpr std::uint64_t LuaPlayerRoleData = 0x10;
constexpr std::uint64_t RoleMapID = 0x50;
constexpr std::uint64_t RolePosX = 0x54;
constexpr std::uint64_t RolePosY = 0x58;
constexpr std::uint64_t DictionaryEntries = 0x18;
constexpr std::uint64_t DictionaryCount = 0x20;
constexpr std::uint64_t ArrayLength = 0x18;
constexpr std::uint64_t ArrayData = 0x20;
constexpr std::uint64_t EntrySize = 0x18;
constexpr std::uint64_t EntryValue = 0x10;
constexpr std::uint64_t GSpriteRoleID = 0x30;
constexpr std::uint64_t GNPCResID = 0xC0;
constexpr std::uint64_t LuaDbSkillID = 0x10;
constexpr std::uint64_t ListItems = 0x10;
constexpr std::uint64_t ListSize = 0x18;
}

static std::wstring ExeDirectory() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (wchar_t* slash = wcsrchr(path, L'\\')) *slash = L'\0';
    return path;
}

static std::string Utf8(const std::wstring& text) {
    if (text.empty()) return {};
    const int count = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                          nullptr, 0, nullptr, nullptr);
    std::string result(static_cast<std::size_t>(count), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), count,
                        nullptr, nullptr);
    return result;
}

static std::wstring Wide(const std::string& text) {
    if (text.empty()) return {};
    const int count = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                          nullptr, 0);
    std::wstring result(static_cast<std::size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), count);
    return result;
}

static std::wstring Trim(std::wstring value) {
    while (!value.empty() && iswspace(value.front())) value.erase(value.begin());
    while (!value.empty() && iswspace(value.back())) value.pop_back();
    return value;
}

static std::wstring Lower(std::wstring value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return value;
}

// UI text is Vietnamese while internal object names are usually English.  Fold
// both forms to one compact ASCII key so labels such as "Đánh quái" and
// "DanhQuai" are matched identically without depending on the system locale.
static wchar_t FoldVietnameseChar(wchar_t c) {
    switch (c) {
        case L'À': case L'Á': case L'Ạ': case L'Ả': case L'Ã':
        case L'Â': case L'Ầ': case L'Ấ': case L'Ậ': case L'Ẩ': case L'Ẫ':
        case L'Ă': case L'Ằ': case L'Ắ': case L'Ặ': case L'Ẳ': case L'Ẵ':
        case L'à': case L'á': case L'ạ': case L'ả': case L'ã':
        case L'â': case L'ầ': case L'ấ': case L'ậ': case L'ẩ': case L'ẫ':
        case L'ă': case L'ằ': case L'ắ': case L'ặ': case L'ẳ': case L'ẵ': return L'a';
        case L'È': case L'É': case L'Ẹ': case L'Ẻ': case L'Ẽ':
        case L'Ê': case L'Ề': case L'Ế': case L'Ệ': case L'Ể': case L'Ễ':
        case L'è': case L'é': case L'ẹ': case L'ẻ': case L'ẽ':
        case L'ê': case L'ề': case L'ế': case L'ệ': case L'ể': case L'ễ': return L'e';
        case L'Ì': case L'Í': case L'Ị': case L'Ỉ': case L'Ĩ':
        case L'ì': case L'í': case L'ị': case L'ỉ': case L'ĩ': return L'i';
        case L'Ò': case L'Ó': case L'Ọ': case L'Ỏ': case L'Õ':
        case L'Ô': case L'Ồ': case L'Ố': case L'Ộ': case L'Ổ': case L'Ỗ':
        case L'Ơ': case L'Ờ': case L'Ớ': case L'Ợ': case L'Ở': case L'Ỡ':
        case L'ò': case L'ó': case L'ọ': case L'ỏ': case L'õ':
        case L'ô': case L'ồ': case L'ố': case L'ộ': case L'ổ': case L'ỗ':
        case L'ơ': case L'ờ': case L'ớ': case L'ợ': case L'ở': case L'ỡ': return L'o';
        case L'Ù': case L'Ú': case L'Ụ': case L'Ủ': case L'Ũ':
        case L'Ư': case L'Ừ': case L'Ứ': case L'Ự': case L'Ử': case L'Ữ':
        case L'ù': case L'ú': case L'ụ': case L'ủ': case L'ũ':
        case L'ư': case L'ừ': case L'ứ': case L'ự': case L'ử': case L'ữ': return L'u';
        case L'Ỳ': case L'Ý': case L'Ỵ': case L'Ỷ': case L'Ỹ':
        case L'ỳ': case L'ý': case L'ỵ': case L'ỷ': case L'ỹ': return L'y';
        case L'Đ': case L'đ': case L'Ð': case L'ð': return L'd';
        default:
            if (c >= 0x0300 && c <= 0x036F) return 0;
            return static_cast<wchar_t>(towlower(c));
    }
}

static std::wstring CompactMatch(const std::wstring& value) {
    std::wstring result;
    result.reserve(value.size());
    for (const wchar_t original : value) {
        const wchar_t c = FoldVietnameseChar(original);
        if ((c >= L'a' && c <= L'z') || (c >= L'0' && c <= L'9'))
            result.push_back(c);
    }
    return result;
}

static bool ContainsCompact(const std::wstring& compact,
                            std::initializer_list<const wchar_t*> needles) {
    for (const wchar_t* needle : needles)
        if (compact.find(needle) != std::wstring::npos) return true;
    return false;
}

static bool ContainsAny(const std::wstring& value,
                        const std::vector<std::wstring>& needles) {
    for (const auto& needle : needles)
        if (value.find(needle) != std::wstring::npos) return true;
    return false;
}

static std::vector<std::wstring> SplitTabs(const std::wstring& line) {
    std::vector<std::wstring> result;
    std::wstring current;
    for (wchar_t c : line) {
        if (c == L'\t') { result.push_back(current); current.clear(); }
        else current.push_back(c);
    }
    result.push_back(current);
    return result;
}

struct ModuleInfo {
    std::uint64_t base = 0;
    std::uint32_t size = 0;
};

class ProcessMemory {
public:
    ~ProcessMemory() { Close(); }
    bool Open(DWORD pid, bool write = true) {
        Close();
        pid_ = pid;
        DWORD rights = PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | SYNCHRONIZE;
        if (write) rights |= PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD;
        handle_ = OpenProcess(rights, FALSE, pid);
        return handle_ != nullptr;
    }
    void Close() {
        if (handle_) CloseHandle(handle_);
        handle_ = nullptr;
        pid_ = 0;
    }
    HANDLE Handle() const { return handle_; }
    bool Alive() const { return handle_ && WaitForSingleObject(handle_, 0) == WAIT_TIMEOUT; }
    template <typename T> bool Read(std::uint64_t address, T& output) const {
        SIZE_T done = 0;
        return handle_ && ReadProcessMemory(handle_, reinterpret_cast<LPCVOID>(address), &output,
                                            sizeof(output), &done) && done == sizeof(output);
    }
    bool ReadBytes(std::uint64_t address, void* output, std::size_t size) const {
        SIZE_T done = 0;
        return handle_ && ReadProcessMemory(handle_, reinterpret_cast<LPCVOID>(address), output,
                                            size, &done) && done == size;
    }
    bool WriteBytes(std::uint64_t address, const void* input, std::size_t size) const {
        SIZE_T done = 0;
        return handle_ && WriteProcessMemory(handle_, reinterpret_cast<LPVOID>(address), input,
                                             size, &done) && done == size;
    }
    std::uint64_t Allocate(std::size_t size, DWORD protection = PAGE_READWRITE) const {
        if (!handle_ || !size) return 0;
        return reinterpret_cast<std::uint64_t>(VirtualAllocEx(
            handle_, nullptr, size, MEM_COMMIT | MEM_RESERVE, protection));
    }
    bool Free(std::uint64_t address) const {
        return address && handle_ &&
               VirtualFreeEx(handle_, reinterpret_cast<LPVOID>(address), 0, MEM_RELEASE);
    }
    ModuleInfo FindModule(const wchar_t* wanted) const {
        ModuleInfo result;
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid_);
        if (snap == INVALID_HANDLE_VALUE) return result;
        MODULEENTRY32W item{};
        item.dwSize = sizeof(item);
        if (Module32FirstW(snap, &item)) {
            do {
                if (_wcsicmp(item.szModule, wanted) == 0) {
                    result.base = reinterpret_cast<std::uint64_t>(item.modBaseAddr);
                    result.size = item.modBaseSize;
                    break;
                }
            } while (Module32NextW(snap, &item));
        }
        CloseHandle(snap);
        return result;
    }
    std::string ReadAscii(std::uint64_t address, std::size_t limit = 256) const {
        std::string result;
        for (std::size_t i = 0; i < limit; ++i) {
            char c = 0;
            if (!Read(address + i, c)) return {};
            if (!c) break;
            result.push_back(c);
        }
        return result;
    }
    std::wstring ReadIl2CppString(std::uint64_t address) const {
        if (!address) return {};
        std::int32_t length = 0;
        if (!Read(address + 0x10, length) || length <= 0 || length > 1024) return {};
        std::wstring result(static_cast<std::size_t>(length), L'\0');
        return ReadBytes(address + 0x14, result.data(), result.size() * sizeof(wchar_t))
                   ? result : std::wstring{};
    }
    std::uint64_t ResolveExport(const ModuleInfo& module, const char* wanted) const {
        IMAGE_DOS_HEADER dos{};
        IMAGE_NT_HEADERS64 nt{};
        if (!module.base || !Read(module.base, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE ||
            !Read(module.base + static_cast<std::uint32_t>(dos.e_lfanew), nt) ||
            nt.Signature != IMAGE_NT_SIGNATURE) return 0;
        const auto& dir = nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        IMAGE_EXPORT_DIRECTORY exports{};
        if (!dir.VirtualAddress || !Read(module.base + dir.VirtualAddress, exports) ||
            exports.NumberOfNames > 200000 || exports.NumberOfFunctions > 200000) return 0;
        std::vector<DWORD> names(exports.NumberOfNames), funcs(exports.NumberOfFunctions);
        std::vector<WORD> ords(exports.NumberOfNames);
        if (!ReadBytes(module.base + exports.AddressOfNames, names.data(), names.size() * 4) ||
            !ReadBytes(module.base + exports.AddressOfFunctions, funcs.data(), funcs.size() * 4) ||
            !ReadBytes(module.base + exports.AddressOfNameOrdinals, ords.data(), ords.size() * 2)) return 0;
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (ReadAscii(module.base + names[i]) != wanted || ords[i] >= funcs.size()) continue;
            return module.base + funcs[ords[i]];
        }
        return 0;
    }
private:
    HANDLE handle_ = nullptr;
    DWORD pid_ = 0;
};

#pragma pack(push, 1)
struct RemotePacket {
    volatile std::uint32_t command = 0;
    std::uint32_t reserved = 0;
    std::uint64_t function = 0;
    std::uint64_t arg1 = 0;
    std::uint64_t arg2 = 0;
    std::uint64_t arg3 = 0;
    std::uint64_t arg4 = 0;
    std::uint64_t arg5 = 0;
    std::uint64_t result = 0;
    std::uint64_t domainGet = 0;
    std::uint64_t threadAttach = 0;
    std::uint64_t threadDetach = 0;
    std::uint64_t sleep = 0;
};
#pragma pack(pop)

static_assert(offsetof(RemotePacket, arg5) == 48);
static_assert(offsetof(RemotePacket, result) == 56);
static_assert(offsetof(RemotePacket, sleep) == 88);
static_assert(offsetof(RemotePacket, reserved) == 4);

class RemoteExecutor {
public:
    explicit RemoteExecutor(ProcessMemory& process) : process_(process) {}
    ~RemoteExecutor() { Stop(); }
    bool Start(const ModuleInfo& game, std::wstring& error) {
        Stop();
        RemotePacket packet{};
        packet.domainGet = process_.ResolveExport(game, "il2cpp_domain_get");
        packet.threadAttach = process_.ResolveExport(game, "il2cpp_thread_attach");
        packet.threadDetach = process_.ResolveExport(game, "il2cpp_thread_detach");
        packet.sleep = process_.ResolveExport(process_.FindModule(L"kernel32.dll"), "Sleep");
        if (!packet.domainGet || !packet.threadAttach || !packet.threadDetach || !packet.sleep) {
            error = L"Không tìm được API IL2CPP";
            return false;
        }
        const std::size_t codeSize = static_cast<std::size_t>(RemoteWorkerEnd - RemoteWorkerStart);
        remoteCode_ = reinterpret_cast<std::uint64_t>(VirtualAllocEx(
            process_.Handle(), nullptr, codeSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
        remotePacket_ = reinterpret_cast<std::uint64_t>(VirtualAllocEx(
            process_.Handle(), nullptr, sizeof(packet), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        if (!remoteCode_ || !remotePacket_ ||
            !process_.WriteBytes(remoteCode_, RemoteWorkerStart, codeSize) ||
            !process_.WriteBytes(remotePacket_, &packet, sizeof(packet))) {
            error = L"Không tạo được kênh trong game";
            Stop();
            return false;
        }
        FlushInstructionCache(process_.Handle(), reinterpret_cast<LPCVOID>(remoteCode_), codeSize);
        thread_ = CreateRemoteThread(process_.Handle(), nullptr, 0,
                                     reinterpret_cast<LPTHREAD_START_ROUTINE>(remoteCode_),
                                     reinterpret_cast<LPVOID>(remotePacket_), 0, nullptr);
        if (!thread_) {
            error = L"Không tạo được worker; hãy chạy quyền Admin";
            Stop();
            return false;
        }
        Sleep(30);
        if (WaitForSingleObject(thread_, 0) != WAIT_TIMEOUT) {
            error = L"Worker IL2CPP đã dừng bất thường";
            Stop();
            return false;
        }
        return true;
    }
    bool Call5(std::uint64_t function, std::uint64_t a1, std::uint64_t a2,
               std::uint64_t a3, std::uint64_t a4, std::uint64_t a5,
               std::uint64_t& result, DWORD timeout = 1500) {
        std::lock_guard<std::mutex> guard(lock_);
        if (!thread_ || WaitForSingleObject(thread_, 0) != WAIT_TIMEOUT) return false;
        // A timed-out IL2CPP call may still be executing inside the game. Never
        // overwrite its packet: that race was one possible source of crashes.
        std::uint64_t current = 0;
        if (!remotePacket_ || !process_.Read(remotePacket_, current) || current != 0) return false;
        struct Fields { std::uint64_t fn, a1, a2, a3, a4, a5, result; } fields{
            function, a1, a2, a3, a4, a5, 0};
        if (!process_.WriteBytes(remotePacket_ + 8, &fields, sizeof(fields))) return false;
        const std::uint32_t run = 1;
        if (!process_.WriteBytes(remotePacket_, &run, 4)) return false;
        const ULONGLONG begin = GetTickCount64();
        while (GetTickCount64() - begin < timeout) {
            std::uint32_t command = 0;
            if (!process_.Read(remotePacket_, command)) return false;
            if (!command) return process_.Read(remotePacket_ + 56, result);
            if (WaitForSingleObject(thread_, 0) != WAIT_TIMEOUT) return false;
            Sleep(1);
        }
        return false;
    }
    bool Call(std::uint64_t function, std::uint64_t a1, std::uint64_t a2,
              std::uint64_t a3, std::uint64_t a4, std::uint64_t& result,
              DWORD timeout = 1500) {
        return Call5(function, a1, a2, a3, a4, 0, result, timeout);
    }
    bool Alive() {
        std::lock_guard<std::mutex> guard(lock_);
        return thread_ && WaitForSingleObject(thread_, 0) == WAIT_TIMEOUT;
    }
    bool Idle() {
        std::lock_guard<std::mutex> guard(lock_);
        if (!thread_ || WaitForSingleObject(thread_, 0) != WAIT_TIMEOUT || !remotePacket_)
            return false;
        std::uint64_t state = 0;
        return process_.Read(remotePacket_, state) && state == 0;
    }
    void Stop() {
        std::lock_guard<std::mutex> guard(lock_);
        bool ended = true;
        if (thread_) {
            if (WaitForSingleObject(thread_, 0) == WAIT_TIMEOUT && remotePacket_) {
                ended = false;
                const std::uint32_t stop = 1;
                for (int i = 0; i < 40; ++i) {
                    process_.WriteBytes(remotePacket_ + 4, &stop, 4);
                    if (WaitForSingleObject(thread_, 50) == WAIT_OBJECT_0) { ended = true; break; }
                }
            }
            CloseHandle(thread_);
            thread_ = nullptr;
        }
        if (ended && process_.Handle()) {
            if (remoteCode_) VirtualFreeEx(process_.Handle(), reinterpret_cast<LPVOID>(remoteCode_), 0, MEM_RELEASE);
            if (remotePacket_) VirtualFreeEx(process_.Handle(), reinterpret_cast<LPVOID>(remotePacket_), 0, MEM_RELEASE);
        }
        remoteCode_ = remotePacket_ = 0;
    }
private:
    ProcessMemory& process_;
    HANDLE thread_ = nullptr;
    std::uint64_t remoteCode_ = 0;
    std::uint64_t remotePacket_ = 0;
    std::mutex lock_;
};

struct GameProcess {
    DWORD pid = 0;
    HWND window = nullptr;
    std::wstring title;
    std::int32_t roleID = 0;
    std::wstring characterName;
};

struct WindowCollector {
    std::map<DWORD, std::pair<HWND, std::wstring>> windows;
};

static BOOL CALLBACK CollectWindow(HWND window, LPARAM parameter) {
    if (!IsWindowVisible(window) || GetWindowTextLengthW(window) <= 0) return TRUE;
    DWORD pid = 0;
    GetWindowThreadProcessId(window, &pid);
    if (!pid) return TRUE;
    wchar_t title[512]{};
    GetWindowTextW(window, title, 512);
    auto& data = *reinterpret_cast<WindowCollector*>(parameter);
    if (!data.windows.count(pid)) data.windows[pid] = {window, title};
    return TRUE;
}

static std::vector<GameProcess> FindGames() {
    WindowCollector collector;
    EnumWindows(CollectWindow, reinterpret_cast<LPARAM>(&collector));
    std::vector<GameProcess> result;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return result;
    PROCESSENTRY32W item{};
    item.dwSize = sizeof(item);
    if (Process32FirstW(snap, &item)) {
        do {
            ProcessMemory process;
            if (!process.Open(item.th32ProcessID, false) || !process.FindModule(kModule).base) continue;
            GameProcess game;
            game.pid = item.th32ProcessID;
            const auto found = collector.windows.find(game.pid);
            if (found != collector.windows.end()) {
                game.window = found->second.first;
                game.title = found->second.second;
            } else {
                game.title = L"Thần Long Mobile";
            }
            result.push_back(std::move(game));
        } while (Process32NextW(snap, &item));
    }
    CloseHandle(snap);
    std::sort(result.begin(), result.end(), [](const auto& a, const auto& b) { return a.pid < b.pid; });
    return result;
}

struct Spot {
    std::wstring name;
    std::int32_t mapID = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct SkillOption {
    std::int32_t id = 0;
    std::wstring name;
};

struct SellNpc {
    std::wstring name;
    std::int32_t resID = 0;
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
};

// IDs recovered directly from the decrypted client Config.unity3d, not inferred from RoleID.
// Both records are in NPC/NPCData for MapID 5 in this exact client build.
static std::int32_t BuiltinNpcResID(const std::wstring& name) {
    const std::wstring key = CompactMatch(name);
    if (key == L"makieuminh") return 373;
    if (key == L"dothanhdang") return 339;
    // Config.unity3d: <NPC ID="463" Name="Long Phá Thiên" .../>.
    // NPCData binds that template to normal Lạc Dương (Map 3); the same NPC template is
    // reused by the user's Lạc Dương Liên Server scene (runtime Map 10000).
    if (key == L"longphathien") return 463;
    return 0;
}

static std::int32_t BuiltinNpcMapID(const std::wstring& name) {
    const std::wstring key = CompactMatch(name);
    if (key == L"longphathien") return 10000;
    if (key == L"makieuminh" || key == L"dothanhdang") return 5;
    return 0;
}

enum class TrainActivationMode { CtrlTabSkill = 1, AutoFight = 2 };

struct TrainConfig {
    TrainActivationMode activation = TrainActivationMode::AutoFight;
    int tolerance = 120;
    int retrySeconds = 12;
    int skillID = 0;
    bool autoSell = false;
    int bagCheckMinutes = 5;
    std::wstring sellNpcName;
    bool healAtStart = false;
    std::wstring healNpcName;
    bool autoBuff = false;
    std::vector<int> buffSkillIDs;
    std::map<int, int> buffMap;
};

struct LiveState {
    bool connected = false;
    bool running = false;
    bool riding = false;
    bool autoFight = false;
    bool moving = false;
    bool autoPathing = false;
    bool dead = false;
    bool mapReady = false;
    bool waitingChangeMap = false;
    bool messageBoxVisible = false;
    std::int32_t mapID = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t roleID = 0;
    std::int32_t freeBagSpace = -1;
    std::wstring characterName;
    std::wstring phase = L"Đã dừng";
    std::wstring detail;
    std::uint64_t sequence = 0;
};

static std::wstring SpotsPath() { return ExeDirectory() + L"\\ThanLongAutoTrain.spots.txt"; }
static std::wstring LegacyNpcsPath() { return ExeDirectory() + L"\\ThanLongAutoTrain.npcs.txt"; }
static std::wstring SellNpcsPath() { return ExeDirectory() + L"\\ThanLongAutoTrain.sell_npcs.txt"; }
static std::wstring HealNpcsPath() { return ExeDirectory() + L"\\ThanLongAutoTrain.heal_npcs.txt"; }
static std::wstring ConfigPath() { return ExeDirectory() + L"\\ThanLongAutoTrain.ini"; }

static std::vector<Spot> LoadSpots() {
    std::vector<Spot> result;
    HANDLE file = CreateFileW(SpotsPath().c_str(), GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return result;
    const DWORD size = GetFileSize(file, nullptr);
    std::string bytes(size, '\0');
    DWORD done = 0;
    if (size && !ReadFile(file, bytes.data(), size, &done, nullptr)) bytes.clear();
    CloseHandle(file);
    if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xEF &&
        static_cast<unsigned char>(bytes[1]) == 0xBB && static_cast<unsigned char>(bytes[2]) == 0xBF) {
        bytes.erase(0, 3);
    }
    const std::wstring text = Wide(bytes);
    std::wstring line;
    auto parse = [&](const std::wstring& value) {
        if (value.empty() || value[0] == L'#') return;
        const auto parts = SplitTabs(value);
        if (parts.size() != 4) return;
        Spot spot{Trim(parts[0]), _wtoi(parts[1].c_str()), _wtoi(parts[2].c_str()),
                  _wtoi(parts[3].c_str())};
        if (!spot.name.empty() && spot.mapID > 0) result.push_back(std::move(spot));
    };
    for (wchar_t c : text) {
        if (c == L'\r') continue;
        if (c == L'\n') { parse(line); line.clear(); }
        else line.push_back(c);
    }
    parse(line);
    return result;
}

static bool SaveSpots(const std::vector<Spot>& spots) {
    std::wstring text = L"# ThanLongAutoTrain spots v1\r\n# Ten bai<TAB>MapID<TAB>X<TAB>Y\r\n";
    for (Spot spot : spots) {
        for (wchar_t& c : spot.name) if (c == L'\t' || c == L'\r' || c == L'\n') c = L' ';
        text += spot.name + L"\t" + std::to_wstring(spot.mapID) + L"\t" +
                std::to_wstring(spot.x) + L"\t" + std::to_wstring(spot.y) + L"\r\n";
    }
    const std::string body = "\xEF\xBB\xBF" + Utf8(text);
    HANDLE file = CreateFileW(SpotsPath().c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD done = 0;
    const bool ok = WriteFile(file, body.data(), static_cast<DWORD>(body.size()), &done, nullptr) &&
                    done == body.size();
    CloseHandle(file);
    return ok;
}


static std::vector<SellNpc> LoadNpcsFrom(const std::wstring& path) {
    std::vector<SellNpc> result;
    HANDLE file = CreateFileW(path.c_str(), GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return result;
    const DWORD size = GetFileSize(file, nullptr);
    std::string bytes(size, '\0');
    DWORD done = 0;
    if (size && !ReadFile(file, bytes.data(), size, &done, nullptr)) bytes.clear();
    CloseHandle(file);
    if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xEF &&
        static_cast<unsigned char>(bytes[1]) == 0xBB && static_cast<unsigned char>(bytes[2]) == 0xBF)
        bytes.erase(0, 3);
    const std::wstring text = Wide(bytes);
    std::wstring line;
    auto parse = [&](const std::wstring& value) {
        if (value.empty() || value[0] == L'#') return;
        const auto parts = SplitTabs(value);
        SellNpc npc;
        if (parts.size() == 6) {
            npc = SellNpc{Trim(parts[0]), _wtoi(parts[1].c_str()), _wtoi(parts[2].c_str()),
                          _wtoi(parts[3].c_str()), _wtoi(parts[4].c_str()),
                          _wtoi(parts[5].c_str())};
        } else if (parts.size() == 5) {
            npc = SellNpc{Trim(parts[0]), 0, _wtoi(parts[1].c_str()),
                          _wtoi(parts[2].c_str()), _wtoi(parts[3].c_str()),
                          _wtoi(parts[4].c_str())};
        } else return;
        const std::int32_t builtinResID = BuiltinNpcResID(npc.name);
        if (builtinResID > 0) {
            npc.resID = builtinResID;
            if (npc.mapID <= 0) npc.mapID = BuiltinNpcMapID(npc.name);
        }
        if (!npc.name.empty() && npc.roleID > 0 && npc.mapID > 0) result.push_back(std::move(npc));
    };
    for (wchar_t c : text) {
        if (c == L'\r') continue;
        if (c == L'\n') { parse(line); line.clear(); }
        else line.push_back(c);
    }
    parse(line);
    return result;
}

static bool SaveNpcsTo(const std::wstring& path, const std::vector<SellNpc>& npcs,
                       const wchar_t* purpose) {
    std::wstring text = L"# ThanLongAutoTrain NPCs v4\r\n# Danh sach rieng: ";
    text += purpose;
    text += L"\r\n# Ten NPC<TAB>ResID<TAB>RoleID<TAB>MapID<TAB>X<TAB>Y\r\n";
    for (SellNpc npc : npcs) {
        for (wchar_t& c : npc.name) if (c == L'\t' || c == L'\r' || c == L'\n') c = L' ';
        text += npc.name + L"\t" + std::to_wstring(npc.resID) + L"\t" +
                std::to_wstring(npc.roleID) + L"\t" + std::to_wstring(npc.mapID) + L"\t" +
                std::to_wstring(npc.x) + L"\t" + std::to_wstring(npc.y) + L"\r\n";
    }
    const std::string body = "\xEF\xBB\xBF" + Utf8(text);
    HANDLE file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr,
                              CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) return false;
    DWORD done = 0;
    const bool ok = WriteFile(file, body.data(), static_cast<DWORD>(body.size()), &done, nullptr) &&
                    done == body.size();
    CloseHandle(file);
    return ok;
}

static int IniInt(const std::wstring& section, const wchar_t* key, int fallback) {
    int inherited = fallback;
    if (_wcsicmp(section.c_str(), L"AutoTrain") != 0) {
        inherited = static_cast<int>(GetPrivateProfileIntW(
            L"AutoTrain", key, fallback, ConfigPath().c_str()));
    }
    return static_cast<int>(GetPrivateProfileIntW(
        section.c_str(), key, inherited, ConfigPath().c_str()));
}

static std::wstring IniText(const std::wstring& section, const wchar_t* key) {
    std::array<wchar_t, 1024> buffer{};
    GetPrivateProfileStringW(section.c_str(), key, L"", buffer.data(),
                             static_cast<DWORD>(buffer.size()), ConfigPath().c_str());
    return buffer.data();
}

static void WriteIniInt(const std::wstring& section, const wchar_t* key, int value) {
    WritePrivateProfileStringW(section.c_str(), key, std::to_wstring(value).c_str(),
                               ConfigPath().c_str());
}

static void WriteIniText(const std::wstring& section, const wchar_t* key,
                         const std::wstring& value) {
    WritePrivateProfileStringW(section.c_str(), key, value.c_str(), ConfigPath().c_str());
}

static std::vector<int> ParseIdList(const std::wstring& text) {
    std::vector<int> out;
    std::set<int> seen;
    std::wstring token;
    auto flush = [&] {
        const int id = _wtoi(Trim(token).c_str());
        if (id > 0 && seen.insert(id).second) out.push_back(id);
        token.clear();
    };
    for (wchar_t c : text) {
        if (c == L',' || c == L';' || c == L'|') flush();
        else token.push_back(c);
    }
    flush();
    return out;
}

static std::wstring JoinIdList(const std::vector<int>& ids) {
    std::wstring out;
    for (int id : ids) {
        if (id <= 0) continue;
        if (!out.empty()) out += L",";
        out += std::to_wstring(id);
    }
    return out;
}

static std::map<int, int> ParseBuffMap(const std::wstring& text) {
    std::map<int, int> out;
    std::wstring pair;
    auto flush = [&] {
        const std::size_t colon = pair.find(L':');
        if (colon != std::wstring::npos) {
            const int skill = _wtoi(Trim(pair.substr(0, colon)).c_str());
            const int buff = _wtoi(Trim(pair.substr(colon + 1)).c_str());
            if (skill > 0 && buff > 0) out[skill] = buff;
        }
        pair.clear();
    };
    for (wchar_t c : text) {
        if (c == L',' || c == L';' || c == L'|') flush();
        else pair.push_back(c);
    }
    flush();
    return out;
}

static std::wstring JoinBuffMap(const std::map<int, int>& values) {
    std::wstring out;
    for (const auto& [skill, buff] : values) {
        if (skill <= 0 || buff <= 0) continue;
        if (!out.empty()) out += L",";
        out += std::to_wstring(skill) + L":" + std::to_wstring(buff);
    }
    return out;
}

static std::wstring ProfileSection(std::int32_t roleID, DWORD pid) {
    return roleID > 0 ? L"AutoTrain.Role" + std::to_wstring(roleID)
                      : L"AutoTrain.PID" + std::to_wstring(pid);
}

static TrainConfig LoadConfig(const std::wstring& section = L"AutoTrain") {
    TrainConfig c;
    const int rawActivation = IniInt(section, L"TrainActivation", 2);
    c.activation = rawActivation == static_cast<int>(TrainActivationMode::CtrlTabSkill)
        ? TrainActivationMode::CtrlTabSkill : TrainActivationMode::AutoFight;
    c.tolerance = std::clamp(IniInt(section, L"Tolerance", 120), 20, 2000);
    c.retrySeconds = std::clamp(IniInt(section, L"RetrySeconds", 12), 5, 60);
    c.skillID = std::max(IniInt(section, L"SkillID", 0), 0);
    c.autoSell = IniInt(section, L"AutoSell", 0) != 0;
    c.bagCheckMinutes = std::clamp(IniInt(section, L"BagCheckMinutes", 5), 1, 180);
    c.sellNpcName = IniText(section, L"SellNpcName");
    c.healAtStart = IniInt(section, L"HealAtStart", 0) != 0;
    c.healNpcName = IniText(section, L"HealNpcName");
    c.autoBuff = IniInt(section, L"AutoBuff", 0) != 0;
    c.buffSkillIDs = ParseIdList(IniText(section, L"BuffSkillIDs"));
    c.buffMap = ParseBuffMap(IniText(section, L"BuffMap"));
    return c;
}

static void SaveConfig(const std::wstring& section, const TrainConfig& c,
                       const std::wstring& spotName = L"") {
    WriteIniInt(section, L"TrainActivation", static_cast<int>(c.activation));
    WriteIniInt(section, L"Tolerance", c.tolerance);
    WriteIniInt(section, L"RetrySeconds", c.retrySeconds);
    WriteIniInt(section, L"SkillID", c.skillID);
    WriteIniInt(section, L"AutoSell", c.autoSell ? 1 : 0);
    WriteIniInt(section, L"BagCheckMinutes", c.bagCheckMinutes);
    WriteIniText(section, L"SellNpcName", c.sellNpcName);
    WriteIniInt(section, L"HealAtStart", c.healAtStart ? 1 : 0);
    WriteIniText(section, L"HealNpcName", c.healNpcName);
    WriteIniInt(section, L"AutoBuff", c.autoBuff ? 1 : 0);
    WriteIniText(section, L"BuffSkillIDs", JoinIdList(c.buffSkillIDs));
    WriteIniText(section, L"BuffMap", JoinBuffMap(c.buffMap));
    WriteIniText(section, L"SpotName", spotName);
}




static LPARAM KeyMessageData(WORD key, bool released) {
    const UINT scan = MapVirtualKeyW(key, MAPVK_VK_TO_VSC);
    LPARAM data = 1 | (static_cast<LPARAM>(scan) << 16);
    if (released) data |= (static_cast<LPARAM>(1) << 30) |
                          (static_cast<LPARAM>(1) << 31);
    return data;
}

static bool PressGameKey(HWND game, WORD key, bool control = false) {
    if (!game || !IsWindow(game)) return false;
    bool ok = true;
    if (control) {
        ok = PostMessageW(game, WM_KEYDOWN, VK_CONTROL,
                          KeyMessageData(VK_CONTROL, false)) != FALSE;
    }
    ok = (PostMessageW(game, WM_KEYDOWN, key, KeyMessageData(key, false)) != FALSE) && ok;
    ok = (PostMessageW(game, WM_KEYUP, key, KeyMessageData(key, true)) != FALSE) && ok;
    if (control) {
        ok = (PostMessageW(game, WM_KEYUP, VK_CONTROL,
                           KeyMessageData(VK_CONTROL, true)) != FALSE) && ok;
    }
    return ok;
}




class TrainSession {
public:
    ~TrainSession() { Stop(); }
    bool Probe(const GameProcess& game, LiveState& output, std::wstring& error) {
        std::lock_guard<std::mutex> operation(operation_);
        if (running_) { output = State(); return output.connected; }
        Cleanup();
        game_ = game;
        if (!Initialize(error)) { Cleanup(); return false; }
        UpdateLive(true, L"Sẵn sàng", L"Đã đọc MapID và tọa độ realtime");
        output = State();
        Cleanup();
        return true;
    }
    bool ProbeSkills(const GameProcess& game, std::vector<SkillOption>& output,
                     std::wstring& error) {
        std::lock_guard<std::mutex> operation(operation_);
        if (running_) {
            error = L"Hãy dừng Auto Train trước khi đọc lại skill";
            return false;
        }
        Cleanup();
        game_ = game;
        if (!Initialize(error) || !EnumerateSkills(output, error)) {
            Cleanup();
            return false;
        }
        Cleanup();
        return true;
    }
    bool ProbeNearestNpc(const GameProcess& game, SellNpc& output, std::wstring& error) {
        std::lock_guard<std::mutex> operation(operation_);
        const bool temporary = !running_.load();
        if (!temporary && game_.pid != game.pid) {
            error = L"Session đang gắn với PID khác";
            return false;
        }
        if (temporary) {
            Cleanup();
            game_ = game;
            if (!Initialize(error)) {
                Cleanup();
                return false;
            }
        }
        if (!RefreshLive(error)) {
            if (temporary) Cleanup();
            return false;
        }
        const LiveState live = State();
        // GetNearestNPC() gives the nearby LuaMapObjectData, which exposes RoleID + Name.
        // ClickNPC(int), however, does NOT consume RoleID: its native predicate compares the
        // argument with GNPC.ResID.  Resolve the live GNPC/GMovingNPC from SessionData by RoleID
        // and persist the real ResID so opening the NPC uses the same identifier as the game.
        std::int32_t currentRoleID = 0;
        std::wstring currentName;
        if (!ReadNearestNpcIdentity(currentRoleID, currentName, error)) {
            error = L"Không tìm thấy NPC gần nhân vật. Hãy đứng sát NPC cần lưu rồi thử lại. • " + error;
            if (temporary) Cleanup();
            return false;
        }
        std::int32_t currentResID = BuiltinNpcResID(currentName);
        std::wstring resDetail;
        if (currentResID > 0) {
            resDetail = L"ID lấy trực tiếp từ Config.unity3d của client: ResID " +
                        std::to_wstring(currentResID);
        } else if (!ResolveNpcResIDByRoleID(currentRoleID, currentName, currentResID, resDetail)) {
            error = L"Đã thấy NPC “" +
                    (currentName.empty() ? std::to_wstring(currentRoleID) : currentName) +
                    L"” nhưng chưa lấy được ResID thật • " + resDetail;
            if (temporary) Cleanup();
            return false;
        }
        output.resID = currentResID;
        output.roleID = currentRoleID;
        output.name = currentName.empty() ? L"NPC " + std::to_wstring(output.resID) : currentName;
        output.mapID = live.mapID;
        output.x = live.x;
        output.y = live.y;
        if (temporary) Cleanup();
        return true;
    }

    void Start(const GameProcess& game, Spot target, TrainConfig config,
               SellNpc sellNpc = {}, SellNpc healNpc = {}) {
        Stop();
        game_ = game;
        target_ = std::move(target);
        config_ = config;
        sellNpc_ = std::move(sellNpc);
        healNpc_ = std::move(healNpc);
        running_ = true;
        worker_ = std::thread([this] { Worker(); });
    }
    void Stop() {
        running_ = false;
        if (worker_.joinable()) worker_.join();
        std::lock_guard<std::mutex> operation(operation_);
        DisableActions();
        Cleanup();
        UpdateLive(false, L"Đã dừng", L"");
    }
    bool Running() const { return running_.load(); }
    LiveState State() const {
        std::lock_guard<std::mutex> guard(stateLock_);
        return state_;
    }
private:
    bool Remote(std::uint64_t method, std::uint64_t a1, std::uint64_t a2,
                std::uint64_t a3, std::uint64_t a4, std::uint64_t& result,
                DWORD timeout = 1500) {
        return executor_ && executor_->Call(module_.base + method, a1, a2, a3, a4, result, timeout);
    }
    bool Remote5(std::uint64_t method, std::uint64_t a1, std::uint64_t a2,
                 std::uint64_t a3, std::uint64_t a4, std::uint64_t a5,
                 std::uint64_t& result, DWORD timeout = 1500) {
        return executor_ && executor_->Call5(module_.base + method, a1, a2, a3, a4, a5,
                                              result, timeout);
    }
    bool RemoteAbsolute(std::uint64_t function, std::uint64_t a1, std::uint64_t a2,
                        std::uint64_t a3, std::uint64_t a4, std::uint64_t& result,
                        DWORD timeout = 1500) {
        return executor_ && function &&
               executor_->Call(function, a1, a2, a3, a4, result, timeout);
    }
    bool Validate(std::wstring& error) {
        IMAGE_DOS_HEADER dos{};
        IMAGE_NT_HEADERS64 nt{};
        if (!process_.Read(module_.base, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE ||
            !process_.Read(module_.base + static_cast<std::uint32_t>(dos.e_lfanew), nt) ||
            nt.Signature != IMAGE_NT_SIGNATURE || nt.FileHeader.TimeDateStamp != 0x6A410C14u ||
            nt.OptionalHeader.SizeOfImage != 0x03DCB000u) {
            error = L"Sai phiên bản game; Auto Train đã chặn để tránh gọi nhầm hàm";
            return false;
        }
        struct Signature { std::uint64_t rva; std::array<unsigned char, 12> bytes; };
        static constexpr Signature signatures[] = {
            {rva::LuaGetRoleData,       {0x48,0x83,0xEC,0x28,0x80,0x3D,0xC2,0xCA,0x14,0x03,0x00,0x75}},
            {rva::LuaPlayerGetRoleID,   {0x48,0x83,0xEC,0x28,0x48,0x8B,0x41,0x10,0x48,0x85,0xC0,0x74}},
            {rva::LuaPlayerGetName,     {0x48,0x83,0xEC,0x28,0x48,0x8B,0x41,0x10,0x48,0x85,0xC0,0x74}},
            {rva::LuaLeaderIsDeath,     {0x48,0x83,0xEC,0x28,0x80,0x3D,0x4B,0xAB,0x2B,0x03,0x00,0x75}},
            {0x4A6510,                  {0x8B,0x41,0x50,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {0x530260,                  {0x8B,0x41,0x54,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {0x54E130,                  {0x8B,0x41,0x58,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaIsRiding,          {0x48,0x83,0xEC,0x28,0x80,0x3D,0x59,0xFF,0x14,0x03,0x00,0x75}},
            {rva::LuaCurrentMountSlot,  {0x48,0x83,0xEC,0x28,0x80,0x3D,0xA6,0xD1,0x14,0x03,0x00,0x75}},
            {rva::LuaToggleRide,        {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xA5,0xE8,0x14,0x03}},
            {rva::LuaGetAutoFightEnabled,         {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x5E,0xCB,0x14,0x03}},
            {rva::LuaGetFreeBagSpace,    {0x48,0x83,0xEC,0x28,0x80,0x3D,0xB2,0x68,0x15,0x03,0x00,0x75}},
            {rva::LuaGetItemsAtSite,      {0x33,0xD2,0xE9,0x39,0x84,0xEA,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaGetItemData,         {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x6F,0x5E,0x15,0x03}},
            {rva::LuaGetItemType,         {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xAE,0x5A,0x15,0x03}},
            {rva::LuaIsItemSellable,      {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xA6,0x04,0x15,0x03}},
            {rva::LuaItemGetID,           {0x8B,0x41,0x10,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaItemGetItemID,       {0x8B,0x41,0x14,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaItemGetPosition,     {0x8B,0x41,0x1C,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaGetBuffs,           {0x48,0x83,0xEC,0x28,0x80,0x3D,0x98,0x8F,0x15,0x03,0x00,0x75}},
            {rva::LuaHasBuff,            {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xA8,0x09,0x15,0x03}},
            {rva::LuaRequestUsingSkill,  {0x33,0xD2,0xE9,0x09,0x45,0x05,0x00,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaGetNearestNPC,      {0x33,0xD2,0xB9,0xFF,0xFF,0xFF,0xFF,0xE9,0x94,0xBF,0xEA,0xFF}},
            {rva::LuaGetNearestNPCByResID,{0x33,0xD2,0xE9,0xA9,0xBF,0xEA,0xFF,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaClickNPC,           {0x48,0x89,0x5C,0x24,0x10,0x57,0x48,0x83,0xEC,0x30,0x80,0x3D}},
            {rva::SessionGetNPCs,        {0x48,0x83,0xEC,0x28,0x80,0x3D,0xCC,0x45,0x0D,0x03,0x00,0x75}},
            {rva::SessionGetMovingNPCs,  {0x48,0x83,0xEC,0x28,0x80,0x3D,0x1D,0x46,0x0D,0x03,0x00,0x75}},
            {rva::GNPCGetResID,           {0x8B,0x81,0xC0,0x00,0x00,0x00,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaMapObjectGetRoleID, {0x8B,0x41,0x10,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaMapObjectGetName,   {0x48,0x8B,0x41,0x18,0xC3,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaIsMoving,          {0x48,0x83,0xEC,0x28,0x80,0x3D,0x4A,0x00,0x15,0x03,0x00,0x75}},
            {rva::LuaGetSkills,         {0x48,0x83,0xEC,0x28,0x80,0x3D,0xDA,0x2E,0x15,0x03,0x00,0x0F}},
            {rva::LuaGetSkillName,      {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xFD,0x32,0x15,0x03}},
            {rva::LuaHasSkill,          {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x09,0x08,0x15,0x03}},
            {rva::LuaCanUseSkill,       {0x48,0x83,0xEC,0x28,0x80,0x3D,0xC7,0xD6,0x15,0x03,0x00,0x75}},
            {rva::LuaCheckSkillCondition,{0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x82,0xD2,0x15,0x03}},
            {rva::LuaIsSkillCooldown,   {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x74,0xFD,0x14,0x03}},
            {rva::LuaRequestUsingSkillWithTarget,{0x45,0x33,0xC0,0xE9,0xD8,0x47,0x05,0x00,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaGetSelectedTarget, {0x48,0x83,0xEC,0x28,0x80,0x3D,0x55,0xCA,0x14,0x03,0x00,0x75}},
            {rva::SelectedTargetGetRoleID,{0x48,0x83,0xEC,0x28,0x80,0x3D,0xD7,0x0C,0x2A,0x03,0x00,0x75}},
            {rva::LuaIsSelectTargetDie, {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x58,0xF3,0x14,0x03}},
            {rva::LuaIsMapReady,        {0x48,0x83,0xEC,0x28,0x80,0x3D,0xC8,0x01,0x15,0x03,0x00,0x75}},
            {rva::LuaFindUI,            {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xBF,0x17,0x12,0x03}},
            {rva::LuaMainFindUI,        {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x1F,0x16,0x12,0x03}},
            {rva::UIObjectGetName,      {0x48,0x83,0xEC,0x28,0x48,0x8B,0x49,0x30,0x48,0x85,0xC9,0x74}},
            {rva::UIObjectActiveInHierarchy,{0x48,0x83,0xEC,0x28,0x48,0x8B,0x49,0x30,0x48,0x85,0xC9,0x74}},
            {rva::UIObjectCoreChildren, {0x48,0x89,0x5C,0x24,0x10,0x56,0x57,0x41,0x54,0x41,0x56,0x41}},
            {rva::UIButtonGetInteractable,{0x48,0x83,0xEC,0x28,0x48,0x8B,0x81,0xE0,0x00,0x00,0x00,0x48}},
            {rva::UIButtonGetText,      {0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83,0xEC,0x20,0x80,0x3D}},
            {rva::UIButtonHandleClick,  {0x40,0x53,0x48,0x83,0xEC,0x30,0x80,0x3D,0x4B,0xA4,0x29,0x03}},
            {rva::UIToggleGetInteractable,{0x48,0x83,0xEC,0x28,0x48,0x8B,0x89,0xE8,0x00,0x00,0x00,0x48}},
            {rva::UIToggleGetSelected,   {0x48,0x83,0xEC,0x28,0x48,0x8B,0x89,0xE8,0x00,0x00,0x00,0x48}},
            {rva::UIToggleGetText,       {0x48,0x83,0xEC,0x28,0x48,0x8B,0x89,0xE8,0x00,0x00,0x00,0x48}},
            {rva::UIToggleSetSelected,   {0x48,0x83,0xEC,0x28,0x48,0x8B,0x89,0xE8,0x00,0x00,0x00,0x48}},
            {rva::UIToggleHandleSelectEvent,{0x48,0x89,0x5C,0x24,0x10,0x48,0x89,0x4C,0x24,0x08,0x56,0x57}},
            {rva::MonoBehaviourExecutorGetInstance,{0x48,0x83,0xEC,0x28,0x80,0x3D,0x7C,0x38,0x2A,0x03,0x00,0x75}},
            {rva::MonoBehaviourExecutorExecuteUiObject,{0x40,0x53,0x55,0x56,0x57,0x48,0x83,0xEC,0x38,0x80,0x3D,0x4D}},
            {rva::SessionWaitingChangeMap,{0x48,0x83,0xEC,0x28,0x80,0x3D,0xF7,0x3F,0x0D,0x03,0x00,0x75}},
            {rva::AutoPathGetInstance,  {0x48,0x83,0xEC,0x28,0x80,0x3D,0x1D,0x23,0x1B,0x03,0x00,0x75}},
            {rva::AutoPathStart,        {0x40,0x53,0x55,0x56,0x57,0x48,0x83,0xEC,0x48,0x80,0x3D,0x8A}},
            {rva::AutoPathStop,         {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x8E,0x23,0x1B,0x03}},
            {rva::AutoPathIsRunning,    {0x48,0x83,0x79,0x28,0x00,0x0F,0x97,0xC0,0xC3,0xCC,0xCC,0xCC}},
        };
        for (const auto& sig : signatures) {
            std::array<unsigned char, 12> found{};
            if (!process_.ReadBytes(module_.base + sig.rva, found.data(), found.size()) ||
                found != sig.bytes) {
                error = L"Chữ ký hàm Auto Train không khớp phiên bản game";
                return false;
            }
        }
        return true;
    }
    bool WriteScratch(std::size_t offset, const std::string& value) {
        if (!scratchBuffer_ || offset + value.size() + 1 > 1024) return false;
        return process_.WriteBytes(scratchBuffer_ + offset, value.c_str(), value.size() + 1);
    }
    bool ResolveClass(const std::string& nameSpace, const std::string& name,
                      std::uint64_t& output) {
        output = 0;
        if (!WriteScratch(256, nameSpace) || !WriteScratch(512, name)) return false;
        return RemoteAbsolute(il2cppClassFromName_, assemblyImage_, scratchBuffer_ + 256,
                              scratchBuffer_ + 512, 0, output, 1200) && output;
    }
    bool InitializeUiRuntime(std::wstring& error) {
        il2cppDomainGet_ = process_.ResolveExport(module_, "il2cpp_domain_get");
        il2cppDomainAssemblyOpen_ = process_.ResolveExport(module_, "il2cpp_domain_assembly_open");
        il2cppAssemblyGetImage_ = process_.ResolveExport(module_, "il2cpp_assembly_get_image");
        il2cppClassFromName_ = process_.ResolveExport(module_, "il2cpp_class_from_name");
        il2cppClassIsAssignableFrom_ =
            process_.ResolveExport(module_, "il2cpp_class_is_assignable_from");
        il2cppClassGetField_ = process_.ResolveExport(module_, "il2cpp_class_get_field_from_name");
        il2cppFieldStaticGetValue_ = process_.ResolveExport(module_, "il2cpp_field_static_get_value");
        il2cppGcHandleNew_ = process_.ResolveExport(module_, "il2cpp_gchandle_new");
        il2cppGcHandleGetTarget_ = process_.ResolveExport(module_, "il2cpp_gchandle_get_target");
        il2cppGcHandleFree_ = process_.ResolveExport(module_, "il2cpp_gchandle_free");
        il2cppGetCorlib_ = process_.ResolveExport(module_, "il2cpp_get_corlib");
        il2cppArrayNew_ = process_.ResolveExport(module_, "il2cpp_array_new");
        il2cppValueBox_ = process_.ResolveExport(module_, "il2cpp_value_box");
        scratchBuffer_ = process_.Allocate(1024);
        staticValueBuffer_ = process_.Allocate(sizeof(std::uint64_t));
        if (!il2cppDomainGet_ || !il2cppDomainAssemblyOpen_ || !il2cppAssemblyGetImage_ ||
            !il2cppClassFromName_ || !il2cppClassIsAssignableFrom_ ||
            !il2cppClassGetField_ || !il2cppFieldStaticGetValue_ ||
            !il2cppGcHandleNew_ || !il2cppGcHandleGetTarget_ || !il2cppGcHandleFree_ ||
            !il2cppGetCorlib_ || !il2cppArrayNew_ || !il2cppValueBox_ ||
            !scratchBuffer_ || !staticValueBuffer_) {
            error = L"Thiếu API phản chiếu UI nội bộ của game";
            return false;
        }
        std::uint64_t domain = 0, assembly = 0;
        if (!RemoteAbsolute(il2cppDomainGet_, 0, 0, 0, 0, domain, 1000) || !domain ||
            !WriteScratch(0, "Assembly-CSharp") ||
            !RemoteAbsolute(il2cppDomainAssemblyOpen_, domain, scratchBuffer_, 0, 0,
                            assembly, 1500)) {
            error = L"Không mở được domain Assembly-CSharp";
            return false;
        }
        if (!assembly) {
            if (!WriteScratch(0, "Assembly-CSharp.dll") ||
                !RemoteAbsolute(il2cppDomainAssemblyOpen_, domain, scratchBuffer_, 0, 0,
                                assembly, 1500) || !assembly) {
                error = L"Không tìm thấy Assembly-CSharp trong IL2CPP";
                return false;
            }
        }
        if (!RemoteAbsolute(il2cppAssemblyGetImage_, assembly, 0, 0, 0,
                            assemblyImage_, 1000) || !assemblyImage_ ||
            !ResolveClass("FGStudio.LuaSystem.Base", "UIObject", uiObjectClass_) ||
            !ResolveClass("FGStudio.LuaSystem.GUI", "UIButton", uiButtonClass_) ||
            !ResolveClass("FGStudio.LuaSystem.GUI", "UIToggle", uiToggleClass_) ||
            !WriteScratch(768, "instances") ||
            !RemoteAbsolute(il2cppClassGetField_, uiObjectClass_, scratchBuffer_ + 768,
                            0, 0, uiInstancesField_, 1000) || !uiInstancesField_) {
            error = L"Không định vị được UI control/instances trong game";
            return false;
        }
        std::uint64_t corlib = 0;
        if (!RemoteAbsolute(il2cppGetCorlib_, 0, 0, 0, 0, corlib, 1000) || !corlib ||
            !WriteScratch(0, "System") || !WriteScratch(128, "Object") ||
            !RemoteAbsolute(il2cppClassFromName_, corlib, scratchBuffer_, scratchBuffer_ + 128,
                            0, systemObjectClass_, 1000) || !systemObjectClass_ ||
            !WriteScratch(128, "Int32") ||
            !RemoteAbsolute(il2cppClassFromName_, corlib, scratchBuffer_, scratchBuffer_ + 128,
                            0, systemInt32Class_, 1000) || !systemInt32Class_) {
            error = L"Không định vị được System.Object/System.Int32 để gọi callback Lua an toàn";
            return false;
        }
        return true;
    }
    bool Initialize(std::wstring& error) {
        if (!process_.Open(game_.pid)) {
            error = L"Không mở được tiến trình game; hãy chạy quyền Admin";
            return false;
        }
        module_ = process_.FindModule(kModule);
        if (!module_.base || !Validate(error)) return false;
        executor_ = std::make_unique<RemoteExecutor>(process_);
        if (!executor_->Start(module_, error)) return false;
        il2cppStringNew_ = process_.ResolveExport(module_, "il2cpp_string_new");
        if (!InitializeUiRuntime(error)) return false;
        std::uint64_t managedName = 0;
        if (!il2cppStringNew_ || !WriteScratch(0, "MessageBox") ||
            !RemoteAbsolute(il2cppStringNew_, scratchBuffer_, 0, 0, 0,
                            managedName, 900) || !managedName ||
            !RemoteAbsolute(il2cppGcHandleNew_, managedName, 0, 0, 0,
                            messageBoxNameHandle_, 900) || !messageBoxNameHandle_) {
            error = L"Không tạo được bộ nhận diện hộp xác nhận trong game";
            return false;
        }
        std::uint64_t manager = 0;
        if (!Remote(rva::AutoPathGetInstance, 0, 0, 0, 0, manager) || !manager) {
            error = L"AutoPathManager chưa sẵn sàng; hãy vào bản đồ";
            return false;
        }
        autoPathManager_ = manager;
        return RefreshLive(error);
    }
    bool EnumerateSkills(std::vector<SkillOption>& output, std::wstring& error) {
        output.clear();
        std::uint64_t dictionary = 0;
        if (!Remote(rva::LuaGetSkills, 0, 0, 0, 0, dictionary, 2500) || !dictionary) {
            error = L"Chưa đọc được danh sách skill";
            return false;
        }
        std::uint64_t entries = 0;
        std::int32_t count = 0;
        if (!process_.Read(dictionary + off::DictionaryEntries, entries) || !entries ||
            !process_.Read(dictionary + off::DictionaryCount, count) || count < 0 || count > 2048) {
            error = L"Danh sách skill không hợp lệ";
            return false;
        }
        std::set<std::int32_t> seen;
        for (std::int32_t i = 0; i < count; ++i) {
            const std::uint64_t entry = entries + off::ArrayData +
                                        static_cast<std::uint64_t>(i) * off::EntrySize;
            std::uint64_t value = 0;
            std::int32_t id = 0;
            if (!process_.Read(entry + off::EntryValue, value) || !value ||
                !process_.Read(value + off::LuaDbSkillID, id) || id <= 0 ||
                !seen.insert(id).second) continue;
            std::uint64_t namePointer = 0;
            std::wstring name;
            if (Remote(rva::LuaGetSkillName, static_cast<std::uint32_t>(id), 0, 0, 0,
                       namePointer, 1800)) {
                name = process_.ReadIl2CppString(namePointer);
            }
            if (name.empty()) name = L"Skill " + std::to_wstring(id);
            output.push_back({id, std::move(name)});
        }
        std::sort(output.begin(), output.end(), [](const SkillOption& a, const SkillOption& b) {
            const int compare = _wcsicmp(a.name.c_str(), b.name.c_str());
            return compare == 0 ? a.id < b.id : compare < 0;
        });
        if (output.empty()) {
            error = L"Nhân vật chưa có skill nào đọc được";
            return false;
        }
        return true;
    }
    void Cleanup() {
        if (executor_ && messageBoxNameHandle_ && il2cppGcHandleFree_) {
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, messageBoxNameHandle_, 0, 0, 0,
                           ignored, 500);
        }
        executor_.reset();
        if (scratchBuffer_) process_.Free(scratchBuffer_);
        if (staticValueBuffer_) process_.Free(staticValueBuffer_);
        process_.Close();
        module_ = {};
        autoPathManager_ = 0;
        il2cppStringNew_ = 0;
        messageBoxNameHandle_ = 0;
        scratchBuffer_ = staticValueBuffer_ = 0;
        il2cppDomainGet_ = il2cppDomainAssemblyOpen_ = il2cppAssemblyGetImage_ = 0;
        il2cppClassFromName_ = il2cppClassIsAssignableFrom_ = 0;
        il2cppClassGetField_ = il2cppFieldStaticGetValue_ = 0;
        il2cppGcHandleNew_ = il2cppGcHandleGetTarget_ = il2cppGcHandleFree_ = 0;
        il2cppGetCorlib_ = il2cppArrayNew_ = il2cppValueBox_ = 0;
        assemblyImage_ = uiObjectClass_ = uiButtonClass_ = uiToggleClass_ =
            uiInstancesField_ = systemObjectClass_ = systemInt32Class_ = 0;
        uiButtonClassCache_.clear();
        uiToggleClassCache_.clear();
    }
    bool FindMessageBox(std::uint64_t& ui) {
        ui = 0;
        std::uint64_t managedName = 0;
        if (!RemoteAbsolute(il2cppGcHandleGetTarget_, messageBoxNameHandle_, 0, 0, 0,
                            managedName, 900) || !managedName ||
            !Remote(rva::LuaMainFindUI, managedName, 0, 0, 0, ui, 900)) return false;
        return true;
    }
    bool ReadMessageBoxVisible(bool& visible) {
        std::uint64_t ui = 0;
        if (!FindMessageBox(ui)) return false;
        visible = ui != 0;
        return true;
    }
    bool IsUiButton(std::uint64_t object) {
        std::uint64_t klass = 0;
        unsigned char disposed = 1;
        if (!object || !process_.Read(object, klass) || !klass ||
            !process_.Read(object + 0x60, disposed) || disposed != 0) return false;
        if (klass == uiButtonClass_) return true;
        const auto cached = uiButtonClassCache_.find(klass);
        if (cached != uiButtonClassCache_.end()) return cached->second;
        std::uint64_t assignable = 0;
        const bool resolved = RemoteAbsolute(il2cppClassIsAssignableFrom_, uiButtonClass_,
                                             klass, 0, 0, assignable, 700);
        const bool isButton = resolved && (assignable & 0xFFu) != 0;
        if (resolved) uiButtonClassCache_[klass] = isButton;
        return isButton;
    }
    bool IsAssignableControl(std::uint64_t object, std::uint64_t targetClass,
                             std::map<std::uint64_t, bool>& cache) {
        std::uint64_t klass = 0;
        unsigned char disposed = 1;
        if (!object || !targetClass || !process_.Read(object, klass) || !klass ||
            !process_.Read(object + 0x60, disposed) || disposed != 0) return false;
        if (klass == targetClass) return true;
        const auto cached = cache.find(klass);
        if (cached != cache.end()) return cached->second;
        std::uint64_t assignable = 0;
        const bool resolved = RemoteAbsolute(il2cppClassIsAssignableFrom_, targetClass,
                                             klass, 0, 0, assignable, 700);
        const bool result = resolved && (assignable & 0xFFu) != 0;
        if (resolved) cache[klass] = result;
        return result;
    }
    bool IsUiToggle(std::uint64_t object) {
        return IsAssignableControl(object, uiToggleClass_, uiToggleClassCache_);
    }
    bool CollectTreeButtons(std::uint64_t root, std::vector<std::uint64_t>& buttons) {
        buttons.clear();
        if (!root) return true;
        std::vector<std::uint64_t> pending{root};
        std::set<std::uint64_t> visited;
        while (!pending.empty() && visited.size() < 2048) {
            const std::uint64_t current = pending.back();
            pending.pop_back();
            if (!current || !visited.insert(current).second) continue;
            if (IsUiButton(current)) buttons.push_back(current);
            std::uint64_t array = 0;
            if (!Remote(rva::UIObjectCoreChildren, current, 0, 0, 0, array, 900))
                return false;
            if (!array) continue;
            std::uint64_t length = 0;
            if (!process_.Read(array + off::ArrayLength, length) || length > 512) return false;
            for (std::uint64_t i = 0; i < length; ++i) {
                std::uint64_t child = 0;
                if (!process_.Read(array + off::ArrayData + i * sizeof(std::uint64_t), child))
                    return false;
                if (child) pending.push_back(child);
            }
        }
        return visited.size() < 2048;
    }
    bool CollectTreeToggles(std::uint64_t root, std::vector<std::uint64_t>& toggles) {
        toggles.clear();
        if (!root) return true;
        std::vector<std::uint64_t> pending{root};
        std::set<std::uint64_t> visited;
        while (!pending.empty() && visited.size() < 2048) {
            const std::uint64_t current = pending.back();
            pending.pop_back();
            if (!current || !visited.insert(current).second) continue;
            if (IsUiToggle(current)) toggles.push_back(current);
            std::uint64_t array = 0;
            if (!Remote(rva::UIObjectCoreChildren, current, 0, 0, 0, array, 900))
                return false;
            if (!array) continue;
            std::uint64_t length = 0;
            if (!process_.Read(array + off::ArrayLength, length) || length > 512) return false;
            for (std::uint64_t i = 0; i < length; ++i) {
                std::uint64_t child = 0;
                if (!process_.Read(array + off::ArrayData + i * sizeof(std::uint64_t), child))
                    return false;
                if (child) pending.push_back(child);
            }
        }
        return visited.size() < 2048;
    }
    bool ReadAllUiButtons(std::vector<std::uint64_t>& buttons) {
        buttons.clear();
        const std::uint64_t zero = 0;
        if (!process_.WriteBytes(staticValueBuffer_, &zero, sizeof(zero))) return false;
        std::uint64_t ignored = 0;
        if (!RemoteAbsolute(il2cppFieldStaticGetValue_, uiInstancesField_,
                            staticValueBuffer_, 0, 0, ignored, 1000)) return false;
        std::uint64_t dictionary = 0, entries = 0;
        std::int32_t count = 0;
        if (!process_.Read(staticValueBuffer_, dictionary) || !dictionary ||
            !process_.Read(dictionary + off::DictionaryEntries, entries) || !entries ||
            !process_.Read(dictionary + off::DictionaryCount, count) ||
            count < 0 || count > 32768) return false;
        for (std::int32_t i = 0; i < count; ++i) {
            std::uint64_t object = 0;
            const std::uint64_t entry = entries + off::ArrayData +
                static_cast<std::uint64_t>(i) * off::EntrySize;
            if (process_.Read(entry + off::EntryValue, object) && IsUiButton(object))
                buttons.push_back(object);
        }
        return true;
    }
    bool InspectToggle(std::uint64_t toggle, std::wstring& name, std::wstring& text) {
        std::uint64_t active = 0, interactable = 0, pointer = 0;
        if (!Remote(rva::UIObjectActiveInHierarchy, toggle, 0, 0, 0, active, 700) ||
            (active & 0xFFu) == 0 ||
            !Remote(rva::UIToggleGetInteractable, toggle, 0, 0, 0, interactable, 700) ||
            (interactable & 0xFFu) == 0) return false;
        if (Remote(rva::UIObjectGetName, toggle, 0, 0, 0, pointer, 700))
            name = process_.ReadIl2CppString(pointer);
        pointer = 0;
        if (Remote(rva::UIToggleGetText, toggle, 0, 0, 0, pointer, 700))
            text = process_.ReadIl2CppString(pointer);
        return true;
    }
    bool InspectButton(std::uint64_t button, std::wstring& name, std::wstring& text) {
        std::uint64_t active = 0, interactable = 0, pointer = 0;
        if (!Remote(rva::UIObjectActiveInHierarchy, button, 0, 0, 0, active, 700) ||
            (active & 0xFFu) == 0 ||
            !Remote(rva::UIButtonGetInteractable, button, 0, 0, 0, interactable, 700) ||
            (interactable & 0xFFu) == 0) return false;
        if (Remote(rva::UIObjectGetName, button, 0, 0, 0, pointer, 700))
            name = process_.ReadIl2CppString(pointer);
        pointer = 0;
        if (Remote(rva::UIButtonGetText, button, 0, 0, 0, pointer, 700))
            text = process_.ReadIl2CppString(pointer);
        return true;
    }
    struct ButtonInfo {
        std::uint64_t object = 0;
        std::wstring name;
        std::wstring text;
        std::wstring handler;
        std::wstring descendants;
        std::wstring label;
        std::wstring nameKey;
        std::wstring textKey;
        std::wstring allKey;
    };
    enum class ButtonRole {
        DauThai, ShopEntry, SellTab, Treatment, TreatmentAck
    };

    void CollectDescendantLabels(std::uint64_t root, std::wstring& output) {
        std::vector<std::uint64_t> pending{root};
        std::set<std::uint64_t> visited;
        while (!pending.empty() && visited.size() < 96) {
            const std::uint64_t current = pending.back();
            pending.pop_back();
            if (!current || !visited.insert(current).second) continue;
            if (current != root) {
                std::uint64_t pointer = 0;
                std::wstring item;
                if (Remote(rva::UIObjectGetName, current, 0, 0, 0, pointer, 500))
                    item = process_.ReadIl2CppString(pointer);
                if (IsUiButton(current) || IsUiToggle(current)) {
                    pointer = 0;
                    const std::uint64_t textMethod = IsUiToggle(current)
                        ? rva::UIToggleGetText : rva::UIButtonGetText;
                    if (Remote(textMethod, current, 0, 0, 0, pointer, 500)) {
                        const std::wstring text = process_.ReadIl2CppString(pointer);
                        if (!text.empty()) item += (item.empty() ? L"" : L" ") + text;
                    }
                }
                item = Trim(item);
                if (!item.empty() && output.find(item) == std::wstring::npos) {
                    if (!output.empty()) output += L" / ";
                    output += item;
                }
            }
            std::uint64_t array = 0;
            if (!Remote(rva::UIObjectCoreChildren, current, 0, 0, 0, array, 500) || !array)
                continue;
            std::uint64_t length = 0;
            if (!process_.Read(array + off::ArrayLength, length) || length > 128) continue;
            for (std::uint64_t i = 0; i < length; ++i) {
                std::uint64_t child = 0;
                if (process_.Read(array + off::ArrayData + i * sizeof(std::uint64_t), child) &&
                    child) pending.push_back(child);
            }
        }
    }
    bool DescribeButton(std::uint64_t button, bool includeDescendants,
                        ButtonInfo& info) {
        info = {};
        if (!InspectButton(button, info.name, info.text)) return false;
        info.object = button;
        if (includeDescendants) CollectDescendantLabels(button, info.descendants);
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
    bool DescribeToggle(std::uint64_t toggle, bool includeDescendants,
                        ButtonInfo& info) {
        info = {};
        if (!InspectToggle(toggle, info.name, info.text)) return false;
        info.object = toggle;
        if (includeDescendants) CollectDescendantLabels(toggle, info.descendants);
        if (!info.name.empty()) info.label = L"Name=“" + info.name + L"”";
        if (!info.text.empty()) {
            if (!info.label.empty()) info.label += L" • ";
            info.label += L"Text=“" + info.text + L"”";
        }
        if (!info.descendants.empty()) {
            if (!info.label.empty()) info.label += L" • ";
            info.label += L"Child=“" + info.descendants + L"”";
        }
        if (info.label.empty()) info.label = L"UIToggle không có nhãn";
        info.nameKey = CompactMatch(info.name);
        info.textKey = CompactMatch(info.text);
        info.allKey = CompactMatch(info.name + L" " + info.text + L" " + info.descendants);
        return true;
    }

    bool ReadActiveButtonInfos(bool includeDescendants,
                               std::vector<ButtonInfo>& output) {
        output.clear();
        std::vector<std::uint64_t> buttons;
        if (!ReadAllUiButtons(buttons)) return false;
        for (const std::uint64_t button : buttons) {
            ButtonInfo info;
            if (DescribeButton(button, includeDescendants, info))
                output.push_back(std::move(info));
        }
        return true;
    }
    static int ScoreButton(const ButtonInfo& info, ButtonRole role) {
        const std::wstring& key = info.allKey;
        switch (role) {
            case ButtonRole::DauThai: {
                int score = 0;
                if (info.textKey == L"dauthai") score = 600;
                else if (ContainsCompact(key, {L"dauthai"})) score = 520;
                if (ContainsCompact(key, {L"cancel", L"close", L"huy", L"thoat"}))
                    score -= 1000;
                return score;
            }
            case ButtonRole::ShopEntry: {
                if (info.textKey == L"muathucuoi") return 760;
                if (ContainsCompact(key, {L"muathucuoi", L"mountshop", L"buymount",
                                          L"shopmount", L"npcmount", L"mountstore"})) return 580;
                if (ContainsCompact(key, {L"banvatpham", L"quicksell", L"trangbi"})) return -1000;
                return 0;
            }
            case ButtonRole::SellTab: {
                if (info.textKey == L"banvatpham") return 700;
                if (ContainsCompact(key, {L"banvatphamnhanh", L"quicksell"})) return -1000;
                if (ContainsCompact(key, {L"banvatpham", L"sellitem", L"itemsell",
                                          L"selltab", L"tabsell"})) return 520;
                return 0;
            }
            case ButtonRole::Treatment: {
                if (info.textKey == L"trilieu" || info.textKey == L"trithuong" ||
                    info.textKey == L"hoiphuc") return 760;
                if (ContainsCompact(key, {L"trilieu", L"trithuong", L"hoiphuc", L"treatment",
                                          L"treat", L"heal", L"recoverhp", L"recover"})) return 560;
                return 0;
            }
            case ButtonRole::TreatmentAck: {
                if (info.textKey == L"tabietroi") return 820;
                if (ContainsCompact(key, {L"tabietroi", L"iknow", L"understand", L"gotit"})) return 620;
                return 0;
            }
        }
        return 0;
    }
    static std::wstring ShortButtonList(const std::vector<ButtonInfo>& buttons) {
        std::wstring result;
        const std::size_t limit = std::min<std::size_t>(buttons.size(), 4);
        for (std::size_t i = 0; i < limit; ++i) {
            if (!result.empty()) result += L" | ";
            result += buttons[i].label;
        }
        if (buttons.size() > limit) result += L" | ...";
        return result;
    }
    bool ChooseButton(const std::vector<ButtonInfo>& buttons, ButtonRole role,
                      ButtonInfo& selected, std::wstring& reason) {
        struct Candidate { ButtonInfo info; int score; };
        std::vector<Candidate> candidates;
        for (const ButtonInfo& info : buttons) {
            const int score = ScoreButton(info, role);
            if (score > 0) candidates.push_back({info, score});
        }
        std::sort(candidates.begin(), candidates.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.score > b.score;
                  });
        if (candidates.empty()) {
            reason = L"Không có UIButton khớp. Đang mở " +
                     std::to_wstring(buttons.size()) + L" nút: " + ShortButtonList(buttons);
            return false;
        }
        if (candidates.size() > 1 && candidates[0].score == candidates[1].score) {
            reason = L"Có nhiều UIButton cùng khớp; không bấm mù: " +
                     candidates[0].info.label + L" | " + candidates[1].info.label;
            return false;
        }
        selected = std::move(candidates[0].info);
        return true;
    }
    bool FindButton(ButtonRole role, ButtonInfo& selected, std::wstring& reason,
                    bool inspectDescendants) {
        std::vector<ButtonInfo> buttons;
        if (!ReadActiveButtonInfos(false, buttons)) {
            reason = L"Không đọc được UIObject.instances/UIButton";
            return false;
        }
        if (ChooseButton(buttons, role, selected, reason)) return true;
        if (!inspectDescendants) return false;
        if (!ReadActiveButtonInfos(true, buttons)) {
            reason = L"Không đọc được nhãn con của UIButton";
            return false;
        }
        return ChooseButton(buttons, role, selected, reason);
    }
    bool FindButtonInTree(std::uint64_t root, ButtonRole role,
                          ButtonInfo& selected, std::wstring& reason) {
        std::vector<std::uint64_t> raw;
        if (!CollectTreeButtons(root, raw)) {
            reason = L"Không đọc được cây UIButton của Lua UI hiện tại";
            return false;
        }
        std::vector<ButtonInfo> buttons;
        buttons.reserve(raw.size());
        for (const std::uint64_t button : raw) {
            ButtonInfo info;
            if (DescribeButton(button, false, info)) buttons.push_back(std::move(info));
        }
        return ChooseButton(buttons, role, selected, reason);
    }
    bool FindToggleInTree(std::uint64_t root, ButtonRole role,
                          ButtonInfo& selected, std::wstring& reason) {
        std::vector<std::uint64_t> raw;
        if (!CollectTreeToggles(root, raw)) {
            reason = L"Không đọc được cây UIToggle của Lua UI hiện tại";
            return false;
        }
        std::vector<ButtonInfo> toggles;
        toggles.reserve(raw.size());
        for (const std::uint64_t toggle : raw) {
            ButtonInfo info;
            if (DescribeToggle(toggle, false, info)) toggles.push_back(std::move(info));
        }
        return ChooseButton(toggles, role, selected, reason);
    }
    bool InvokeToggle(std::uint64_t toggle) {
        std::uint64_t selected = 0, ignored = 0;
        if (!Remote(rva::UIToggleGetSelected, toggle, 0, 0, 0, selected, 900)) return false;
        if ((selected & 0xFFu) != 0) return true;
        if (Remote(rva::UIToggleSetSelected, toggle, 1, 0, 0, ignored, 1800)) {
            Sleep(60);
            selected = 0;
            if (Remote(rva::UIToggleGetSelected, toggle, 0, 0, 0, selected, 900) &&
                (selected & 0xFFu) != 0) return true;
        }
        // Some Lua-backed toggles do not propagate set_Selected into their script callback.
        // Call the same bool handler used by the game's own Toggle listener, never a fake pointer event.
        if (!Remote(rva::UIToggleHandleSelectEvent, toggle, 1, 0, 0, ignored, 1800)) return false;
        Sleep(60);
        selected = 0;
        if (!Remote(rva::UIToggleGetSelected, toggle, 0, 0, 0, selected, 900)) return false;
        return (selected & 0xFFu) != 0;
    }
    bool CreateEmptyUiArgArray(std::uint64_t& array, std::uint64_t& handle) {
        array = handle = 0;
        if (!systemObjectClass_ || !il2cppArrayNew_ ||
            !RemoteAbsolute(il2cppArrayNew_, systemObjectClass_, 0, 0, 0, array, 1200) || !array ||
            !RemoteAbsolute(il2cppGcHandleNew_, array, 0, 0, 0, handle, 900) || !handle)
            return false;
        return true;
    }
    bool CreateManagedUtf8(const char* text, std::uint64_t& managed, std::uint64_t& handle) {
        managed = handle = 0;
        if (!text || !*text || !WriteScratch(0, std::string(text)) || !il2cppStringNew_ ||
            !RemoteAbsolute(il2cppStringNew_, scratchBuffer_, 0, 0, 0, managed, 1000) || !managed ||
            !RemoteAbsolute(il2cppGcHandleNew_, managed, 0, 0, 0, handle, 900) || !handle)
            return false;
        return true;
    }
    bool CreateBoxedInt32(std::int32_t value, std::uint64_t& boxed,
                          std::uint64_t& handle) {
        boxed = handle = 0;
        if (!systemInt32Class_ || !il2cppValueBox_ || !staticValueBuffer_ ||
            !process_.WriteBytes(staticValueBuffer_, &value, sizeof(value)) ||
            !RemoteAbsolute(il2cppValueBox_, systemInt32Class_, staticValueBuffer_, 0, 0,
                            boxed, 1000) || !boxed ||
            !RemoteAbsolute(il2cppGcHandleNew_, boxed, 0, 0, 0, handle, 900) || !handle)
            return false;
        return true;
    }
    bool InvokeUiScriptIntArg(const char* uiName, const char* functionName,
                              std::int32_t value, std::wstring& detail) {
        std::uint64_t boxed = 0, boxedHandle = 0;
        if (!CreateBoxedInt32(value, boxed, boxedHandle)) {
            detail = L"Không box được tham số số cho Lua action " + Wide(uiName) +
                     L"." + Wide(functionName);
            return false;
        }
        const bool called = InvokeUiScriptOneArg(uiName, functionName, boxed, detail);
        std::uint64_t ignored = 0;
        RemoteAbsolute(il2cppGcHandleFree_, boxedHandle, 0, 0, 0, ignored, 600);
        return called;
    }

    bool WaitForLuaUi(const char* uiName, std::uint64_t& ui, std::wstring& detail,
                      int attempts = 12, int sleepMs = 100, int stableRequired = 1) {
        ui = 0;
        stableRequired = std::max(1, stableRequired);
        std::uint64_t nameObj = 0, nameHandle = 0;
        if (!CreateManagedUtf8(uiName, nameObj, nameHandle)) {
            detail = L"Không tạo được tên Lua UI " + Wide(uiName);
            return false;
        }
        auto freeHandle = [&]() {
            if (!nameHandle) return;
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, nameHandle, 0, 0, 0, ignored, 600);
            nameHandle = 0;
        };
        std::uint64_t lastFound = 0;
        int stableScans = 0;
        for (int i = 0; i < attempts; ++i) {
            if (i) Sleep(static_cast<DWORD>(sleepMs));
            std::uint64_t found = 0;
            const bool present =
                (Remote(rva::LuaFindUI, nameObj, 0, 0, 0, found, 900) && found) ||
                (Remote(rva::LuaMainFindUI, nameObj, 0, 0, 0, found, 900) && found);
            if (!present) {
                lastFound = 0;
                stableScans = 0;
                continue;
            }
            if (found == lastFound) ++stableScans;
            else {
                lastFound = found;
                stableScans = 1;
            }
            if (stableScans >= stableRequired) {
                ui = found;
                freeHandle();
                detail = L"Lua UI đã ổn định " + std::to_wstring(stableScans) +
                         L" lần: " + Wide(uiName);
                return true;
            }
        }
        freeHandle();
        detail = L"Lua UI chưa ổn định: " + Wide(uiName);
        return false;
    }

    bool InvokeMainUiScriptNoArgs(const char* uiName, const char* functionName,
                                  std::wstring& detail) {
        std::uint64_t uiNameObj = 0, uiNameHandle = 0, functionObj = 0, functionHandle = 0;
        std::uint64_t ui = 0, executor = 0, args = 0, argsHandle = 0, result = 0;
        auto freeHandle = [&](std::uint64_t handle) {
            if (!handle) return;
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, handle, 0, 0, 0, ignored, 600);
        };
        if (!CreateManagedUtf8(uiName, uiNameObj, uiNameHandle)) {
            detail = L"Không tạo được tên Lua UI";
            return false;
        }
        bool found = Remote(rva::LuaFindUI, uiNameObj, 0, 0, 0, ui, 1000) && ui;
        if (!found)
            found = Remote(rva::LuaMainFindUI, uiNameObj, 0, 0, 0, ui, 1000) && ui;
        if (!found) {
            freeHandle(uiNameHandle);
            detail = L"Không tìm thấy Lua UI " + Wide(uiName);
            return false;
        }
        if (!CreateManagedUtf8(functionName, functionObj, functionHandle) ||
            !Remote(rva::MonoBehaviourExecutorGetInstance, 0, 0, 0, 0, executor, 900) || !executor ||
            !CreateEmptyUiArgArray(args, argsHandle)) {
            freeHandle(functionHandle);
            freeHandle(uiNameHandle);
            detail = L"Không chuẩn bị được Lua action " + Wide(functionName);
            return false;
        }
        const bool called = Remote5(rva::MonoBehaviourExecutorExecuteUiObject,
                                    executor, ui, functionObj, args, 0, result, 2400);
        freeHandle(argsHandle);
        freeHandle(functionHandle);
        freeHandle(uiNameHandle);
        detail = called ? L"Đã gọi Lua action " + Wide(uiName) + L"." + Wide(functionName)
                        : L"Lua action không phản hồi: " + Wide(uiName) + L"." + Wide(functionName);
        return called;
    }

    bool InvokeButton(std::uint64_t button) {
        std::uint64_t ignored = 0;
        return Remote(rva::UIButtonHandleClick, button, 0, 0, 0, ignored, 1800);
    }
    bool ClickInternalConfirm(std::wstring& detail) {
        std::uint64_t root = 0;
        std::vector<std::uint64_t> buttons;
        if (!FindMessageBox(root) || !root || !CollectTreeButtons(root, buttons)) {
            detail = L"Không đọc được cây nút của MessageBox";
            return false;
        }
        struct Candidate { std::uint64_t object; int score; std::wstring label; };
        std::vector<Candidate> candidates;
        const std::vector<std::wstring> positive{
            L"đồng ý", L"dong y", L"xác nhận", L"xac nhan", L"confirm",
            L"buttonok", L"btnok", L"buttonyes", L"btnyes", L"yes"};
        const std::vector<std::wstring> negative{
            L"hủy", L"huy", L"không", L"khong", L"cancel", L"buttonno",
            L"btnno", L"đóng", L"close", L"thoát", L"thoat"};
        for (const auto button : buttons) {
            std::wstring name, text;
            if (!InspectButton(button, name, text)) continue;
            const std::wstring label = Trim(name + L" " + text);
            const std::wstring normalized = Lower(label);
            int score = ContainsAny(normalized, positive) ? 20 : 0;
            if (ContainsAny(normalized, negative)) score -= 100;
            const std::wstring exactName = Trim(Lower(name));
            const std::wstring exactText = Trim(Lower(text));
            if (exactName == L"ok" || exactText == L"ok" || exactText == L"có" ||
                exactText == L"yes") score += 20;
            candidates.push_back({button, score, label});
        }
        if (candidates.empty()) {
            detail = L"MessageBox không có UIButton đang hoạt động";
            return false;
        }
        std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
            return a.score > b.score;
        });
        const bool unambiguous = candidates.size() == 1
            ? candidates.front().score >= 0
            : candidates.front().score > 0 && candidates.front().score > candidates[1].score;
        if (!unambiguous) {
            detail = L"Không xác định duy nhất nút đồng ý trong MessageBox";
            return false;
        }
        const bool sent = InvokeButton(candidates.front().object);
        detail = sent ? L"Đã gọi callback nội bộ: " + candidates.front().label
                      : L"Callback xác nhận map không phản hồi";
        return sent;
    }
    bool ReadDeathFlag(bool& dead) {
        dead = false;
        std::uint64_t player = 0, value = 0;
        if (!Remote(rva::LuaGetRoleData, 0, 0, 0, 0, player, 900) || !player ||
            !Remote(rva::LuaLeaderIsDeath, player, 0, 0, 0, value, 900)) return false;
        dead = (value & 0xFFu) != 0;
        return true;
    }
    bool ClickInternalDauThai(std::wstring& detail) {
        bool dead = false;
        if (!ReadDeathFlag(dead)) {
            detail = L"Không đọc được IsDeath ngay trước khi bấm Đầu thai";
            return false;
        }
        if (!dead) {
            detail = L"Không bấm Đầu thai vì IsDeath=false";
            return false;
        }
        ButtonInfo dauThai;
        if (!FindButton(ButtonRole::DauThai, dauThai, detail, true)) {
            detail = L"IsDeath=true nhưng chưa định vị được nút Đầu thai • " + detail;
            return false;
        }
        const bool sent = InvokeButton(dauThai.object);
        detail = sent
            ? L"Đã gọi UIButton.HandleClickEvent() của Đầu thai • " + dauThai.label
            : L"UIButton.HandleClickEvent() của Đầu thai không phản hồi • " + dauThai.label;
        return sent;
    }
    bool ClickInternalAutoFight(std::wstring& detail) {
        // Interface.unity3d: TopIcon.AutoTrainClick() is only a wrapper around
        // GUI.FindUI("AutoFight_Main"):StartAutoFight(C_AutoModel.Train).
        // Calling the wrapper from the remote worker previously proved unreliable, while opening
        // AutoFightGroup and replaying its buttons was correlated with client crashes.  Resolve the
        // persistent AutoFight_Main script directly and invoke the exact lower-level Lua action with
        // C_AutoModel.Train = 1.  No transient AUTO menu is created/destroyed.
        std::wstring callDetail;
        if (!InvokeUiScriptIntArg("AutoFight_Main", "StartAutoFight", 1, callDetail)) {
            detail = L"Không gọi được AutoFight_Main.StartAutoFight(Train) • " + callDetail;
            return false;
        }
        Sleep(500);
        for (int i = 0; i < 4; ++i) {
            std::uint64_t enableF1 = 1;
            if (Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enableF1, 700) &&
                (enableF1 & 0xFFu) == 0) {
                detail = L"AUTO → Đánh quái: StartAutoFight(Train) trực tiếp • EnableAutoF1=OFF";
                return true;
            }
            Sleep(250);
        }
        detail = L"Đã gọi StartAutoFight(Train) nhưng EnableAutoF1 chưa OFF; backoff, không mở menu AUTO";
        return false;
    }
    bool ClickInternalAutoStop(std::wstring& detail) {
        // Same lower-level action used by TopIcon.AutoStopClick().  C_AutoModel.None = 0.
        std::wstring callDetail;
        if (!InvokeUiScriptIntArg("AutoFight_Main", "StartAutoFight", 0, callDetail)) {
            detail = L"Không gọi được AutoFight_Main.StartAutoFight(None) • " + callDetail;
            return false;
        }
        Sleep(500);
        for (int i = 0; i < 4; ++i) {
            std::uint64_t enableF1 = 0;
            if (Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enableF1, 700) &&
                (enableF1 & 0xFFu) != 0) {
                detail = L"AUTO → Dừng: StartAutoFight(None) trực tiếp • EnableAutoF1=ON";
                return true;
            }
            Sleep(250);
        }
        detail = L"Đã gọi StartAutoFight(None) nhưng game chưa xác nhận Dừng";
        return false;
    }
    bool EnsureAutoStoppedForExclusive(std::wstring& detail) {
        std::uint64_t enableF1 = 1;
        if (!Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enableF1, 800)) {
            detail = L"Không đọc được trạng thái AUTO trước thao tác độc quyền";
            return false;
        }
        if ((enableF1 & 0xFFu) != 0) {
            detail = L"AUTO đã dừng";
            return true;
        }
        std::wstring stopDetail;
        if (!ClickInternalAutoStop(stopDetail)) {
            detail = L"AUTO còn chạy; không mở NPC để tránh race • " + stopDetail;
            return false;
        }
        Sleep(900);
        enableF1 = 0;
        if (!Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enableF1, 800) ||
            (enableF1 & 0xFFu) == 0) {
            detail = L"Đã yêu cầu Dừng nhưng AUTO chưa ổn định; hoãn thao tác NPC";
            return false;
        }
        detail = L"AUTO đã dừng ổn định trước thao tác NPC";
        return true;
    }
    bool ReadFreeBagSpace(int& freeSpace) {
        freeSpace = -1;
        std::uint64_t value = 0;
        if (!Remote(rva::LuaGetFreeBagSpace, 0, 0, 0, 0, value, 900)) return false;
        freeSpace = static_cast<std::int32_t>(value);
        return freeSpace >= 0 && freeSpace <= 10000;
    }
    bool ReadNearestNpcIdentity(std::int32_t& roleID, std::wstring& name,
                                std::wstring& detail) {
        roleID = 0;
        name.clear();
        std::uint64_t npc = 0, role = 0, namePointer = 0;
        if (!Remote(rva::LuaGetNearestNPC, 0, 0, 0, 0, npc, 1000) || !npc) {
            detail = L"GetNearestNPC() chưa trả về NPC";
            return false;
        }
        if (!Remote(rva::LuaMapObjectGetRoleID, npc, 0, 0, 0, role, 800) || role == 0) {
            detail = L"NPC gần nhất không có RoleID hợp lệ";
            return false;
        }
        if (Remote(rva::LuaMapObjectGetName, npc, 0, 0, 0, namePointer, 800) && namePointer)
            name = process_.ReadIl2CppString(namePointer);
        roleID = static_cast<std::int32_t>(role);
        return roleID > 0;
    }
    bool ReadNpcResIDFromSessionDictionary(std::uint64_t getter,
                                             std::int32_t roleID,
                                             std::int32_t& resID) {
        resID = 0;
        std::uint64_t dictionary = 0;
        if (!Remote(getter, 0, 0, 0, 0, dictionary, 900) || !dictionary) return false;
        std::uint64_t entries = 0;
        std::int32_t count = 0;
        if (!process_.Read(dictionary + off::DictionaryEntries, entries) || !entries ||
            !process_.Read(dictionary + off::DictionaryCount, count) || count < 0 || count > 8192)
            return false;
        for (std::int32_t i = 0; i < count; ++i) {
            const std::uint64_t entry = entries + off::ArrayData +
                static_cast<std::uint64_t>(i) * off::EntrySize;
            std::uint64_t npcObject = 0;
            if (!process_.Read(entry + off::EntryValue, npcObject) || !npcObject) continue;
            // GSprite.get_RoleID() is exactly [this+0x30] on this guarded game build.
            // Read it locally so saving an NPC stays instant even when many NPCs are loaded.
            std::int32_t objectRoleID = 0;
            if (!process_.Read(npcObject + off::GSpriteRoleID, objectRoleID) ||
                objectRoleID != roleID) continue;
            // GNPC/GMovingNPC.get_ResID() is [this+0xC0].  Verify the field through the
            // real getter once for the matched object before persisting it.
            std::int32_t rawResID = 0;
            if (!process_.Read(npcObject + off::GNPCResID, rawResID) || rawResID <= 0) return false;
            std::uint64_t getterValue = 0;
            if (!Remote(rva::GNPCGetResID, npcObject, 0, 0, 0, getterValue, 800) ||
                static_cast<std::int32_t>(getterValue) != rawResID) return false;
            resID = rawResID;
            return true;
        }
        return false;
    }
    bool ValidateNpcResIDCandidate(std::int32_t roleID, const std::wstring& expectedName,
                                   std::int32_t candidate, std::wstring& detail) {
        if (roleID <= 0 || candidate <= 0) return false;
        std::uint64_t npc = 0;
        if (!Remote(rva::LuaGetNearestNPCByResID, static_cast<std::uint32_t>(candidate),
                    0, 0, 0, npc, 1000) || !npc) return false;

        std::uint64_t returnedRole = 0, namePointer = 0;
        if (!Remote(rva::LuaMapObjectGetRoleID, npc, 0, 0, 0, returnedRole, 800) ||
            static_cast<std::int32_t>(returnedRole) != roleID) return false;

        std::wstring returnedName;
        if (Remote(rva::LuaMapObjectGetName, npc, 0, 0, 0, namePointer, 800) && namePointer)
            returnedName = process_.ReadIl2CppString(namePointer);
        const std::wstring expectedKey = CompactMatch(expectedName);
        const std::wstring returnedKey = CompactMatch(returnedName);
        if (!expectedKey.empty() && !returnedKey.empty() && expectedKey != returnedKey)
            return false;

        detail = L"Game xác minh RoleID " + std::to_wstring(roleID) +
                 L" ↔ ResID " + std::to_wstring(candidate);
        return true;
    }
    bool ResolveNpcResIDByRoleID(std::int32_t roleID, const std::wstring& name,
                                 std::int32_t& resID, std::wstring& detail) {
        resID = 0;
        if (roleID <= 0) {
            detail = L"RoleID NPC không hợp lệ";
            return false;
        }
        const std::int32_t builtin = BuiltinNpcResID(name);
        if (builtin > 0) {
            resID = builtin;
            detail = L"ResID " + std::to_wstring(resID) +
                     L" lấy từ database Config.unity3d của chính client";
            return true;
        }

        // Static NPC RoleID on this client uses the 1,000,000,000 namespace.  The two
        // live failures reported by the user are 1000000346 and 1000000378, while native
        // client code contains the same billion-base normalization.  Never trust the
        // arithmetic alone: ask GetNearestNPC(candidate) and require the returned RoleID
        // (and name, when available) to match before using/persisting the candidate.
        constexpr std::int32_t kStaticNpcRoleBase = 1000000000;
        constexpr std::int32_t kStaticNpcRoleSpan = 10000000;
        if (roleID >= kStaticNpcRoleBase &&
            roleID < kStaticNpcRoleBase + kStaticNpcRoleSpan) {
            const std::int32_t candidate = roleID - kStaticNpcRoleBase;
            std::wstring verifyDetail;
            if (ValidateNpcResIDCandidate(roleID, name, candidate, verifyDetail)) {
                resID = candidate;
                detail = verifyDetail + L" • không cần quét Dictionary nội bộ";
                return true;
            }
        }

        // Compatibility fallback for moving/special NPC types whose RoleID is not in the
        // static billion namespace.  This path is intentionally secondary because the
        // concrete Dictionary layout is more version-sensitive than the public Lua API.
        if (ReadNpcResIDFromSessionDictionary(rva::SessionGetNPCs, roleID, resID)) {
            detail = L"SessionData.NPCs: RoleID " + std::to_wstring(roleID) +
                     L" → ResID " + std::to_wstring(resID);
            return true;
        }
        if (ReadNpcResIDFromSessionDictionary(rva::SessionGetMovingNPCs, roleID, resID)) {
            detail = L"SessionData.MovingNPCs: RoleID " + std::to_wstring(roleID) +
                     L" → ResID " + std::to_wstring(resID);
            return true;
        }
        detail = L"Không ánh xạ được RoleID " + std::to_wstring(roleID) +
                 L" sang ResID bằng API game hoặc SessionData";
        return false;
    }
    bool ResolveSavedNpcResID(const SellNpc& saved, std::int32_t& resID,
                              std::wstring& detail) {
        resID = saved.resID;

        // Known static NPCs are sourced from the decrypted Config.unity3d. Validate the
        // nearby NPC by name, then use the config ID directly; RoleID is a runtime object ID
        // and is not arithmetically convertible to this ResID.
        const std::int32_t builtin = BuiltinNpcResID(saved.name);
        if (builtin > 0) {
            std::int32_t nearestRoleID = 0;
            std::wstring nearestName, nearestDetail;
            if (!ReadNearestNpcIdentity(nearestRoleID, nearestName, nearestDetail)) {
                detail = L"Chưa thấy NPC gần nhân vật để xác minh “" + saved.name + L"” • " + nearestDetail;
                return false;
            }
            if (CompactMatch(nearestName) != CompactMatch(saved.name)) {
                detail = L"NPC gần nhất “" + nearestName + L"” khác NPC cần mở “" + saved.name + L"”";
                return false;
            }
            resID = builtin;
            detail = L"Đã xác minh tên NPC gần nhất • dùng ResID " + std::to_wstring(resID) +
                     L" từ Config.unity3d";
            return true;
        }

        // Prefer a previously saved ResID only when the game's own nearest-NPC query
        // proves that it still maps back to the saved RoleID/name.
        if (resID > 0 && saved.roleID > 0) {
            std::wstring verifyDetail;
            if (ValidateNpcResIDCandidate(saved.roleID, saved.name, resID, verifyDetail)) {
                detail = verifyDetail;
                return true;
            }
            resID = 0; // stale/legacy entry: re-resolve from the live nearby NPC below.
        }

        std::int32_t nearestRoleID = 0;
        std::wstring nearestName, nearestDetail;
        if (!ReadNearestNpcIdentity(nearestRoleID, nearestName, nearestDetail)) {
            detail = L"Chưa xác minh được NPC đã lưu; hãy đứng sát NPC • " + nearestDetail;
            return false;
        }
        const std::wstring savedKey = CompactMatch(saved.name);
        const std::wstring nearestKey = CompactMatch(nearestName);
        const bool sameName = !savedKey.empty() && !nearestKey.empty() && savedKey == nearestKey;
        const bool sameRole = saved.roleID > 0 && saved.roleID == nearestRoleID;
        if (!sameName && !sameRole) {
            detail = L"NPC gần nhất “" +
                     (nearestName.empty() ? L"không rõ tên" : nearestName) +
                     L"” khác NPC đã lưu “" + saved.name + L"”";
            return false;
        }

        if (!ResolveNpcResIDByRoleID(nearestRoleID, nearestName, resID, detail))
            return false;

        std::wstring verifyDetail;
        if (!ValidateNpcResIDCandidate(nearestRoleID, nearestName, resID, verifyDetail)) {
            detail = L"Đã suy ra ResID " + std::to_wstring(resID) +
                     L" nhưng game không xác minh NPC tương ứng; không ClickNPC mù";
            return false;
        }
        detail = verifyDetail;
        return true;
    }

    bool OpenNpc(const SellNpc& saved, std::wstring& detail) {
        std::int32_t resID = 0;
        if (!ResolveSavedNpcResID(saved, resID, detail)) return false;
        std::uint64_t ignored = 0;
        if (!Remote(rva::LuaClickNPC, static_cast<std::uint32_t>(resID),
                    0, 0, 0, ignored, 1600)) {
            detail = L"ClickNPC(ResID=" + std::to_wstring(resID) + L") không phản hồi";
            return false;
        }
        detail = L"Đã gọi ClickNPC bằng ResID " + std::to_wstring(resID) + L" • " + saved.name;
        return true;
    }
    bool WaitButton(ButtonRole role, ButtonInfo& button, std::wstring& reason,
                    int attempts = 10, int sleepMs = 160) {
        for (int i = 0; i < attempts; ++i) {
            if (i) Sleep(static_cast<DWORD>(sleepMs));
            if (FindButton(role, button, reason, i == attempts - 1)) return true;
        }
        return false;
    }
    bool WaitLuaNoArgs(const char* uiName, const char* functionName,
                       std::wstring& detail, int attempts = 30, int sleepMs = 35) {
        std::wstring last;
        for (int i = 0; i < attempts; ++i) {
            if (i) Sleep(static_cast<DWORD>(sleepMs));
            if (InvokeMainUiScriptNoArgs(uiName, functionName, last)) {
                detail = last;
                return true;
            }
        }
        detail = last.empty() ? L"Lua UI/action chưa sẵn sàng" : last;
        return false;
    }
    bool OpenSellUi(std::wstring& detail) {
        if (!OpenNpc(sellNpc_, detail)) return false;

        // ClickNPC changes target/task state and asks the server to create GameDialog.  The crash
        // reports happen shortly after this edge, so keep the remote worker completely quiet for
        // a full settle window before touching any UI object.
        Sleep(1200);

        std::uint64_t gameDialog = 0;
        std::wstring uiDetail;
        bool dialogReady = WaitForLuaUi("GameDialog", gameDialog, uiDetail, 16, 150, 2);
        if (!dialogReady) {
            // Some server scripts can jump directly into NPCShop.  Accept only a shop root that
            // survives two consecutive reads; never globally scan UIObject.instances here.
            std::uint64_t directShop = 0;
            std::wstring shopProbe;
            if (!WaitForLuaUi("NPCShop", directShop, shopProbe, 8, 150, 2)) {
                detail = L"ClickNPC xong nhưng GameDialog/NPCShop chưa ổn định • " + uiDetail;
                return false;
            }
        } else {
            // Resolve only controls below the stable GameDialog root.  The old global UI scan
            // could make hundreds of Unity getter calls while unrelated panels were changing.
            Sleep(300);
            ButtonInfo shopEntry;
            std::wstring reason;
            bool foundEntry = FindButtonInTree(gameDialog, ButtonRole::ShopEntry,
                                               shopEntry, reason);
            if (!foundEntry) {
                Sleep(350);
                std::uint64_t freshDialog = 0;
                if (WaitForLuaUi("GameDialog", freshDialog, uiDetail, 4, 150, 2)) {
                    gameDialog = freshDialog;
                    foundEntry = FindButtonInTree(gameDialog, ButtonRole::ShopEntry,
                                                  shopEntry, reason);
                }
            }
            if (!foundEntry) {
                detail = L"GameDialog ổn định nhưng không xác định được ‘Mua thú cưỡi’ • " + reason;
                return false;
            }
            std::wstring luaDetail;
            if (!InvokeUiScriptOneArg("GameDialog", "FunctionButtonClicked",
                                      shopEntry.object, luaDetail)) {
                detail = L"GameDialog.FunctionButtonClicked(Mua thú cưỡi) không phản hồi • " +
                         luaDetail;
                return false;
            }

            // FunctionButtonClicked destroys GameDialog and builds NPCShop.  Do not inspect the
            // tree during that destruction/rebuild window.
            Sleep(900);
        }

        std::uint64_t shop = 0;
        if (!WaitForLuaUi("NPCShop", shop, uiDetail, 16, 150, 2)) {
            detail = L"Đã chọn shop nhưng NPCShop chưa ổn định • " + uiDetail;
            return false;
        }
        Sleep(350);

        // Keep the exact game tab action, but search only inside this stable NPCShop root.
        ButtonInfo sellTab;
        std::wstring reason;
        bool foundSell = FindToggleInTree(shop, ButtonRole::SellTab, sellTab, reason);
        if (!foundSell) {
            Sleep(350);
            std::uint64_t freshShop = 0;
            if (WaitForLuaUi("NPCShop", freshShop, uiDetail, 4, 150, 2)) {
                shop = freshShop;
                foundSell = FindToggleInTree(shop, ButtonRole::SellTab, sellTab, reason);
            }
        }
        if (!foundSell) {
            detail = L"NPCShop ổn định nhưng chưa định vị được ‘Bán vật phẩm’ • " + reason;
            return false;
        }
        std::wstring sellTabDetail;
        if (!InvokeUiScriptOneArg("NPCShop", "ToggleTabHeaderSelected",
                                  sellTab.object, sellTabDetail)) {
            detail = L"NPCShop.ToggleTabHeaderSelected không phản hồi • " + sellTabDetail;
            return false;
        }

        // Sell Engine v2 needs the actual sell-tab script but does not need QuickSell or the
        // visual Equipment filter.  Wait until that script itself is stable before sending data.
        Sleep(650);
        std::uint64_t sellScript = 0;
        if (!WaitForLuaUi("NPCShop_SellItemTab", sellScript, uiDetail, 10, 150, 2)) {
            detail = L"Đã vào Bán vật phẩm nhưng NPCShop_SellItemTab chưa ổn định • " + uiDetail;
            return false;
        }
        detail = L"NPCShop ổn định → Bán vật phẩm • Sell Engine data sẵn sàng";
        return true;
    }
    struct BagSellItem {
        std::uint64_t object = 0;
        std::int32_t dbID = 0;
        std::int32_t itemID = 0;
        std::int32_t position = -1;
        bool sellable = false;
    };
    bool ReadManagedObjectCollection(std::uint64_t collection,
                                     std::vector<std::uint64_t>& objects) {
        objects.clear();
        if (!collection) return false;

        // GetItemsAtSite() on this build normally returns a managed List<...>.
        // Validate both the List and raw-array layouts so a minor binding change does not turn
        // into an unsafe pointer walk.
        std::uint64_t array = 0, arrayLength = 0;
        std::int32_t count = -1;
        if (process_.Read(collection + off::ListItems, array) && array &&
            process_.Read(collection + off::ListSize, count) && count >= 0 && count <= 512 &&
            process_.Read(array + off::ArrayLength, arrayLength) &&
            arrayLength >= static_cast<std::uint64_t>(count) && arrayLength <= 4096) {
            objects.reserve(static_cast<std::size_t>(count));
            for (int i = 0; i < count; ++i) {
                std::uint64_t object = 0;
                if (process_.Read(array + off::ArrayData + static_cast<std::uint64_t>(i) * 8,
                                  object) && object)
                    objects.push_back(object);
            }
            return true;
        }

        // Defensive raw Il2CppArray fallback.
        arrayLength = 0;
        if (process_.Read(collection + off::ArrayLength, arrayLength) && arrayLength <= 512) {
            objects.reserve(static_cast<std::size_t>(arrayLength));
            for (std::uint64_t i = 0; i < arrayLength; ++i) {
                std::uint64_t object = 0;
                if (process_.Read(collection + off::ArrayData + i * 8, object) && object)
                    objects.push_back(object);
            }
            return true;
        }
        return false;
    }
    bool ReadBagEquipmentItems(std::vector<BagSellItem>& items, std::wstring& detail) {
        items.clear();
        std::uint64_t collection = 0;
        constexpr std::uint32_t kBagSite = 10; // C_ItemSite.Bag recovered from Interface.unity3d.
        if (!Remote(rva::LuaGetItemsAtSite, kBagSite, 0, 0, 0, collection, 1400) || !collection) {
            detail = L"GetItemsAtSite(Bag) không trả về danh sách";
            return false;
        }
        // GetItemsAtSite may return a managed list created for this call. Hold a strong GC handle
        // for the entire scan so the collection and its item references cannot disappear between
        // the remote return and our cross-process reads.
        std::uint64_t collectionHandle = 0;
        if (!il2cppGcHandleNew_ ||
            !RemoteAbsolute(il2cppGcHandleNew_, collection, 0, 0, 0,
                            collectionHandle, 900) || !collectionHandle) {
            detail = L"Không giữ được snapshot Bag bằng GC handle";
            return false;
        }
        auto freeCollection = [&]() {
            if (!collectionHandle) return;
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, collectionHandle, 0, 0, 0, ignored, 600);
            collectionHandle = 0;
        };

        std::vector<std::uint64_t> objects;
        if (!ReadManagedObjectCollection(collection, objects)) {
            freeCollection();
            detail = L"Không đọc được cấu trúc danh sách vật phẩm trong Bag";
            return false;
        }

        for (const std::uint64_t object : objects) {
            // LuaItemData's three getters used here are trivial field reads in this exact build:
            // ID +0x10, ItemID +0x14, Position +0x1C.  Read those immutable snapshot fields
            // directly instead of issuing three remote managed calls per item on every rescan.
            // This keeps the sell loop mostly read-only while the server/UI may be rebuilding.
            std::int32_t dbID = 0, itemID = 0, position = -1;
            if (!process_.Read(object + 0x10, dbID) ||
                !process_.Read(object + 0x14, itemID) ||
                !process_.Read(object + 0x1C, position))
                continue;
            if (dbID <= 0 || itemID <= 0 || position < 0 || position >= 1000) continue;

            bool isEquip = false;
            const auto equipIt = itemEquipCache_.find(itemID);
            if (equipIt != itemEquipCache_.end()) {
                isEquip = equipIt->second;
            } else {
                std::uint64_t typePointer = 0;
                if (!Remote(rva::LuaGetItemType, static_cast<std::uint32_t>(itemID),
                            0, 0, 0, typePointer, 800) || !typePointer)
                    continue;
                isEquip = CompactMatch(process_.ReadIl2CppString(typePointer)) == L"equip";
                itemEquipCache_[itemID] = isEquip;
            }
            if (!isEquip) continue;

            bool sellable = false;
            const auto sellIt = itemSellableCache_.find(itemID);
            if (sellIt != itemSellableCache_.end()) {
                sellable = sellIt->second;
            } else {
                std::uint64_t sellableValue = 0;
                sellable = Remote(rva::LuaIsItemSellable,
                                  static_cast<std::uint32_t>(itemID),
                                  0, 0, 0, sellableValue, 800) &&
                           (sellableValue & 0xFFu) != 0 &&
                           !(itemID >= 40000000 && itemID < 50000000);
                itemSellableCache_[itemID] = sellable;
            }
            items.push_back({object, dbID, itemID, position, sellable});
        }
        std::sort(items.begin(), items.end(), [](const BagSellItem& a, const BagSellItem& b) {
            if (a.position != b.position) return a.position < b.position;
            return a.dbID < b.dbID;
        });
        freeCollection();
        detail = L"Đọc data Bag: " + std::to_wstring(items.size()) + L" trang bị";
        return true;
    }
    bool CreateSingleUiArgArray(std::uint64_t object, std::uint64_t& array,
                                std::uint64_t& handle) {
        array = handle = 0;
        if (!object || !systemObjectClass_ || !il2cppArrayNew_ ||
            !RemoteAbsolute(il2cppArrayNew_, systemObjectClass_, 1, 0, 0, array, 1200) || !array ||
            !RemoteAbsolute(il2cppGcHandleNew_, array, 0, 0, 0, handle, 900) || !handle)
            return false;
        if (!process_.WriteBytes(array + off::ArrayData, &object, sizeof(object))) {
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, handle, 0, 0, 0, ignored, 500);
            array = handle = 0;
            return false;
        }
        return true;
    }
    bool InvokeUiScriptOneArg(const char* uiName, const char* functionName,
                              std::uint64_t argument, std::wstring& detail) {
        std::uint64_t uiNameObj = 0, uiNameHandle = 0, functionObj = 0, functionHandle = 0;
        std::uint64_t ui = 0, executor = 0, args = 0, argsHandle = 0, result = 0;
        auto freeHandle = [&](std::uint64_t handle) {
            if (!handle) return;
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, handle, 0, 0, 0, ignored, 600);
        };
        if (!argument || !CreateManagedUtf8(uiName, uiNameObj, uiNameHandle)) {
            detail = L"Không tạo được tên Lua UI " + Wide(uiName);
            return false;
        }
        bool found = Remote(rva::LuaFindUI, uiNameObj, 0, 0, 0, ui, 1000) && ui;
        if (!found)
            found = Remote(rva::LuaMainFindUI, uiNameObj, 0, 0, 0, ui, 1000) && ui;
        if (!found) {
            freeHandle(uiNameHandle);
            detail = L"Không tìm thấy Lua UI " + Wide(uiName);
            return false;
        }
        if (!CreateManagedUtf8(functionName, functionObj, functionHandle) ||
            !Remote(rva::MonoBehaviourExecutorGetInstance, 0, 0, 0, 0, executor, 900) || !executor ||
            !CreateSingleUiArgArray(argument, args, argsHandle)) {
            freeHandle(functionHandle);
            freeHandle(uiNameHandle);
            detail = L"Không chuẩn bị được Lua action " + Wide(uiName) + L"." + Wide(functionName);
            return false;
        }
        const bool called = Remote5(rva::MonoBehaviourExecutorExecuteUiObject,
                                    executor, ui, functionObj, args, 0, result, 2200);
        freeHandle(argsHandle);
        freeHandle(functionHandle);
        freeHandle(uiNameHandle);
        detail = called ? L"Đã gọi Lua action " + Wide(uiName) + L"." + Wide(functionName)
                        : L"Lua action không phản hồi: " + Wide(uiName) + L"." + Wide(functionName);
        return called;
    }
    bool WaitItemRemoved(std::int32_t dbID, std::wstring& detail) {
        if (dbID <= 0) return false;
        // Do not guess a fixed server delay. The Lua BagItemsGrid itself waits for
        // RemoveItem/UpdateItemsList; mirror that by polling the authoritative item API.
        constexpr int kPolls = 24;
        constexpr DWORD kPollMs = 125;
        for (int i = 0; i < kPolls; ++i) {
            std::uint64_t item = 0;
            if (!Remote(rva::LuaGetItemData, static_cast<std::uint32_t>(dbID),
                        0, 0, 0, item, 900)) {
                detail = L"Mất phản hồi GetItemData khi chờ server xác nhận bán";
                return false;
            }
            if (!item) {
                detail = L"Server đã xóa DBID " + std::to_wstring(dbID);
                return true;
            }
            Sleep(kPollMs);
        }
        detail = L"Hết thời gian chờ server xóa DBID " + std::to_wstring(dbID);
        return false;
    }
    void CloseSellUiAfterQuiet(std::wstring& detail) {
        // Never destroy the shop in the same instant as the final sell response.
        Sleep(800);
        std::wstring closeDetail;
        if (InvokeMainUiScriptNoArgs("NPCShop", "ButtonCloseClicked", closeDetail)) {
            detail += L" • đóng NPCShop bằng Lua action";
            Sleep(500);
            return;
        }
        if (!closeDetail.empty())
            detail += L" • chưa đóng được NPCShop bằng Lua: " + closeDetail;
    }
    bool TrySellAtNpc(int& freeAfter, std::wstring& detail) {
        freeAfter = -1;
        std::wstring autoStopDetail;
        if (!EnsureAutoStoppedForExclusive(autoStopDetail)) {
            detail = L"Không bắt đầu bán đồ • " + autoStopDetail;
            return false;
        }
        int freeBefore = -1;
        ReadFreeBagSpace(freeBefore);
        if (!OpenSellUi(detail)) return false;

        // Sell Engine v2: the recovered NPCShop_SellItemTab Lua proves that clicking an ItemBox
        // only ends in RequestSellItem(dbItemData). Avoid 90 Unity UIButton callbacks entirely.
        // Read authoritative bag data, protect the first equipment by Position, sell one item,
        // wait for the server to remove that DBID, then rescan. This prevents racing
        // BagItemsGrid:Clear()/DoFilter()/RebuildLayout() on Unity's main thread.
        constexpr int kMaxSellRequests = 90;
        std::map<std::int32_t, int> failures;
        std::set<std::int32_t> skipped;
        int requests = 0;
        int confirmed = 0;
        int skippedCount = 0;

        while (requests < kMaxSellRequests) {
            std::vector<BagSellItem> items;
            std::wstring scanDetail;
            if (!ReadBagEquipmentItems(items, scanDetail)) {
                detail = L"Sell Engine v2: " + scanDetail;
                CloseSellUiAfterQuiet(detail);
                return false;
            }
            if (items.size() <= 1) {
                detail = L"Sell Engine v2 xong: chỉ còn " + std::to_wstring(items.size()) +
                         L" trang bị (ô đầu luôn được giữ)";
                break;
            }

            // items[0] is the protected visual slot #1. After each acknowledged sale the
            // bag is rescanned, so the next item naturally becomes visual slot #2.
            const BagSellItem* candidate = nullptr;
            for (std::size_t i = 1; i < items.size(); ++i) {
                if (skipped.find(items[i].dbID) != skipped.end()) continue;
                candidate = &items[i];
                break;
            }
            if (!candidate) {
                detail = L"Sell Engine v2: không còn trang bị hợp lệ ngoài ô đầu";
                break;
            }
            if (!candidate->sellable) {
                skipped.insert(candidate->dbID);
                ++skippedCount;
                continue;
            }

            // Never carry a LuaItemData pointer from the scan into an action.  Resolve the DBID
            // again immediately before sending; if the server/UI changed the inventory after the
            // scan, simply rescan instead of invoking Lua with a stale object.
            std::uint64_t freshItem = 0;
            if (!Remote(rva::LuaGetItemData, static_cast<std::uint32_t>(candidate->dbID),
                        0, 0, 0, freshItem, 900) || !freshItem) {
                Sleep(150);
                continue;
            }

            std::wstring sendDetail;
            ++requests;
            if (!InvokeUiScriptOneArg("NPCShop_SellItemTab", "RequestSellItem",
                                      freshItem, sendDetail)) {
                const int count = ++failures[candidate->dbID];
                if (count >= 3) {
                    skipped.insert(candidate->dbID);
                    ++skippedCount;
                }
                // A failed Lua invoke must not be followed by another action immediately.
                Sleep(800);
                continue;
            }

            std::wstring ackDetail;
            if (WaitItemRemoved(candidate->dbID, ackDetail)) {
                ++confirmed;
                failures.erase(candidate->dbID);
                // Keep the user's one-second sell cadence.  ACK already proves the item is gone;
                // this additional quiet window lets BagItemsGrid finish Clear/DoFilter/RebuildLayout
                // before the next RequestSellItem.
                Sleep(900);
                continue;
            }

            const int count = ++failures[candidate->dbID];
            if (count >= 3) {
                skipped.insert(candidate->dbID);
                ++skippedCount;
            }
            // If an ACK was missed, give any late response a full quiet window before rescan/retry.
            Sleep(1000);
        }

        int value = -1;
        if (ReadFreeBagSpace(value)) freeAfter = value;
        else freeAfter = freeBefore;

        if (detail.empty() || detail.find(L"Sell Engine v2") == std::wstring::npos)
            detail = L"Sell Engine v2 hoàn tất";
        detail += L" • request " + std::to_wstring(requests) + L"/90 • server xác nhận " +
                  std::to_wstring(confirmed) + L" • bỏ qua " + std::to_wstring(skippedCount);
        if (freeBefore >= 0 && freeAfter >= 0)
            detail += L" • tay nải " + std::to_wstring(freeBefore) + L" → " +
                      std::to_wstring(freeAfter) + L" ô trống";
        CloseSellUiAfterQuiet(detail);
        return true;
    }

    void UpdateTransitionState(bool mapReady, bool waitingChangeMap,
                               bool messageBoxVisible) {
        std::lock_guard<std::mutex> guard(stateLock_);
        state_.connected = true;
        state_.running = running_.load();
        state_.mapReady = mapReady;
        state_.waitingChangeMap = waitingChangeMap;
        state_.messageBoxVisible = messageBoxVisible;
        state_.moving = false;
        state_.autoPathing = false;
        ++state_.sequence;
    }
    bool RefreshLive(std::wstring& error) {
        std::uint64_t mapReadyValue = 0, waitingChangeMapValue = 0;
        bool messageBoxVisible = false;
        if (!Remote(rva::LuaIsMapReady, 0, 0, 0, 0, mapReadyValue) ||
            !Remote(rva::SessionWaitingChangeMap, 0, 0, 0, 0,
                    waitingChangeMapValue)) {
            error = L"Mất phản hồi trạng thái chuyển map";
            return false;
        }
        const bool mapReady = (mapReadyValue & 0xFFu) != 0;
        const bool waitingChangeMap = (waitingChangeMapValue & 0xFFu) != 0;
        if (!mapReady || waitingChangeMap) {
            UpdateTransitionState(mapReady, waitingChangeMap, false);
            return true;
        }
        if (!ReadMessageBoxVisible(messageBoxVisible)) {
            error = L"Mất phản hồi bộ nhận diện hộp xác nhận";
            return false;
        }
        std::uint64_t luaPlayer = 0;
        if (!Remote(rva::LuaGetRoleData, 0, 0, 0, 0, luaPlayer) || !luaPlayer) {
            error = L"Hãy đăng nhập nhân vật và vào bản đồ";
            return false;
        }
        std::uint64_t roleData = 0;
        std::int32_t map = 0, x = 0, y = 0;
        if (!process_.Read(luaPlayer + off::LuaPlayerRoleData, roleData) || !roleData ||
            !process_.Read(roleData + off::RoleMapID, map) ||
            !process_.Read(roleData + off::RolePosX, x) ||
            !process_.Read(roleData + off::RolePosY, y) || map <= 0) {
            error = L"Chưa đọc được MapID/X/Y realtime";
            return false;
        }
        std::uint64_t roleIDValue = 0, namePointer = 0;
        if (!Remote(rva::LuaPlayerGetRoleID, luaPlayer, 0, 0, 0, roleIDValue, 900) ||
            !Remote(rva::LuaPlayerGetName, luaPlayer, 0, 0, 0, namePointer, 900)) {
            error = L"Mất phản hồi danh tính nhân vật";
            return false;
        }
        const std::wstring characterName = process_.ReadIl2CppString(namePointer);
        std::uint64_t riding = 0, autoFight = 0, moving = 0, pathing = 0, dead = 0;
        if (!Remote(rva::LuaLeaderIsDeath, luaPlayer, 0, 0, 0, dead) ||
            !Remote(rva::LuaIsRiding, 0, 0, 0, 0, riding) ||
            !Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, autoFight) ||
            !Remote(rva::LuaIsMoving, 0, 0, 0, 0, moving) ||
            !Remote(rva::AutoPathIsRunning, autoPathManager_, 0, 0, 0, pathing)) {
            error = L"Mất phản hồi trạng thái nhân vật";
            return false;
        }
        std::lock_guard<std::mutex> guard(stateLock_);
        state_.connected = true;
        state_.running = running_.load();
        state_.mapID = map;
        state_.x = x;
        state_.y = y;
        state_.roleID = static_cast<std::int32_t>(roleIDValue);
        state_.characterName = characterName;
        state_.riding = (riding & 0xFFu) != 0;
        // Interface Lua semantics: StartAutoFight(Train) sets EnableAutoF1=false;
        // StopAllCurrentTask restores it to true. Expose autoFight as the inferred combat state.
        state_.autoFight = (autoFight & 0xFFu) == 0;
        state_.moving = (moving & 0xFFu) != 0;
        state_.autoPathing = (pathing & 0xFFu) != 0;
        state_.dead = (dead & 0xFFu) != 0;
        state_.mapReady = mapReady;
        state_.waitingChangeMap = waitingChangeMap;
        state_.messageBoxVisible = messageBoxVisible;
        ++state_.sequence;
        return true;
    }
    void UpdateFreeBagSpace(int freeSpace) {
        std::lock_guard<std::mutex> guard(stateLock_);
        state_.freeBagSpace = freeSpace;
        ++state_.sequence;
    }
    void UpdateLive(bool connected, const std::wstring& phase, const std::wstring& detail) {
        std::lock_guard<std::mutex> guard(stateLock_);
        state_.connected = connected;
        state_.running = running_.load();
        state_.phase = phase;
        state_.detail = detail;
        ++state_.sequence;
    }
    bool ToggleRide(bool) {
        std::uint64_t slot = 0;
        std::uint64_t ignored = 0;
        if (!Remote(rva::LuaCurrentMountSlot, 0, 0, 0, 0, slot)) return false;
        return Remote(rva::LuaToggleRide, static_cast<std::uint32_t>(slot), 0, 0, 0, ignored);
    }
    bool ReadSelectedTargetRoleID(std::int32_t& roleID) {
        roleID = 0;
        std::uint64_t selected = 0, value = 0;
        if (!Remote(rva::LuaGetSelectedTarget, 0, 0, 0, 0, selected, 800)) return false;
        if (!selected) return true;
        if (!Remote(rva::SelectedTargetGetRoleID, selected, 0, 0, 0, value, 800)) return false;
        roleID = static_cast<std::int32_t>(value);
        return true;
    }
    bool EnsureCtrlTabTarget(std::int32_t& roleID, std::wstring& detail) {
        roleID = 0;
        bool selectedRead = ReadSelectedTargetRoleID(roleID);
        bool targetDead = false;
        if (selectedRead && roleID > 0) {
            std::uint64_t dead = 0;
            if (Remote(rva::LuaIsSelectTargetDie, static_cast<std::uint32_t>(roleID),
                       0, 0, 0, dead, 800))
                targetDead = (dead & 0xFFu) != 0;
        }

        // Ctrl+Tab mode is deliberately separate from AUTO → Đánh quái.
        // If there is no live target, use the game's real Ctrl+Tab target-selection command.
        // Do not silently switch to internal target-selection APIs; that would blend two
        // different combat modes and can retarget while the mount/navigation state owns the PID.
        if (!selectedRead || roleID <= 0 || targetDead) {
            PressGameKey(game_.window, VK_TAB, true);
            Sleep(180);
            if (!ReadSelectedTargetRoleID(roleID) || roleID <= 0) {
                detail = L"Ctrl+Tab chưa chọn được mục tiêu";
                return false;
            }
            std::uint64_t dead = 0;
            if (Remote(rva::LuaIsSelectTargetDie, static_cast<std::uint32_t>(roleID),
                       0, 0, 0, dead, 800) && (dead & 0xFFu) != 0) {
                detail = L"Ctrl+Tab đang trỏ vào mục tiêu đã chết";
                return false;
            }
        }
        return true;
    }
    bool TriggerCtrlTabSkill(std::wstring& detail) {
        if (config_.skillID <= 0) {
            detail = L"Chưa chọn skill";
            return false;
        }
        std::int32_t targetRoleID = 0;
        if (!EnsureCtrlTabTarget(targetRoleID, detail)) return false;
        std::uint64_t value = 0;
        if (!Remote(rva::LuaHasSkill, static_cast<std::uint32_t>(config_.skillID),
                    0, 0, 0, value, 900) || (value & 0xFFu) == 0) {
            detail = L"Game báo nhân vật không có skill đã chọn";
            return false;
        }
        if (!Remote(rva::LuaCanUseSkill, 0, 0, 0, 0, value, 900) ||
            (value & 0xFFu) == 0) {
            detail = L"Nhân vật chưa thể dùng skill";
            return false;
        }
        if (!Remote(rva::LuaIsSkillCooldown, static_cast<std::uint32_t>(config_.skillID),
                    0, 0, 0, value, 900)) {
            detail = L"Không đọc được hồi chiêu";
            return false;
        }
        if ((value & 0xFFu) != 0) {
            detail = L"Skill đang hồi";
            return false;
        }
        if (!Remote(rva::LuaCheckSkillCondition, static_cast<std::uint32_t>(config_.skillID),
                    0, 0, 0, value, 900) || (value & 0xFFu) == 0) {
            detail = L"Chưa đủ điều kiện dùng skill";
            return false;
        }
        std::uint64_t ignored = 0;
        const bool sent = Remote(rva::LuaRequestUsingSkillWithTarget,
                                 static_cast<std::uint32_t>(config_.skillID),
                                 static_cast<std::uint32_t>(targetRoleID),
                                 0, 0, ignored, 1200);
        detail = sent ? L"Skill " + std::to_wstring(config_.skillID) +
                            L" → Role " + std::to_wstring(targetRoleID)
                      : L"Gửi skill thất bại";
        return sent;
    }
    struct BuffSnapshot {
        std::map<int, std::uint64_t> durationById;
    };
    bool ReadBuffObject(std::uint64_t object, BuffSnapshot& out) {
        if (!object) return false;
        std::uint64_t idValue = 0, durationValue = 0;
        if (!Remote(rva::LuaBuffGetBuffID, object, 0, 0, 0, idValue, 700)) return false;
        const int id = static_cast<std::int32_t>(idValue);
        if (id <= 0 || id > 100000000) return false;
        Remote(rva::LuaBuffGetDurationTick, object, 0, 0, 0, durationValue, 700);
        out.durationById[id] = durationValue;
        return true;
    }
    bool ReadBuffSnapshot(BuffSnapshot& out) {
        out.durationById.clear();
        std::uint64_t collection = 0;
        if (!Remote(rva::LuaGetBuffs, 0, 0, 0, 0, collection, 1400) || !collection) return false;

        // The API return type is a managed collection. Support both common layouts used by
        // this build (List<LuaBuffData> and Dictionary<*, LuaBuffData>) and validate every
        // count/pointer before reading; never assume a layout after a failed sanity check.
        std::uint64_t array = 0;
        std::int32_t count = -1;
        bool layoutValid = false;
        if (process_.Read(collection + off::ListItems, array) && array &&
            process_.Read(collection + off::ListSize, count) && count >= 0 && count <= 512) {
            std::uint64_t arrayLength = 0;
            if (process_.Read(array + off::ArrayLength, arrayLength) &&
                arrayLength >= static_cast<std::uint64_t>(count) && arrayLength <= 4096) {
                layoutValid = true;
                for (int i = 0; i < count; ++i) {
                    std::uint64_t value = 0;
                    if (process_.Read(array + off::ArrayData + static_cast<std::uint64_t>(i) * 8,
                                      value) && value) ReadBuffObject(value, out);
                }
            }
        }
        if (layoutValid) return true;

        array = 0; count = -1;
        if (process_.Read(collection + off::DictionaryEntries, array) && array &&
            process_.Read(collection + off::DictionaryCount, count) && count >= 0 && count <= 512) {
            layoutValid = true;
            for (int i = 0; i < count; ++i) {
                const std::uint64_t entry = array + off::ArrayData +
                    static_cast<std::uint64_t>(i) * off::EntrySize;
                std::uint64_t value = 0;
                if (process_.Read(entry + off::EntryValue, value) && value) ReadBuffObject(value, out);
            }
        }
        return layoutValid;
    }
    bool HasMappedBuff(int buffID) {
        if (buffID <= 0) return false;
        std::uint64_t value = 0;
        return Remote(rva::LuaHasBuff, static_cast<std::uint32_t>(buffID), 0, 0, 0,
                      value, 900) && (value & 0xFFu) != 0;
    }
    bool TryApplyAndVerifyBuff(int skillID, std::wstring& detail) {
        if (skillID <= 0) return false;
        const auto mapped = config_.buffMap.find(skillID);
        if (mapped != config_.buffMap.end() && HasMappedBuff(mapped->second)) {
            detail = L"Skill " + std::to_wstring(skillID) + L" đã có buff " +
                     std::to_wstring(mapped->second);
            return true;
        }
        BuffSnapshot before, after;
        const bool haveBefore = ReadBuffSnapshot(before);
        std::uint64_t ignored = 0;
        if (!Remote(rva::LuaRequestUsingSkill, static_cast<std::uint32_t>(skillID),
                    0, 0, 0, ignored, 1800)) {
            detail = L"RequestUsingSkill(" + std::to_wstring(skillID) + L") thất bại";
            return false;
        }
        Sleep(650);
        const bool haveAfter = ReadBuffSnapshot(after);
        if (mapped != config_.buffMap.end() && HasMappedBuff(mapped->second)) {
            detail = L"Buff " + std::to_wstring(mapped->second) + L" đã xuất hiện";
            return true;
        }
        if (haveBefore && haveAfter) {
            int detected = 0;
            for (const auto& [buffID, duration] : after.durationById) {
                const auto old = before.durationById.find(buffID);
                if (old == before.durationById.end() || duration > old->second) {
                    if (detected != 0 && detected != buffID) {
                        detected = -1;  // ambiguous: more than one buff changed.
                        break;
                    }
                    detected = buffID;
                }
            }
            if (detected > 0) {
                config_.buffMap[skillID] = detected;
                detail = L"Đã xác minh skill " + std::to_wstring(skillID) + L" → buff " +
                         std::to_wstring(detected);
                return true;
            }
        }
        detail = L"Skill " + std::to_wstring(skillID) +
                 L" chưa tạo/refresh buff có thể xác minh";
        return false;
    }

    bool TryTreatmentAtNpc(std::wstring& detail) {
        std::wstring autoStopDetail;
        if (!EnsureAutoStoppedForExclusive(autoStopDetail)) {
            detail = L"Không bắt đầu trị liệu • " + autoStopDetail;
            return false;
        }
        if (!OpenNpc(healNpc_, detail)) return false;

        std::wstring reason;
        ButtonInfo treatment;
        if (!WaitButton(ButtonRole::Treatment, treatment, reason, 36, 35)) {
            detail = L"Đã mở NPC trị liệu nhưng chưa thấy lựa chọn ‘Trị liệu’ • " + reason;
            std::wstring closeDetail;
            InvokeMainUiScriptNoArgs("GameDialog", "ButtonCloseClicked", closeDetail);
            return false;
        }
        std::wstring actionDetail;
        if (!InvokeUiScriptOneArg("GameDialog", "FunctionButtonClicked",
                                  treatment.object, actionDetail)) {
            detail = L"GameDialog.FunctionButtonClicked(Trị liệu) không phản hồi • " + actionDetail;
            return false;
        }

        // The confirmation is a MessageBox script. Call its exact Lua action instead of resolving
        // a transient UIButton/Rect and fabricating pointer-event arguments.
        std::wstring confirmDetail;
        if (!WaitLuaNoArgs("MessageBox", "ButtonOKClicked", confirmDetail, 36, 35)) {
            detail = L"Đã chọn Trị liệu nhưng MessageBox.ButtonOKClicked chưa sẵn sàng • " +
                     confirmDetail;
            return false;
        }

        ButtonInfo ack;
        reason.clear();
        if (!WaitButton(ButtonRole::TreatmentAck, ack, reason, 36, 35)) {
            detail = L"Đã xác nhận trị liệu nhưng chưa thấy ‘Ta biết rồi’ • " + reason;
            return false;
        }
        std::wstring ackDetail;
        if (!InvokeUiScriptOneArg("GameDialog", "FunctionButtonClicked",
                                  ack.object, ackDetail)) {
            detail = L"GameDialog.FunctionButtonClicked(Ta biết rồi) không phản hồi • " + ackDetail;
            return false;
        }

        detail = L"Đã hoàn tất trị liệu bằng Lua thật: GameDialog → MessageBox → GameDialog • " +
                 healNpc_.name;
        return true;
    }

    bool TriggerAutoFight(std::wstring& detail) {
        // Do not replace the real menu action with set_AutoFight.  Static
        // analysis shows that setter only writes PlayZone+0x20; the game's
        // Auto -> Đánh quái callbacks perform additional setup.
        return ClickInternalAutoFight(detail);
    }
    bool StartPathTo(const Spot& destination) {
        std::uint64_t ignored = 0;
        return Remote(rva::AutoPathStart, autoPathManager_,
                      static_cast<std::uint32_t>(destination.mapID),
                      static_cast<std::uint32_t>(destination.x),
                      static_cast<std::uint32_t>(destination.y), ignored, 2500);
    }
    void StopPathOnly() {
        if (!executor_ || !autoPathManager_) return;
        std::uint64_t ignored = 0;
        Remote(rva::AutoPathStop, autoPathManager_, 0, 0, 0, ignored, 700);
    }
    void DisableActions() {
        // Navigation/path shutdown must never depend on opening/scanning the AUTO UI.
        // AUTO → Dừng is attempted by Worker only when this tool itself confirmed that it
        // activated AUTO at the train spot. A stale EnableAutoF1 flag must not deadlock movement.
        StopPathOnly();
    }
    bool AtSpot(const LiveState& s, const Spot& destination) const {
        if (s.mapID != destination.mapID) return false;
        const long long dx = static_cast<long long>(s.x) - destination.x;
        const long long dy = static_cast<long long>(s.y) - destination.y;
        return dx * dx + dy * dy <= static_cast<long long>(config_.tolerance) * config_.tolerance;
    }
    bool AtTarget(const LiveState& s) const { return AtSpot(s, target_); }
    long long DistanceSquaredTo(const LiveState& s, const Spot& destination) const {
        if (s.mapID != destination.mapID) return LLONG_MAX;
        const long long dx = static_cast<long long>(s.x) - destination.x;
        const long long dy = static_cast<long long>(s.y) - destination.y;
        return dx * dx + dy * dy;
    }
    bool ResolveCrossServerPortal(const LiveState& live, const Spot& finalTarget,
                                  Spot& portal, std::wstring& routeName) const {
        portal = {};
        routeName.clear();
        // Special routing is needed only while standing in Lạc Dương Liên Server.
        // Once the server reports the actual destination map, normal navigation resumes.
        if (live.mapID != 10000 || finalTarget.mapID == 10000) return false;

        const std::wstring key = CompactMatch(finalTarget.name);
        if (finalTarget.mapID == 10005 ||
            ContainsCompact(key, {L"thanhlientrai", L"thanhlien", L"tlt"})) {
            portal = {L"Cổng Thanh Liên Trại", 10000, 15600, 8250};
            routeName = L"Thanh Liên Trại";
            return true;
        }
        if (finalTarget.mapID == 10007 ||
            ContainsCompact(key, {L"khovinhdao", L"khovinh", L"kvd"})) {
            portal = {L"Cổng Khô Vinh Đạo", 10000, 8195, 1190};
            routeName = L"Khô Vinh Đạo";
            return true;
        }
        if (finalTarget.mapID == 10004 ||
            ContainsCompact(key, {L"phamlientrai", L"phamlien", L"plt"})) {
            portal = {L"Cổng Phàm Liên Trại", 10000, 1215, 8475};
            routeName = L"Phàm Liên Trại";
            return true;
        }
        return false;
    }
    void Worker() {
        UpdateLive(false, L"Đang kết nối", L"");
        std::wstring error;
        {
            std::lock_guard<std::mutex> operation(operation_);
            if (!Initialize(error)) {
                running_ = false;
                UpdateLive(false, L"Lỗi", error);
                Cleanup();
                return;
            }
        }
        auto nextNavigate = Clock::now();
        auto lastProgress = Clock::now();
        auto nextConfirm = Clock::now();
        auto nextRideDecision = Clock::time_point{};
        auto mountFightUntil = Clock::time_point{};
        auto nextMountFightAction = Clock::now();
        auto walkFallbackUntil = Clock::time_point{};
        bool mountCleanupAutoConfirmed = false;
        int mountCleanupAutoStartAttempts = 0;
        int mountCleanupStopFailures = 0;
        int mountFailureCount = 0;
        bool awaitingRideCheck = false;
        auto nextTrainAction = Clock::now();
        auto nextAutoPositionCheck = Clock::now();
        bool autoPositionLatched = false;
        auto dauThaiRetryAfter = Clock::time_point{};
        auto transitionDeadline = Clock::time_point{};
        auto nextBagCheck = Clock::now();
        auto sellRetryAfter = Clock::time_point{};
        auto healRetryAfter = Clock::now();
        auto nextBuffCheck = Clock::now();
        auto nextBuffAction = Clock::now();
        auto actionBarrierUntil = Clock::time_point{};
        // NPC UI is an exclusive action domain. While treatment/shop callbacks are being
        // processed, do not interleave map/mount/AUTO/AutoPath probes. After the UI is closed,
        // leave the client completely quiet for a short period, then require the normal two
        // stable RefreshLive scans before navigation is allowed to resume.
        auto npcUiQuietUntil = Clock::time_point{};
        std::wstring npcUiQuietPhase;
        std::wstring npcUiQuietDetail;
        bool ctrlTabCombatActive = false;
        bool autoCombatConfirmed = false;
        int autoActivationFailures = 0;
        bool sellingTrip = false;
        bool healingTrip = config_.healAtStart && healNpc_.roleID > 0;
        int sellFailures = 0;
        int healFailures = 0;
        bool buffCycle = false;
        std::size_t buffIndex = 0;
        bool outsideTarget = false;
        bool deathLatched = false;
        bool dauThaiClicked = false;
        int deathStableScans = 0;
        int aliveAfterDeathScans = 0;
        int dauThaiRetries = 0;
        bool awaitingMapTransition = false;
        bool transitionSeen = false;
        bool mapGuard = false;
        bool recovering = false;
        int readyScans = 0;
        int healthyScans = 2;
        std::int32_t lastObservedMap = 0;
        long long bestDistance = LLONG_MAX;
        while (running_.load()) {
            {
                std::lock_guard<std::mutex> operation(operation_);
                if (!process_.Alive()) {
                    UpdateLive(false, L"Game đã đóng", L"");
                    running_ = false;
                    break;
                }
                error.clear();
                const auto cycleNow = Clock::now();
                const bool windowHung = game_.window && IsWindow(game_.window) &&
                                        IsHungAppWindow(game_.window);
                if (cycleNow < npcUiQuietUntil) {
                    // Important: no IL2CPP status probes here. The NPC Lua UI/server response may
                    // still be destroying panels and changing task state; probing map/mount/AUTO in
                    // this window was the source of “Mất phản hồi trạng thái chuyển map” immediately
                    // after treatment/selling and could leave the client unstable.
                    UpdateLive(true,
                               npcUiQuietPhase.empty() ? L"CHỜ NPC HOÀN TẤT" : npcUiQuietPhase,
                               npcUiQuietDetail.empty()
                                   ? L"Đang giữ yên toàn bộ AutoPath/ngựa/AUTO/map-check sau thao tác NPC"
                                   : npcUiQuietDetail);
                } else if (windowHung) {
                    recovering = true;
                    healthyScans = 0;
                    ctrlTabCombatActive = false;
                    autoCombatConfirmed = false;
                    UpdateLive(true, L"TẠM DỪNG AN TOÀN",
                               L"Windows báo cửa sổ game đang treo • không gửi thêm lệnh");
                } else if (!RefreshLive(error)) {
                    if (!executor_ || !executor_->Alive()) {
                        UpdateLive(false, L"Mất kết nối", error);
                        running_ = false;
                    } else {
                        recovering = true;
                        healthyScans = 0;
                        ctrlTabCombatActive = false;
                        autoCombatConfirmed = false;
                        UpdateLive(true, L"TẠM DỪNG AN TOÀN",
                                   error + (executor_->Idle()
                                       ? L" • đang chờ hai lần đọc ổn định"
                                       : L" • lệnh cũ còn bận, tuyệt đối không ghi đè"));
                    }
                } else {
                    LiveState live = State();
                    const auto now = Clock::now();

                    // The two combat modes deliberately have different coordinate policies.
                    // Ctrl+Tab mode reacts to X/Y every worker cycle.  AUTO → Đánh quái checks
                    // X/Y continuously only until the first verified arrival; once AUTO is active,
                    // the current spot is trusted for five minutes at a time.  MapID is still
                    // guarded continuously so an unexpected map transition is never ignored.
                    bool atTrainingTarget = false;
                    if (config_.activation == TrainActivationMode::CtrlTabSkill) {
                        atTrainingTarget = AtTarget(live);
                    } else if (live.mapID == target_.mapID) {
                        if (autoCombatConfirmed && autoPositionLatched &&
                            now < nextAutoPositionCheck) {
                            atTrainingTarget = true;
                        } else {
                            atTrainingTarget = AtTarget(live);
                            if (autoCombatConfirmed && autoPositionLatched &&
                                now >= nextAutoPositionCheck) {
                                if (atTrainingTarget)
                                    nextAutoPositionCheck = now + std::chrono::minutes(5);
                                else
                                    autoPositionLatched = false;
                            }
                        }
                    }
                    if (!recovering && live.mapReady && !live.waitingChangeMap && !live.dead &&
                        config_.autoSell && !sellingTrip && !healingTrip && now >= nextBagCheck) {
                        int probedFree = -1;
                        if (ReadFreeBagSpace(probedFree)) {
                            live.freeBagSpace = probedFree;
                            UpdateFreeBagSpace(probedFree);
                            if (probedFree > 0)
                                nextBagCheck = now + std::chrono::minutes(config_.bagCheckMinutes);
                            // Keep nextBagCheck due when full so the branch below can latch
                            // the selling trip in this same worker iteration.
                        } else {
                            live.freeBagSpace = -1;
                            UpdateFreeBagSpace(-1);
                            nextBagCheck = now + std::chrono::seconds(10);
                        }
                    }


                    if (recovering) {
                        ++healthyScans;
                        if (healthyScans == 1) StopPathOnly();
                        if (healthyScans < 2) {
                            UpdateLive(true, L"TẠM DỪNG AN TOÀN",
                                       L"Game đã phản hồi lại • kiểm tra ổn định lần 1/2");
                        } else {
                            recovering = false;
                            nextNavigate = now;
                            lastProgress = now;
                            bestDistance = LLONG_MAX;
                        }
                    }

                    if (!recovering && now >= actionBarrierUntil) {
                        if (live.mapID > 0 && lastObservedMap > 0 &&
                            live.mapID != lastObservedMap) {
                            mapGuard = true;
                            readyScans = 0;
                            transitionSeen = true;
                            lastProgress = now;
                            bestDistance = LLONG_MAX;
                            nextNavigate = now;
                        }
                        if (live.mapID > 0) lastObservedMap = live.mapID;

                        if (!live.mapReady || live.waitingChangeMap) {
                            ctrlTabCombatActive = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            walkFallbackUntil = Clock::time_point{};
                            mountCleanupAutoConfirmed = false;
                            mountCleanupAutoStartAttempts = 0;
                            mountCleanupStopFailures = 0;
                            mountFailureCount = 0;
                            autoPositionLatched = false;
                            mapGuard = true;
                            readyScans = 0;
                            transitionSeen = true;
                            UpdateLive(true, L"ĐANG CHUYỂN MAP",
                                       L"Game chưa sẵn sàng • tạm dừng toàn bộ lệnh");
                        } else if (awaitingMapTransition && !transitionSeen) {
                            if (now < transitionDeadline) {
                                UpdateLive(true, L"ĐANG QUA CỔNG",
                                           L"Đã gọi nút xác nhận nội bộ • chờ cờ chuyển map");
                            } else {
                                awaitingMapTransition = false;
                                nextNavigate = now;
                                UpdateLive(true, L"Đang ra bãi",
                                           L"Game chưa chuyển map; cho phép dò đường lại");
                            }
                        } else if (mapGuard) {
                            ++readyScans;
                            if (readyScans < 2) {
                                UpdateLive(true, L"ĐANG ỔN ĐỊNH MAP",
                                           L"Map đã sẵn sàng • xác nhận ổn định lần 1/2");
                            } else {
                                mapGuard = false;
                                awaitingMapTransition = false;
                                transitionSeen = false;
                                lastProgress = now;
                                bestDistance = LLONG_MAX;
                                nextNavigate = now;
                            }
                        } else if (live.dead) {
                            ctrlTabCombatActive = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            walkFallbackUntil = Clock::time_point{};
                            mountCleanupAutoConfirmed = false;
                            mountCleanupAutoStartAttempts = 0;
                            mountCleanupStopFailures = 0;
                            mountFailureCount = 0;
                            autoPositionLatched = false;
                            aliveAfterDeathScans = 0;
                            if (!deathLatched) {
                                // Critical repeated-PK guard: do NOT open AUTO/other UI on the death edge.
                                // Stop only AutoPath, then wait for the death/map state to be stable before
                                // touching the freshly-created death overlay. This avoids stale UIObject races.
                                std::uint64_t ignored = 0;
                                if (autoPathManager_)
                                    Remote(rva::AutoPathStop, autoPathManager_, 0, 0, 0, ignored, 700);
                                deathLatched = true;
                                dauThaiClicked = false;
                                deathStableScans = 0;
                                dauThaiRetries = 0;
                                dauThaiRetryAfter = now + std::chrono::milliseconds(350);
                            }
                            if (live.mapReady && !live.waitingChangeMap) ++deathStableScans;
                            else deathStableScans = 0;

                            if (dauThaiClicked && now < dauThaiRetryAfter) {
                                UpdateLive(true, L"ĐANG ĐẦU THAI",
                                           L"Đã gọi Đầu thai một lần • khóa toàn bộ UI/path cho tới khi IsDeath tắt");
                            } else if (dauThaiClicked && now >= dauThaiRetryAfter && dauThaiRetries < 1 &&
                                       deathStableScans >= 2) {
                                // One single late retry is allowed only if the death UI is still truly active.
                                std::wstring detail;
                                const bool clicked = ClickInternalDauThai(detail);
                                ++dauThaiRetries;
                                dauThaiRetryAfter = now + std::chrono::seconds(15);
                                UpdateLive(true, L"ĐANG ĐẦU THAI",
                                           detail + L" • retry an toàn " + std::to_wstring(dauThaiRetries) + L"/1");
                                (void)clicked;
                            } else if (!dauThaiClicked && deathStableScans >= 2 && now >= dauThaiRetryAfter) {
                                std::wstring detail;
                                const bool clicked = ClickInternalDauThai(detail);
                                if (clicked) {
                                    dauThaiClicked = true;
                                    dauThaiRetryAfter = now + std::chrono::seconds(15);
                                } else {
                                    dauThaiRetryAfter = now + std::chrono::seconds(2);
                                }
                                UpdateLive(true, clicked ? L"ĐANG ĐẦU THAI" : L"NHÂN VẬT ĐÃ CHẾT", detail);
                            } else {
                                UpdateLive(true, L"NHÂN VẬT ĐÃ CHẾT",
                                           L"Chờ IsDeath + MapReady ổn định 2 lần trước khi gọi Đầu thai");
                            }
                        } else if (deathLatched) {
                            ++aliveAfterDeathScans;
                            if (aliveAfterDeathScans < 2) {
                                UpdateLive(true, L"ĐÃ ĐẦU THAI",
                                           L"IsDeath đã tắt • xác nhận sống ổn định lần 1/2; chưa gửi UI/path");
                            } else {
                                deathLatched = false;
                                dauThaiClicked = false;
                                deathStableScans = 0;
                                dauThaiRetries = 0;
                                mapGuard = true;
                                readyScans = 0;
                                nextNavigate = now;
                                if (config_.healAtStart && healNpc_.roleID > 0) {
                                    healingTrip = true;
                                    healFailures = 0;
                                    healRetryAfter = now;
                                }
                                UpdateLive(true, L"ĐÃ ĐẦU THAI",
                                           L"Sống ổn định 2 lần • tiếp tục guard map trước khi ra bãi");
                            }
                        } else if (config_.autoSell && !sellingTrip && !healingTrip &&
                                   now >= nextBagCheck && live.freeBagSpace == 0 &&
                                   sellNpc_.roleID > 0) {
                            DisableActions();
                            ctrlTabCombatActive = false;
                            // Selling owns this PID, but do not even latch the selling trip while
                            // persistent AUTO combat is still active.  Opening an NPC while AUTO can
                            // retarget was one concrete race behind the crash reports.
                            if (live.autoFight) {
                                std::wstring stopDetail;
                                const bool stopped = ClickInternalAutoStop(stopDetail);
                                autoCombatConfirmed = false;
                                autoPositionLatched = false;
                                if (!stopped) {
                                    nextBagCheck = now + std::chrono::seconds(3);
                                    const auto quietUntil =
                                        Clock::now() + std::chrono::milliseconds(900);
                                    actionBarrierUntil = std::max(actionBarrierUntil, quietUntil);
                                    UpdateLive(true, L"CHỜ DỪNG AUTO TRƯỚC KHI BÁN",
                                               stopDetail + L" • chưa đi NPC/chưa mở shop");
                                    continue;
                                }
                                const auto quietUntil =
                                    Clock::now() + std::chrono::milliseconds(900);
                                actionBarrierUntil = std::max(actionBarrierUntil, quietUntil);
                                nextNavigate = quietUntil;
                                nextRideDecision = quietUntil;
                            }
                            autoCombatConfirmed = false;
                            autoPositionLatched = false;
                            sellingTrip = true;
                            outsideTarget = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            walkFallbackUntil = Clock::time_point{};
                            mountCleanupAutoConfirmed = false;
                            mountCleanupAutoStartAttempts = 0;
                            mountCleanupStopFailures = 0;
                            mountFailureCount = 0;
                            bestDistance = LLONG_MAX;
                            lastProgress = now;
                            nextNavigate = now;
                            sellRetryAfter = now;
                            UpdateLive(true, L"TAY NẢI ĐÃ FULL",
                                       L"GetFreeBagSpace=0 • dừng train và đi NPC " + sellNpc_.name);
                        } else if (healingTrip &&
                                   AtSpot(live, Spot{healNpc_.name, healNpc_.mapID,
                                                    healNpc_.x, healNpc_.y})) {
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            walkFallbackUntil = Clock::time_point{};
                            mountCleanupAutoConfirmed = false;
                            mountCleanupAutoStartAttempts = 0;
                            mountCleanupStopFailures = 0;
                            mountFailureCount = 0;
                            if (live.autoPathing) {
                                std::uint64_t ignored = 0;
                                Remote(rva::AutoPathStop, autoPathManager_, 0, 0, 0, ignored);
                            }
                            if (live.riding) {
                                if (now >= nextRideDecision) {
                                    const bool sent = ToggleRide(false);
                                    nextRideDecision = now + std::chrono::seconds(4);
                                    UpdateLive(true, L"ĐÃ TỚI NPC TRỊ LIỆU",
                                               sent ? L"Đã yêu cầu xuống ngựa • chờ 4 giây"
                                                    : L"Xuống ngựa thất bại • thử lại sau 4 giây");
                                }
                            } else if (now >= healRetryAfter) {
                                // EXCLUSIVE NPC MODE: from this point until TryTreatmentAtNpc has
                                // closed its UI, this worker performs no navigation/mount/AUTO action.
                                StopPathOnly();
                                ctrlTabCombatActive = false;
                                buffCycle = false;
                                awaitingRideCheck = false;
                                mountFightUntil = Clock::time_point{};
                                awaitingMapTransition = false;
                                transitionSeen = false;
                                mapGuard = false;
                                readyScans = 0;
                                UpdateLive(true, L"TRỊ LIỆU ĐỘC QUYỀN",
                                           L"Khóa AutoPath/ngựa/AUTO/map-check • chỉ xử lý chuỗi NPC trị liệu");
                                std::wstring detail;
                                const bool healed = TryTreatmentAtNpc(detail);
                                // UI was just closed/destroyed. Do not immediately interrogate map or
                                // mount state; give Lua/server state 900 ms of silence, then reuse the
                                // existing recovery gate which requires two stable RefreshLive scans.
                                const auto npcDone = Clock::now();
                                npcUiQuietUntil = npcDone + std::chrono::milliseconds(900);
                                npcUiQuietPhase = healed ? L"ĐÃ TRỊ LIỆU • CHỜ GAME ỔN ĐỊNH"
                                                         : L"TRỊ LIỆU LỖI • CHỜ GAME ỔN ĐỊNH";
                                npcUiQuietDetail = detail + L" • tạm khóa mọi lệnh 0.9 giây rồi xác minh trạng thái 2 lần";
                                recovering = true;
                                healthyScans = 0;
                                actionBarrierUntil = npcUiQuietUntil;
                                nextNavigate = npcUiQuietUntil;
                                nextConfirm = npcUiQuietUntil;
                                nextRideDecision = npcUiQuietUntil;
                                lastProgress = npcDone;
                                bestDistance = LLONG_MAX;
                                if (healed || ++healFailures >= 3) {
                                    healingTrip = false;
                                    outsideTarget = false;
                                    nextNavigate = npcUiQuietUntil;
                                    bestDistance = LLONG_MAX;
                                    lastProgress = npcDone;
                                    UpdateLive(true, healed ? L"ĐÃ TRỊ LIỆU" : L"BỎ QUA TRỊ LIỆU",
                                               detail + (healed ? L" • quay lại bãi " + target_.name
                                                                : L" • thất bại 3 lần; tiếp tục ra bãi"));
                                } else {
                                    healRetryAfter = npcUiQuietUntil + std::chrono::seconds(3);
                                    UpdateLive(true, L"TRỊ LIỆU CHƯA THÀNH CÔNG",
                                               detail + L" • thử " + std::to_wstring(healFailures) + L"/3");
                                }
                            }
                        } else if (sellingTrip &&
                                   AtSpot(live, Spot{sellNpc_.name, sellNpc_.mapID,
                                                    sellNpc_.x, sellNpc_.y})) {
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            walkFallbackUntil = Clock::time_point{};
                            mountCleanupAutoConfirmed = false;
                            mountCleanupAutoStartAttempts = 0;
                            mountCleanupStopFailures = 0;
                            mountFailureCount = 0;
                            if (live.autoPathing) {
                                std::uint64_t ignored = 0;
                                Remote(rva::AutoPathStop, autoPathManager_, 0, 0, 0, ignored);
                            }
                            if (live.riding) {
                                if (now >= nextRideDecision) {
                                    const bool sent = ToggleRide(false);
                                    nextRideDecision = now + std::chrono::seconds(4);
                                    UpdateLive(true, L"ĐÃ TỚI NPC",
                                               sent ? L"Đã yêu cầu xuống ngựa • chờ 4 giây"
                                                    : L"Xuống ngựa thất bại • thử lại sau 4 giây");
                                }
                            } else if (now >= sellRetryAfter) {
                                // EXCLUSIVE NPC MODE: the whole shop chain owns this PID until the
                                // shop/bag UI has been closed. No mount/map/AUTO/AutoPath request may
                                // be emitted in parallel with item clicks or server inventory updates.
                                StopPathOnly();
                                ctrlTabCombatActive = false;
                                buffCycle = false;
                                awaitingRideCheck = false;
                                mountFightUntil = Clock::time_point{};
                                awaitingMapTransition = false;
                                transitionSeen = false;
                                mapGuard = false;
                                readyScans = 0;
                                UpdateLive(true, L"BÁN ĐỒ ĐỘC QUYỀN",
                                           L"Khóa AutoPath/ngựa/AUTO/map-check • chỉ xử lý shop và tay nải");
                                int freeAfter = -1;
                                std::wstring detail;
                                const bool sold = TrySellAtNpc(freeAfter, detail);
                                const auto npcDone = Clock::now();
                                npcUiQuietUntil = npcDone + std::chrono::milliseconds(900);
                                npcUiQuietPhase = sold ? L"ĐÃ ĐÓNG SHOP • CHỜ GAME ỔN ĐỊNH"
                                                       : L"SHOP LỖI • CHỜ GAME ỔN ĐỊNH";
                                npcUiQuietDetail = detail + L" • tạm khóa mọi lệnh 0.9 giây rồi xác minh trạng thái 2 lần";
                                recovering = true;
                                healthyScans = 0;
                                actionBarrierUntil = npcUiQuietUntil;
                                nextNavigate = npcUiQuietUntil;
                                nextConfirm = npcUiQuietUntil;
                                nextRideDecision = npcUiQuietUntil;
                                lastProgress = npcDone;
                                bestDistance = LLONG_MAX;
                                if (sold) {
                                    sellFailures = 0;
                                    if (freeAfter >= 0) UpdateFreeBagSpace(freeAfter);
                                    sellingTrip = false;
                                    outsideTarget = false;
                                    awaitingRideCheck = false;
                                    mountFightUntil = Clock::time_point{};
                                    nextBagCheck = npcDone + std::chrono::minutes(config_.bagCheckMinutes);
                                    nextNavigate = npcUiQuietUntil;
                                    bestDistance = LLONG_MAX;
                                    lastProgress = npcDone;
                                    if (config_.healAtStart && healNpc_.roleID > 0) {
                                        healingTrip = true;
                                        healFailures = 0;
                                        healRetryAfter = now;
                                    }
                                    UpdateLive(true, L"ĐÃ BÁN ĐỒ",
                                               detail + (healingTrip
                                                   ? L" • qua NPC trị liệu trước khi về bãi"
                                                   : L" • quay lại bãi " + target_.name));
                                } else {
                                    ++sellFailures;
                                    const bool exhausted90 = detail.find(L"tối đa") != std::wstring::npos;
                                    if (sellFailures >= 3 || exhausted90) {
                                        UpdateLive(true, L"ĐÃ DỪNG RIÊNG CỬA SỔ",
                                                   detail + L" • bán đồ thất bại " +
                                                   std::to_wstring(sellFailures) +
                                                   L" lần; dừng PID này để tránh thao tác mù");
                                        running_ = false;
                                    } else {
                                        sellRetryAfter = npcUiQuietUntil + std::chrono::seconds(7);
                                        UpdateLive(true, L"BÁN ĐỒ TẠM DỪNG AN TOÀN",
                                                   detail + L" • lỗi " +
                                                   std::to_wstring(sellFailures) + L"/3");
                                    }
                                }
                            } else {
                                UpdateLive(true, L"ĐÃ TỚI NPC", L"Đang chờ thử lại chuỗi bán an toàn");
                            }
                        } else if (!sellingTrip && !healingTrip && atTrainingTarget) {
                            outsideTarget = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            walkFallbackUntil = Clock::time_point{};
                            mountCleanupAutoConfirmed = false;
                            mountCleanupAutoStartAttempts = 0;
                            mountCleanupStopFailures = 0;
                            mountFailureCount = 0;
                            bestDistance = 0;
                            if (live.autoPathing) {
                                std::uint64_t ignored = 0;
                                Remote(rva::AutoPathStop, autoPathManager_, 0, 0, 0, ignored);
                            }
                            if (live.riding) {
                                // Keep arrival identical to CtrlTabSkill: down-mount first.  Never open
                                // or scan the AUTO menu while the mount transition owns the session.
                                // Any stale AutoFight state is cleaned only after the fresh scan says
                                // riding=false, so AUTO cannot delay or race the 4-second mount check.
                                if (now >= nextRideDecision) {
                                    const bool sent = ToggleRide(false);
                                    nextRideDecision = now + std::chrono::seconds(4);
                                    UpdateLive(true, L"Đã đến bãi",
                                               sent ? L"Đã yêu cầu xuống ngựa • chờ 4 giây kiểm tra"
                                                    : L"Lệnh xuống ngựa thất bại • thử lại sau 4 giây");
                                } else {
                                    UpdateLive(true, L"Đã đến bãi",
                                               L"Đang chờ trạng thái ngựa phản hồi");
                                }
                            } else {
                                nextRideDecision = Clock::time_point{};
                                const bool wantsBuff = config_.autoBuff && !config_.buffSkillIDs.empty();
                                if (wantsBuff && (buffCycle || now >= nextBuffCheck)) {
                                    if (!buffCycle && config_.activation == TrainActivationMode::AutoFight &&
                                        autoCombatConfirmed && live.autoFight) {
                                        // Only stop AUTO that this session itself successfully activated.
                                        // Never loop on a raw/stale AutoFight flag: repeated UI scans here were
                                        // another source of hangs. One best-effort stop is enough; the next scan
                                        // begins the buff cycle regardless, and combat is re-activated afterward.
                                        std::wstring stopDetail;
                                        const bool stopped = ClickInternalAutoStop(stopDetail);
                                        autoCombatConfirmed = false;
                                        buffCycle = true;
                                        buffIndex = 0;
                                        nextBuffAction = now + std::chrono::milliseconds(350);
                                        ctrlTabCombatActive = false;
                                        UpdateLive(true, L"CHUẨN BỊ BUFF",
                                                   stopped ? L"Đã AUTO → Dừng một lần trước chu kỳ buff"
                                                           : L"Không định vị được Dừng; không retry UI liên tục, chuyển sang buff");
                                    } else {
                                        if (!buffCycle) {
                                            buffCycle = true;
                                            buffIndex = 0;
                                            nextBuffAction = now;
                                            ctrlTabCombatActive = false;
                                        }
                                        if (buffIndex >= config_.buffSkillIDs.size()) {
                                            buffCycle = false;
                                            nextBuffCheck = now + std::chrono::minutes(5);
                                            nextTrainAction = now;
                                            UpdateLive(true, L"BUFF XONG",
                                                       L"Đã kiểm tra hết buff • lần tiếp theo sau 5 phút");
                                        } else if (now >= nextBuffAction) {
                                            const int buffSkill = config_.buffSkillIDs[buffIndex];
                                            std::wstring buffDetail;
                                            const bool verified = TryApplyAndVerifyBuff(buffSkill, buffDetail);
                                            ++buffIndex;
                                            if (!verified)
                                                buffDetail += L" • chỉ thử 1 lần; bỏ qua tới lượt check buff sau";
                                            nextBuffAction = now + std::chrono::milliseconds(850);
                                            if (buffIndex >= config_.buffSkillIDs.size()) {
                                                buffCycle = false;
                                                nextBuffCheck = now + std::chrono::minutes(5);
                                                nextTrainAction = now;
                                                buffDetail += L" • hoàn tất lượt buff; check lại sau 5 phút";
                                            }
                                            UpdateLive(true, buffCycle ? L"ĐANG BUFF" : L"BUFF XONG",
                                                       buffDetail);
                                        } else {
                                            UpdateLive(true, L"ĐANG BUFF",
                                                       L"Đang chờ xác minh hiệu ứng buff");
                                        }
                                    }
                                } else if (now >= nextTrainAction) {
                                    bool sent = false;
                                    std::wstring detail;
                                    switch (config_.activation) {
                                        case TrainActivationMode::CtrlTabSkill:
                                            sent = TriggerCtrlTabSkill(detail);
                                            nextTrainAction = now +
                                                std::chrono::milliseconds(sent ? 900 : 700);
                                            break;
                                        case TrainActivationMode::AutoFight:
                                            if (autoCombatConfirmed && live.autoFight) {
                                                sent = true;
                                                detail = L"AUTO → Đánh quái đã được xác minh sau khi tới bãi";
                                            } else {
                                                // EnableAutoF1 may remain ON after a previous/dead state even while
                                                // the character is doing nothing. Do not require AUTO → Dừng before
                                                // navigation/activation. The only proof accepted here is a fresh
                                                // AUTO → Đánh quái action issued after arrival + down-mount.
                                                sent = TriggerAutoFight(detail);
                                                if (sent) {
                                                    autoCombatConfirmed = true;
                                                    autoActivationFailures = 0;
                                                    autoPositionLatched = true;
                                                    nextAutoPositionCheck =
                                                        now + std::chrono::minutes(5);
                                                } else {
                                                    autoCombatConfirmed = false;
                                                    ++autoActivationFailures;
                                                }
                                                nextTrainAction = now + std::chrono::seconds(
                                                    sent ? 2 : (autoActivationFailures >= 3 ? 15 : 5));
                                            }
                                            break;
                                    }
                                    if (config_.activation == TrainActivationMode::CtrlTabSkill && sent)
                                        ctrlTabCombatActive = true;
                                    const bool confirmed =
                                        config_.activation == TrainActivationMode::CtrlTabSkill
                                            ? ctrlTabCombatActive
                                            : autoCombatConfirmed && live.autoFight;
                                    UpdateLive(true, confirmed ? L"ĐANG TRAIN"
                                                               : L"Đang xác nhận train",
                                               target_.name + L" • " + detail);
                                } else {
                                    if (config_.activation == TrainActivationMode::AutoFight &&
                                        autoCombatConfirmed && !live.autoFight) {
                                        autoCombatConfirmed = false;
                                        nextTrainAction = now;
                                    }
                                    const bool confirmed =
                                        config_.activation == TrainActivationMode::CtrlTabSkill
                                            ? ctrlTabCombatActive
                                            : autoCombatConfirmed && live.autoFight;
                                    UpdateLive(true, confirmed ? L"ĐANG TRAIN"
                                                               : L"Đang xác nhận train",
                                               target_.name);
                                }
                            }
                        } else {
                            const bool wasAutoConfirmed = autoCombatConfirmed;
                            autoCombatConfirmed = false;
                            autoPositionLatched = false;
                            const Spot finalDestination = healingTrip
                                ? Spot{healNpc_.name, healNpc_.mapID, healNpc_.x, healNpc_.y}
                                : sellingTrip
                                    ? Spot{sellNpc_.name, sellNpc_.mapID, sellNpc_.x, sellNpc_.y}
                                    : target_;
                            Spot destination = finalDestination;
                            Spot crossServerPortal;
                            std::wstring crossServerRoute;
                            const bool usingCrossServerPortal =
                                !healingTrip && !sellingTrip &&
                                ResolveCrossServerPortal(live, target_, crossServerPortal,
                                                         crossServerRoute);
                            if (usingCrossServerPortal) destination = crossServerPortal;

                            ctrlTabCombatActive = false;
                            buffCycle = false;
                            nextTrainAction = now;
                            if (!outsideTarget) {
                                // NAVIGATION is authoritative. Never wait for a raw AutoFight flag to
                                // become OFF before AutoPath/mount/portal commands. If this tool itself
                                // previously confirmed AUTO at the train spot, request AUTO → Dừng once
                                // as best effort, but movement proceeds even if the UI cannot be resolved.
                                StopPathOnly();
                                mountFailureCount = 0;
                                walkFallbackUntil = Clock::time_point{};
                                mountCleanupAutoConfirmed = false;
                                mountCleanupAutoStartAttempts = 0;
                                mountCleanupStopFailures = 0;
                                if (config_.activation == TrainActivationMode::AutoFight &&
                                    wasAutoConfirmed && live.mapReady && !live.waitingChangeMap) {
                                    std::wstring stopDetail;
                                    ClickInternalAutoStop(stopDetail);

                                    // Do not issue AutoPath/mount/portal work in the same worker turn as
                                    // the Lua AUTO-stop action.  Give the game a short quiet window to
                                    // finish its UI/Lua state transition first; this avoids two independent
                                    // control requests racing each other on the same client.
                                    const auto quietUntil = Clock::now() + std::chrono::milliseconds(700);
                                    actionBarrierUntil = std::max(actionBarrierUntil, quietUntil);
                                    nextNavigate = quietUntil;
                                    nextConfirm = quietUntil;
                                    nextRideDecision = quietUntil;
                                } else {
                                    nextNavigate = now;
                                }
                                outsideTarget = true;
                            }
                            const long long distance = DistanceSquaredTo(live, destination);
                            if (distance != LLONG_MAX &&
                                (bestDistance == LLONG_MAX || distance + 2500 < bestDistance)) {
                                bestDistance = distance;
                                lastProgress = now;
                            }
                            const bool mountSettled = now >= nextRideDecision;
                            const bool stalled = now - lastProgress >= std::chrono::seconds(7) &&
                                                 !live.moving;
                            const bool atCrossServerGate =
                                usingCrossServerPortal && AtSpot(live, destination);
                            const bool portalConfirmationReady =
                                mountFightUntil == Clock::time_point{} && mountSettled &&
                                live.messageBoxVisible && now >= nextConfirm &&
                                ((usingCrossServerPortal && (atCrossServerGate || stalled)) ||
                                 (!usingCrossServerPortal &&
                                  live.mapID != destination.mapID && stalled));
                            if (portalConfirmationReady) {
                                // Cross-server maps must first leave Map 10000 through their exact
                                // portal. For generic routes preserve the existing stalled-at-gate logic.
                                StopPathOnly();
                                std::wstring detail;
                                const bool clicked = ClickInternalConfirm(detail);
                                nextConfirm = now + std::chrono::seconds(
                                    clicked ? config_.retrySeconds : 2);
                                lastProgress = now;
                                if (clicked) {
                                    awaitingMapTransition = true;
                                    transitionSeen = false;
                                    transitionDeadline = now + std::chrono::seconds(12);
                                }
                                UpdateLive(true, L"ĐANG QUA CỔNG", detail);
                            } else if (live.riding) {
                                // Mount success owns the next transition.  Once riding=true, never
                                // select/attack another target in this recovery cycle; doing so can
                                // make the game dismount again immediately.
                                awaitingRideCheck = false;
                                mountFightUntil = Clock::time_point{};
                                walkFallbackUntil = Clock::time_point{};
                                mountFailureCount = 0;
                                if (config_.activation == TrainActivationMode::AutoFight &&
                                    (mountCleanupAutoConfirmed || live.autoFight)) {
                                    // A delayed mount can complete while the 10-second AUTO cleanup
                                    // is still active. riding=true has absolute priority: cancel AUTO
                                    // immediately on the first scan that sees the mount, rather than
                                    // waiting for the old 10-second cleanup timer. Waiting here let the
                                    // game's own AUTO acquire another mob and pull the character back
                                    // off the mount even though this tool itself no longer targeted.
                                    if (mountCleanupStopFailures == 0 || now >= nextMountFightAction) {
                                        std::wstring stopDetail;
                                        const bool stopped = ClickInternalAutoStop(stopDetail);
                                        if (stopped) {
                                            mountCleanupAutoConfirmed = false;
                                            mountCleanupAutoStartAttempts = 0;
                                            mountCleanupStopFailures = 0;
                                            const auto quietUntil =
                                                Clock::now() + std::chrono::milliseconds(700);
                                            actionBarrierUntil = std::max(actionBarrierUntil, quietUntil);
                                            nextNavigate = quietUntil;
                                            nextRideDecision = quietUntil;
                                            nextMountFightAction = quietUntil;
                                            UpdateLive(true, L"ĐÃ LÊN NGỰA",
                                                       L"riding=true • dừng AUTO ngay lập tức • tuyệt đối không retarget");
                                        } else {
                                            ++mountCleanupStopFailures;
                                            nextMountFightAction = now + std::chrono::seconds(2);
                                            // Do not start any target/skill action while mounted.  On the next
                                            // eligible scan only retry Stop; movement stays paused until AUTO
                                            // is actually verified OFF.
                                            UpdateLive(true, L"ĐÃ LÊN NGỰA • CHỜ DỪNG AUTO",
                                                       stopDetail + L" • không target/skill/AutoPath khi AUTO còn chưa dừng");
                                        }
                                    } else {
                                        UpdateLive(true, L"ĐÃ LÊN NGỰA • CHỜ DỪNG AUTO",
                                                   L"Không target/skill/AutoPath • retry Dừng sau 2 giây");
                                    }
                                } else if (now >= nextNavigate) {
                                    mountCleanupAutoStartAttempts = 0;
                                    mountCleanupStopFailures = 0;
                                    nextRideDecision = Clock::time_point{};
                                    const bool sent = StartPathTo(destination);
                                    UpdateLive(true, L"Đang ra bãi",
                                               sent ? L"Đã lên ngựa • gửi AutoPath trực tiếp"
                                                    : L"Đã lên ngựa nhưng gửi AutoPath thất bại; sẽ thử lại");
                                    nextNavigate = now +
                                        std::chrono::seconds(config_.retrySeconds);
                                }
                            } else if (walkFallbackUntil > now) {
                                // After two failed mount attempts, walk for one minute.  No combat
                                // targeting is allowed in this window; AutoPath alone owns movement.
                                awaitingRideCheck = false;
                                mountFightUntil = Clock::time_point{};
                                mountCleanupAutoConfirmed = false;
                                if (!live.autoPathing || now >= nextNavigate) {
                                    const bool sent = StartPathTo(destination);
                                    nextNavigate = now + std::chrono::seconds(config_.retrySeconds);
                                    (void)sent;
                                }
                                const auto remain = std::chrono::duration_cast<std::chrono::seconds>(
                                    walkFallbackUntil - now).count();
                                UpdateLive(true, L"CHẠY BỘ TẠM THỜI",
                                           L"2 lần lên ngựa thất bại • chạy bộ còn " +
                                           std::to_wstring(std::max<long long>(1, remain)) +
                                           L" giây; không target/đánh quái");
                            } else if (walkFallbackUntil != Clock::time_point{}) {
                                // One-minute walk window is over.  Stop the current foot path and
                                // restart the mount sequence from attempt #1.
                                walkFallbackUntil = Clock::time_point{};
                                mountFailureCount = 0;
                                StopPathOnly();
                                nextRideDecision = now;
                                nextNavigate = now;
                                UpdateLive(true, L"THỬ LÊN NGỰA LẠI",
                                           L"Đã chạy bộ 1 phút • bắt đầu lại chu kỳ lên ngựa");
                            } else if (mountFightUntil > now) {
                                std::wstring fightDetail;
                                if (config_.activation == TrainActivationMode::CtrlTabSkill) {
                                    if (now >= nextMountFightAction) {
                                        const bool sent = TriggerCtrlTabSkill(fightDetail);
                                        nextMountFightAction = now +
                                            std::chrono::milliseconds(sent ? 900 : 700);
                                    }
                                } else {
                                    // AUTO mode: at most two StartAutoFight attempts in the whole
                                    // 10-second cleanup window. Repeated Lua invokes every worker tick
                                    // were an unnecessary crash amplifier when AUTO failed to confirm.
                                    if (!mountCleanupAutoConfirmed && now >= nextMountFightAction &&
                                        mountCleanupAutoStartAttempts < 2) {
                                        ++mountCleanupAutoStartAttempts;
                                        const bool sent = TriggerAutoFight(fightDetail);
                                        mountCleanupAutoConfirmed = sent;
                                        nextMountFightAction = sent
                                            ? mountFightUntil
                                            : (mountCleanupAutoStartAttempts < 2
                                                ? now + std::chrono::seconds(3)
                                                : mountFightUntil);
                                    } else if (mountCleanupAutoConfirmed) {
                                        fightDetail = L"AUTO → Đánh quái đang dọn quái";
                                    } else if (mountCleanupAutoStartAttempts >= 2) {
                                        fightDetail = L"AUTO chưa xác nhận sau 2 lần • không gọi lặp trong pha dọn quái";
                                    }
                                }
                                const auto remain = std::chrono::duration_cast<std::chrono::seconds>(
                                    mountFightUntil - now).count();
                                UpdateLive(true, L"DỌN QUÁI TRƯỚC KHI LÊN NGỰA",
                                           L"Sau 4 giây vẫn chưa lên ngựa • đánh thêm khoảng " +
                                           std::to_wstring(std::max<long long>(1, remain)) +
                                           L" giây rồi thử lại" +
                                           (fightDetail.empty() ? L"" : L" • " + fightDetail));
                            } else if (mountFightUntil != Clock::time_point{}) {
                                // Cleanup window has ended.  AUTO mode must be fully stopped before
                                // another mount request; otherwise an enemy target can immediately
                                // pull the character off the mount.
                                if (config_.activation == TrainActivationMode::AutoFight &&
                                    (mountCleanupAutoConfirmed || live.autoFight)) {
                                    std::wstring stopDetail;
                                    const bool stopped = ClickInternalAutoStop(stopDetail);
                                    if (!stopped && ++mountCleanupStopFailures < 2) {
                                        mountFightUntil = now + std::chrono::seconds(2);
                                        nextMountFightAction = mountFightUntil;
                                        UpdateLive(true, L"CHỜ DỪNG AUTO",
                                                   stopDetail + L" • chưa gọi ngựa cho tới khi AUTO dừng");
                                    } else if (stopped) {
                                        mountCleanupAutoConfirmed = false;
                                        mountCleanupAutoStartAttempts = 0;
                                        mountCleanupStopFailures = 0;
                                        mountFightUntil = Clock::time_point{};
                                        const auto quietUntil =
                                            Clock::now() + std::chrono::milliseconds(700);
                                        actionBarrierUntil = std::max(actionBarrierUntil, quietUntil);
                                        nextRideDecision = quietUntil;
                                        nextNavigate = quietUntil;
                                        UpdateLive(true, L"Đang ra bãi",
                                                   L"Đã dừng AUTO sau 10 giây dọn quái • chuẩn bị thử ngựa");
                                    } else {
                                        // Never mount while AUTO is still unverified: that is exactly
                                        // the condition that can retarget a nearby mob and pull the
                                        // character off the mount.  Fall back to walking for one minute,
                                        // then restart a clean mount cycle.
                                        mountCleanupAutoConfirmed = false;
                                        mountCleanupAutoStartAttempts = 0;
                                        mountCleanupStopFailures = 0;
                                        mountFightUntil = Clock::time_point{};
                                        mountFailureCount = 0;
                                        walkFallbackUntil = now + std::chrono::minutes(1);
                                        const bool sent = StartPathTo(destination);
                                        nextNavigate = now + std::chrono::seconds(config_.retrySeconds);
                                        nextRideDecision = Clock::time_point{};
                                        UpdateLive(true, L"CHẠY BỘ 1 PHÚT",
                                                   stopDetail +
                                                   (sent
                                                       ? L" • AUTO chưa dừng chắc chắn nên không gọi ngựa; chạy bộ 1 phút"
                                                       : L" • AUTO chưa dừng chắc chắn; hoãn ngựa và retry AutoPath chạy bộ"));
                                    }
                                } else {
                                    mountFightUntil = Clock::time_point{};
                                    nextRideDecision = now;
                                    nextNavigate = now;
                                    UpdateLive(true, L"Đang ra bãi",
                                               L"Kết thúc 10 giây dọn quái • chuẩn bị thử lên ngựa lại");
                                }
                            } else if (awaitingRideCheck && now < nextRideDecision) {
                                UpdateLive(true, L"Đang ra bãi",
                                           L"Đã gọi ngựa • chờ đủ 4 giây kiểm tra lại");
                            } else if (!awaitingRideCheck &&
                                       nextRideDecision != Clock::time_point{} &&
                                       now < nextRideDecision) {
                                // A rejected/failed ToggleRide call must still respect the same
                                // four-second retry cadence.  Never spam mount requests every worker tick.
                                UpdateLive(true, L"Đang ra bãi",
                                           L"Lệnh ngựa chưa được nhận • chờ đủ 4 giây rồi thử lại");
                            } else if (!live.riding) {
                                if (awaitingRideCheck) {
                                    awaitingRideCheck = false;
                                    ++mountFailureCount;
                                    if (mountFailureCount >= 2) {
                                        // Exactly two failed mount attempts: walk for one minute,
                                        // then reset the counter and retry the whole mount cycle.
                                        mountFailureCount = 0;
                                        mountFightUntil = Clock::time_point{};
                                        mountCleanupAutoConfirmed = false;
                                        mountCleanupAutoStartAttempts = 0;
                                        mountCleanupStopFailures = 0;
                                        walkFallbackUntil = now + std::chrono::minutes(1);
                                        const bool sent = StartPathTo(destination);
                                        nextNavigate = now + std::chrono::seconds(config_.retrySeconds);
                                        nextRideDecision = Clock::time_point{};
                                        UpdateLive(true, L"CHẠY BỘ 1 PHÚT",
                                                   sent
                                                       ? L"Lên ngựa thất bại lần 2/2 • chạy bộ 1 phút rồi thử ngựa lại"
                                                       : L"Lên ngựa thất bại lần 2/2 • bắt đầu pha chạy bộ, AutoPath sẽ retry");
                                    } else {
                                        mountFightUntil = now + std::chrono::seconds(10);
                                        nextMountFightAction = now;
                                        nextRideDecision = mountFightUntil;
                                        nextNavigate = mountFightUntil;
                                        std::wstring fightDetail;
                                        if (config_.activation == TrainActivationMode::CtrlTabSkill) {
                                            const bool sent = TriggerCtrlTabSkill(fightDetail);
                                            nextMountFightAction = now +
                                                std::chrono::milliseconds(sent ? 900 : 700);
                                        } else {
                                            mountCleanupAutoStartAttempts = 1;
                                            const bool sent = TriggerAutoFight(fightDetail);
                                            mountCleanupAutoConfirmed = sent;
                                            nextMountFightAction = sent
                                                ? mountFightUntil
                                                : now + std::chrono::seconds(3);
                                        }
                                        UpdateLive(true, L"DỌN QUÁI 10 GIÂY",
                                                   L"Lên ngựa thất bại lần " +
                                                   std::to_wstring(mountFailureCount) +
                                                   L"/2 • đánh thêm 10 giây rồi thử lại" +
                                                   (fightDetail.empty() ? L"" : L" • " + fightDetail));
                                    }
                                } else {
                                    StopPathOnly();
                                    const bool sent = ToggleRide(true);
                                    // The four-second attempt timer is authoritative even when the
                                    // immediate remote call reports failure. Otherwise a rejected mount
                                    // command can retry forever without ever reaching the 2-attempt walk fallback.
                                    awaitingRideCheck = true;
                                    nextRideDecision = now + std::chrono::seconds(4);
                                    nextNavigate = nextRideDecision;
                                    mountFightUntil = Clock::time_point{};
                                    mountCleanupAutoConfirmed = false;
                                    mountCleanupAutoStartAttempts = 0;
                                    mountCleanupStopFailures = 0;
                                    UpdateLive(true, L"Đang ra bãi",
                                               sent ? L"Đã gọi ngựa • chờ 4 giây"
                                                    : L"Gọi ngựa thất bại • thử lại sau 4 giây");
                                }
                            } else {
                                UpdateLive(true, L"Đang ra bãi",
                                           (healingTrip ? L"NPC trị liệu " : sellingTrip ? L"NPC bán đồ " : L"Đích ") + destination.name +
                                           L" • Map " + std::to_wstring(destination.mapID) +
                                           L" • " + std::to_wstring(destination.x) + L"," +
                                           std::to_wstring(destination.y));
                            }
                        }
                    }
                }
            }
            for (int i = 0; i < 5 && running_.load(); ++i) Sleep(100);
        }
        // If this session itself proved AUTO combat was activated at the train spot,
        // request the normal AUTO → Dừng once before detach. Never loop/retry on shutdown.
        if (config_.activation == TrainActivationMode::AutoFight && autoCombatConfirmed) {
            std::wstring stopDetail;
            ClickInternalAutoStop(stopDetail);
        }
        std::lock_guard<std::mutex> operation(operation_);
        DisableActions();
        Cleanup();
    }

    GameProcess game_;
    Spot target_;
    TrainConfig config_;
    SellNpc sellNpc_;
    SellNpc healNpc_;
    ProcessMemory process_;
    ModuleInfo module_;
    std::unique_ptr<RemoteExecutor> executor_;
    std::uint64_t autoPathManager_ = 0;
    std::uint64_t il2cppStringNew_ = 0;
    std::uint64_t messageBoxNameHandle_ = 0;
    std::uint64_t scratchBuffer_ = 0;
    std::uint64_t staticValueBuffer_ = 0;
    std::uint64_t il2cppDomainGet_ = 0;
    std::uint64_t il2cppDomainAssemblyOpen_ = 0;
    std::uint64_t il2cppAssemblyGetImage_ = 0;
    std::uint64_t il2cppClassFromName_ = 0;
    std::uint64_t il2cppClassIsAssignableFrom_ = 0;
    std::uint64_t il2cppClassGetField_ = 0;
    std::uint64_t il2cppFieldStaticGetValue_ = 0;
    std::uint64_t il2cppGcHandleNew_ = 0;
    std::uint64_t il2cppGcHandleGetTarget_ = 0;
    std::uint64_t il2cppGcHandleFree_ = 0;
    std::uint64_t il2cppGetCorlib_ = 0;
    std::uint64_t il2cppArrayNew_ = 0;
    std::uint64_t il2cppValueBox_ = 0;
    std::uint64_t assemblyImage_ = 0;
    std::uint64_t uiObjectClass_ = 0;
    std::uint64_t uiButtonClass_ = 0;
    std::uint64_t uiToggleClass_ = 0;
    std::uint64_t uiInstancesField_ = 0;
    std::uint64_t systemObjectClass_ = 0;
    std::uint64_t systemInt32Class_ = 0;
    std::map<std::uint64_t, bool> uiButtonClassCache_;
    std::map<std::uint64_t, bool> uiToggleClassCache_;
    // Item template classification is immutable for this client build.  Cache it so a sell
    // rescan does not call managed item-type helpers for every unchanged template each time.
    std::map<std::int32_t, bool> itemEquipCache_;
    std::map<std::int32_t, bool> itemSellableCache_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    mutable std::mutex stateLock_;
    LiveState state_;
    std::mutex operation_;
};

enum ControlIdV5 {
    V5_GAMES = 3001, V5_REFRESH, V5_PROBE, V5_SPOT_NAME, V5_SAVE_SPOT,
    V5_DELETE_SPOT, V5_SPOTS, V5_TOLERANCE, V5_RETRY,
    V5_ACTIVATION, V5_SKILL, V5_SCAN_SKILLS, V5_START,
    V5_STOP, V5_STATE, V5_COORDS, V5_FLAGS, V5_DETAIL, V5_TAB,
    V5_AUTO_SELL, V5_BAG_MINUTES, V5_SELL_NPC, V5_SAVE_NPC,
    V5_HEAL_AT_START, V5_HEAL_NPC, V5_SAVE_HEAL_NPC,
    V5_AUTO_BUFF, V5_BUFF_SKILLS
};

struct GameRuntimeV5 {
    explicit GameRuntimeV5(GameProcess value)
        : game(std::move(value)), session(std::make_unique<TrainSession>()) {}
    GameProcess game;
    std::unique_ptr<TrainSession> session;
    TrainConfig config;
    std::vector<SkillOption> skills;
    std::wstring selectedSpot;
    std::wstring profileSection;
    LiveState last;
};

class Application {
public:
    int Run(HINSTANCE instance, int show) {
        instance_ = instance;
        SetProcessDPIAware();
        INITCOMMONCONTROLSEX controls{sizeof(controls),
                                      ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES |
                                          ICC_STANDARD_CLASSES};
        InitCommonControlsEx(&controls);
        WNDCLASSEXW klass{};
        klass.cbSize = sizeof(klass);
        klass.lpfnWndProc = WindowProc;
        klass.hInstance = instance;
        klass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        klass.hIcon = reinterpret_cast<HICON>(LoadImageW(
            instance, MAKEINTRESOURCEW(101), IMAGE_ICON, 32, 32,
            LR_DEFAULTCOLOR | LR_SHARED));
        klass.hIconSm = reinterpret_cast<HICON>(LoadImageW(
            instance, MAKEINTRESOURCEW(101), IMAGE_ICON, 16, 16,
            LR_DEFAULTCOLOR | LR_SHARED));
        klass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        klass.lpszClassName = L"ThanLongAutoTrainWindowV7";
        RegisterClassExW(&klass);
        window_ = CreateWindowExW(
            0, klass.lpszClassName, kTitle,
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT, CW_USEDEFAULT, 1080, 885, nullptr, nullptr,
            instance, this);
        if (!window_) return 1;
        ShowWindow(window_, show);
        UpdateWindow(window_);
        MSG message{};
        while (GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        return static_cast<int>(message.wParam);
    }

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam,
                                       LPARAM lParam) {
        Application* app = reinterpret_cast<Application*>(
            GetWindowLongPtrW(window, GWLP_USERDATA));
        if (message == WM_NCCREATE) {
            const auto* create = reinterpret_cast<CREATESTRUCTW*>(lParam);
            app = reinterpret_cast<Application*>(create->lpCreateParams);
            app->window_ = window;
            SetWindowLongPtrW(window, GWLP_USERDATA,
                              reinterpret_cast<LONG_PTR>(app));
        }
        return app ? app->Handle(message, wParam, lParam)
                   : DefWindowProcW(window, message, wParam, lParam);
    }

    HWND Make(const wchar_t* klass, const wchar_t* text, DWORD style,
              int x, int y, int width, int height, int id, HFONT font = nullptr,
              DWORD exStyle = 0) {
        HWND control = CreateWindowExW(
            exStyle, klass, text, WS_CHILD | WS_VISIBLE | style,
            x, y, width, height, window_,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), instance_, nullptr);
        SendMessageW(control, WM_SETFONT,
                     reinterpret_cast<WPARAM>(font ? font : font_), TRUE);
        if (buildingPage_ == 1) mainPageControls_.push_back(control);
        else if (buildingPage_ == 2) aboutPageControls_.push_back(control);
        return control;
    }

    void SelectPage(int page) {
        const bool about = page == 1;
        for (HWND control : mainPageControls_)
            ShowWindow(control, about ? SW_HIDE : SW_SHOW);
        for (HWND control : aboutPageControls_)
            ShowWindow(control, about ? SW_SHOW : SW_HIDE);
        if (tab_) TabCtrl_SetCurSel(tab_, about ? 1 : 0);
    }

    HWND Label(const wchar_t* text, int x, int y, int width, int height,
               HFONT font = nullptr, int id = 0) {
        return Make(L"STATIC", text, SS_LEFT | SS_CENTERIMAGE,
                    x, y, width, height, id, font);
    }

    HWND Button(const wchar_t* text, int x, int y, int width, int height, int id) {
        const int scaledWidth = std::max(70, (width * 3) / 5);
        const int scaledHeight = std::max(20, (height * 3) / 5);
        return Make(L"BUTTON", text, BS_PUSHBUTTON | WS_TABSTOP,
                    x, y, scaledWidth, scaledHeight, id, buttonFont_);
    }

    void AddColumn(HWND list, const wchar_t* text, int width, int index,
                   bool centered = false) {
        LVCOLUMNW column{};
        column.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
        column.pszText = const_cast<wchar_t*>(text);
        column.cx = width;
        column.fmt = centered ? LVCFMT_CENTER : LVCFMT_LEFT;
        ListView_InsertColumn(list, index, &column);
    }

    std::wstring Text(HWND control) const {
        const int count = GetWindowTextLengthW(control);
        std::wstring result(static_cast<std::size_t>(count) + 1, L'\0');
        GetWindowTextW(control, result.data(), count + 1);
        result.resize(static_cast<std::size_t>(count));
        return result;
    }

    void CreateUi() {
        font_ = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS,
                            CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                            DEFAULT_PITCH, L"Segoe UI");
        smallFont_ = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH, L"Segoe UI");
        boldFont_ = CreateFontW(-14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS,
                                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                DEFAULT_PITCH, L"Segoe UI");
        buttonFont_ = CreateFontW(-9, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                  VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS,
                                  CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                  DEFAULT_PITCH, L"Segoe UI");
        titleFont_ = CreateFontW(-21, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                 VIETNAMESE_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH, L"Segoe UI");

        Label(L"THẦN LONG MOBILE • AUTO TRAIN v0.9.0", 16, 5, 650, 31, titleFont_);
        Button(L"↻  QUÉT CỬA SỔ GAME", 855, 10, 189, 34, V5_REFRESH);

        tab_ = Make(WC_TABCONTROLW, L"", TCS_TABS | TCS_SINGLELINE | WS_TABSTOP,
                    16, 55, 1028, 28, V5_TAB, font_);
        TCITEMW tabItem{};
        tabItem.mask = TCIF_TEXT;
        tabItem.pszText = const_cast<wchar_t*>(L"AUTO TRAIN");
        TabCtrl_InsertItem(tab_, 0, &tabItem);
        tabItem.pszText = const_cast<wchar_t*>(L"GIỚI THIỆU");
        TabCtrl_InsertItem(tab_, 1, &tabItem);

        buildingPage_ = 1;
        Label(L"TICK CỬA SỔ CẦN BẮT ĐẦU / TẠM DỪNG", 16, 86, 520, 20, boldFont_);
        gamesList_ = Make(WC_LISTVIEWW, L"",
                          LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL |
                              LVS_NOSORTHEADER | WS_TABSTOP,
                          16, 107, 1028, 137, V5_GAMES, font_, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(
            gamesList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
                            LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER);
        AddColumn(gamesList_, L"PID", 78, 0, true);
        AddColumn(gamesList_, L"Nhân vật / RoleID", 252, 1);
        AddColumn(gamesList_, L"Trạng thái riêng", 306, 2);
        AddColumn(gamesList_, L"Map", 104, 3, true);
        AddColumn(gamesList_, L"Tọa độ realtime", 267, 4, true);

        stateLabel_ = Label(L"ĐÃ DỪNG", 16, 247, 130, 26, boldFont_, V5_STATE);
        coordsLabel_ = Label(L"Map -- • X -- • Y --", 152, 247, 202, 26,
                             boldFont_, V5_COORDS);
        flagsLabel_ = Label(L"Ngựa: -- • Di chuyển: -- • Chết: -- • Map: -- • Túi đồ trống: -- ô",
                            335, 247, 615, 26, smallFont_, V5_FLAGS);
        Button(L"ĐỌC LẠI VỊ TRÍ", 958, 247, 126, 26, V5_PROBE);

        Label(L"TÊN BÃI", 16, 280, 72, 24, boldFont_);
        spotName_ = Make(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                         88, 278, 315, 27, V5_SPOT_NAME, font_, WS_EX_CLIENTEDGE);
        Button(L"LƯU TỌA ĐỘ HIỆN TẠI", 413, 277, 207, 29, V5_SAVE_SPOT);
        Button(L"XÓA BÃI ĐANG CHỌN", 630, 277, 190, 29, V5_DELETE_SPOT);
        
        spotsList_ = Make(WC_LISTVIEWW, L"",
                          LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL |
                              LVS_NOSORTHEADER | WS_TABSTOP,
                          16, 312, 1028, 146, V5_SPOTS, font_, WS_EX_CLIENTEDGE);
        ListView_SetExtendedListViewStyle(
            spotsList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES |
                           LVS_EX_CHECKBOXES | LVS_EX_DOUBLEBUFFER);
        AddColumn(spotsList_, L"Tên bãi train", 452, 0);
        AddColumn(spotsList_, L"Map ID", 170, 1, true);
        AddColumn(spotsList_, L"Tọa độ X", 190, 2, true);
        AddColumn(spotsList_, L"Tọa độ Y", 190, 3, true);

        Label(L"ĐIỀU HƯỚNG", 16, 468, 105, 25, boldFont_);
        Label(L"AutoPath nội bộ", 122, 468, 180, 25, font_);
        Label(L"Sai số", 322, 468, 53, 25, smallFont_);
        tolerance_ = Make(L"EDIT", L"120", WS_BORDER | ES_NUMBER | WS_TABSTOP,
                          375, 466, 60, 27, V5_TOLERANCE, font_, WS_EX_CLIENTEDGE);
        Label(L"Lặp sau", 448, 468, 61, 25, smallFont_);
        retry_ = Make(L"EDIT", L"12", WS_BORDER | ES_NUMBER | WS_TABSTOP,
                      510, 466, 55, 27, V5_RETRY, font_, WS_EX_CLIENTEDGE);
        Label(L"giây", 571, 468, 35, 25, smallFont_);

        Label(L"BẬT TRAIN", 16, 503, 105, 25, boldFont_);
        activationCombo_ = Make(WC_COMBOBOXW, L"",
                                CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                                122, 501, 260, 160, V5_ACTIVATION);
        int activationItem = static_cast<int>(SendMessageW(
            activationCombo_, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(L"Ctrl+Tab → skill đã chọn")));
        SendMessageW(activationCombo_, CB_SETITEMDATA, activationItem,
                     static_cast<LPARAM>(TrainActivationMode::CtrlTabSkill));
        activationItem = static_cast<int>(SendMessageW(
            activationCombo_, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(L"AUTO → Đánh quái")));
        SendMessageW(activationCombo_, CB_SETITEMDATA, activationItem,
                     static_cast<LPARAM>(TrainActivationMode::AutoFight));
        skillCombo_ = Make(WC_COMBOBOXW, L"",
                           CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                           393, 501, 390, 220, V5_SKILL);
        Button(L"ĐỌC SKILL", 793, 500, 119, 29, V5_SCAN_SKILLS);
        autoSellCheck_ = Make(L"BUTTON", L"Tự bán đồ", BS_AUTOCHECKBOX | WS_TABSTOP,
                              16, 538, 105, 29, V5_AUTO_SELL, font_);
        Label(L"Check", 128, 538, 42, 29, smallFont_);
        bagMinutes_ = Make(L"EDIT", L"5", WS_BORDER | ES_NUMBER | WS_TABSTOP,
                           170, 539, 45, 27, V5_BAG_MINUTES, font_, WS_EX_CLIENTEDGE);
        Label(L"phút", 220, 538, 38, 29, smallFont_);
        sellNpcCombo_ = Make(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                             263, 537, 425, 180, V5_SELL_NPC);
        Button(L"LƯU NPC BÁN ĐỒ GẦN NHẤT", 698, 537, 300, 30, V5_SAVE_NPC);

        healAtStartCheck_ = Make(L"BUTTON", L"Trị liệu khi ra bãi",
                                 BS_AUTOCHECKBOX | WS_TABSTOP,
                                 16, 572, 160, 28, V5_HEAL_AT_START, font_);
        healNpcCombo_ = Make(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                             184, 571, 504, 180, V5_HEAL_NPC);
        Button(L"LƯU NPC TRỊ LIỆU GẦN NHẤT", 698, 571, 300, 30, V5_SAVE_HEAL_NPC);

        autoBuffCheck_ = Make(L"BUTTON", L"Auto buff", BS_AUTOCHECKBOX | WS_TABSTOP,
                              16, 606, 95, 28, V5_AUTO_BUFF, font_);
        Label(L"Chọn nhiều skill buff (Ctrl/Shift):", 116, 604, 210, 25, smallFont_);
        buffSkillList_ = Make(L"LISTBOX", L"",
                              LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT | WS_VSCROLL | WS_TABSTOP,
                              330, 604, 600, 72, V5_BUFF_SKILLS, smallFont_, WS_EX_CLIENTEDGE);

        detailLabel_ = Label(L"Sẵn sàng.", 16, 684, 1028, 34, font_, V5_DETAIL);
        startButton_ = Button(L"▶  BẮT ĐẦU CỬA SỔ ĐÃ TICK",
                              16, 724, 646, 52, V5_START);
        stopButton_ = Button(L"■  TẠM DỪNG CỬA SỔ ĐÃ TICK",
                             675, 724, 369, 52, V5_STOP);

        buildingPage_ = 2;
        Make(L"STATIC", L"GIỚI THIỆU", SS_CENTER | SS_CENTERIMAGE,
             150, 145, 780, 46, 0, titleFont_);
        Make(L"STATIC", L"Phần mềm được thiết kế bởi Thắng Nguyễn - ĐỒ LONG",
             SS_CENTER | SS_CENTERIMAGE, 150, 205, 780, 48, 0, boldFont_);
        Make(L"STATIC", L"Phiên bản 0.9.0",
             SS_CENTER | SS_CENTERIMAGE, 150, 270, 780, 35, 0, smallFont_);
        buildingPage_ = 0;
        SelectPage(0);

        spots_ = LoadSpots();
        sellNpcs_ = LoadNpcsFrom(SellNpcsPath());
        if (sellNpcs_.empty()) sellNpcs_ = LoadNpcsFrom(LegacyNpcsPath());
        healNpcs_ = LoadNpcsFrom(HealNpcsPath());
        uiReady_ = true;
        RefreshGames();
        SetTimer(window_, 1, 500, nullptr);
    }

    GameRuntimeV5* Runtime(DWORD pid) {
        const auto found = runtimes_.find(pid);
        return found == runtimes_.end() ? nullptr : found->second.get();
    }

    GameRuntimeV5* SelectedRuntime() { return Runtime(selectedPid_); }

    DWORD PidAt(int index) const {
        if (index < 0 || index >= ListView_GetItemCount(gamesList_)) return 0;
        LVITEMW item{};
        item.mask = LVIF_PARAM;
        item.iItem = index;
        return ListView_GetItem(gamesList_, &item)
                   ? static_cast<DWORD>(item.lParam) : 0;
    }

    std::set<DWORD> CheckedPids() const {
        std::set<DWORD> result;
        const int count = ListView_GetItemCount(gamesList_);
        for (int i = 0; i < count; ++i) {
            if (ListView_GetCheckState(gamesList_, i)) {
                const DWORD pid = PidAt(i);
                if (pid) result.insert(pid);
            }
        }
        return result;
    }

    std::optional<int> CheckedSpotIndex() const {
        for (int i = 0; i < static_cast<int>(spots_.size()); ++i)
            if (ListView_GetCheckState(spotsList_, i)) return i;
        return std::nullopt;
    }

    std::wstring CheckedSpotName() const {
        const auto index = CheckedSpotIndex();
        return index ? spots_[static_cast<std::size_t>(*index)].name : L"";
    }

    const Spot* FindSpot(const std::wstring& name) const {
        const auto found = std::find_if(spots_.begin(), spots_.end(),
            [&](const Spot& spot) { return _wcsicmp(spot.name.c_str(), name.c_str()) == 0; });
        return found == spots_.end() ? nullptr : &*found;
    }

    static const SellNpc* FindNpcIn(const std::vector<SellNpc>& list,
                                      const std::wstring& name) {
        const auto found = std::find_if(list.begin(), list.end(),
            [&](const SellNpc& npc) { return _wcsicmp(npc.name.c_str(), name.c_str()) == 0; });
        return found == list.end() ? nullptr : &*found;
    }
    const SellNpc* FindSellNpc(const std::wstring& name) const {
        return FindNpcIn(sellNpcs_, name);
    }
    const SellNpc* FindHealNpc(const std::wstring& name) const {
        return FindNpcIn(healNpcs_, name);
    }

    void PopulateNpcCombo(HWND combo, const std::vector<SellNpc>& list,
                          const std::wstring& selectedName) {
        if (!combo) return;
        SendMessageW(combo, CB_RESETCONTENT, 0, 0);
        int selected = CB_ERR;
        for (std::size_t i = 0; i < list.size(); ++i) {
            const std::wstring label = list[i].name + L"  [ID " +
                std::to_wstring(list[i].roleID) + L" • Map " +
                std::to_wstring(list[i].mapID) + L" • " +
                std::to_wstring(list[i].x) + L"," + std::to_wstring(list[i].y) + L"]";
            const int index = static_cast<int>(SendMessageW(
                combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str())));
            SendMessageW(combo, CB_SETITEMDATA, index, static_cast<LPARAM>(i));
            if (!selectedName.empty() && _wcsicmp(list[i].name.c_str(), selectedName.c_str()) == 0)
                selected = index;
        }
        if (selected == CB_ERR && !list.empty()) selected = 0;
        SendMessageW(combo, CB_SETCURSEL, selected, 0);
    }
    void PopulateNpcs(const std::wstring& sellSelected = L"",
                      const std::wstring& healSelected = L"") {
        PopulateNpcCombo(sellNpcCombo_, sellNpcs_, sellSelected);
        PopulateNpcCombo(healNpcCombo_, healNpcs_, healSelected);
    }

    void ShowDetail(const std::wstring& text) {
        if (detailLabel_) SetWindowTextW(detailLabel_, text.c_str());
    }

    void ShowManualDetail(const std::wstring& text) {
        detailHoldUntil_ = Clock::now() + std::chrono::seconds(8);
        ShowDetail(text);
    }

    void PopulateSpots(const std::wstring& checkedName = L"") {
        changingSpotChecks_ = true;
        ListView_DeleteAllItems(spotsList_);
        for (std::size_t i = 0; i < spots_.size(); ++i) {
            LVITEMW item{};
            item.mask = LVIF_TEXT;
            item.iItem = static_cast<int>(i);
            item.pszText = spots_[i].name.data();
            ListView_InsertItem(spotsList_, &item);
            const std::wstring map = std::to_wstring(spots_[i].mapID);
            const std::wstring x = std::to_wstring(spots_[i].x);
            const std::wstring y = std::to_wstring(spots_[i].y);
            ListView_SetItemText(spotsList_, static_cast<int>(i), 1,
                                 const_cast<wchar_t*>(map.c_str()));
            ListView_SetItemText(spotsList_, static_cast<int>(i), 2,
                                 const_cast<wchar_t*>(x.c_str()));
            ListView_SetItemText(spotsList_, static_cast<int>(i), 3,
                                 const_cast<wchar_t*>(y.c_str()));
            ListView_SetCheckState(
                spotsList_, static_cast<int>(i),
                !checkedName.empty() &&
                    _wcsicmp(spots_[i].name.c_str(), checkedName.c_str()) == 0);
        }
        changingSpotChecks_ = false;
    }

    void PopulateSkills(GameRuntimeV5& runtime) {
        SendMessageW(skillCombo_, CB_RESETCONTENT, 0, 0);
        int selection = 0;
        if (runtime.skills.empty()) {
            const std::wstring label = runtime.config.skillID > 0
                ? L"Skill đã lưu • ID " + std::to_wstring(runtime.config.skillID)
                : L"Bấm ĐỌC SKILL để tải danh sách";
            const int index = static_cast<int>(SendMessageW(
                skillCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str())));
            SendMessageW(skillCombo_, CB_SETITEMDATA, index, runtime.config.skillID);
        } else {
            for (const auto& skill : runtime.skills) {
                const std::wstring label = skill.name + L"  [ID " +
                                           std::to_wstring(skill.id) + L"]";
                const int index = static_cast<int>(SendMessageW(
                    skillCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str())));
                SendMessageW(skillCombo_, CB_SETITEMDATA, index, skill.id);
                if (skill.id == runtime.config.skillID) selection = index;
            }
        }
        SendMessageW(skillCombo_, CB_SETCURSEL, selection, 0);
        PopulateBuffSkills(runtime);
    }

    void PopulateBuffSkills(GameRuntimeV5& runtime) {
        if (!buffSkillList_) return;
        SendMessageW(buffSkillList_, LB_RESETCONTENT, 0, 0);
        std::set<int> selected(runtime.config.buffSkillIDs.begin(), runtime.config.buffSkillIDs.end());
        if (runtime.skills.empty()) {
            for (int id : runtime.config.buffSkillIDs) {
                const std::wstring label = L"Buff skill đã lưu [ID " + std::to_wstring(id) + L"]";
                const int row = static_cast<int>(SendMessageW(buffSkillList_, LB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(label.c_str())));
                SendMessageW(buffSkillList_, LB_SETITEMDATA, row, id);
                SendMessageW(buffSkillList_, LB_SETSEL, TRUE, row);
            }
            return;
        }
        for (const SkillOption& skill : runtime.skills) {
            const std::wstring label = skill.name + L"  [ID " + std::to_wstring(skill.id) + L"]";
            const int row = static_cast<int>(SendMessageW(buffSkillList_, LB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(label.c_str())));
            SendMessageW(buffSkillList_, LB_SETITEMDATA, row, skill.id);
            if (selected.count(skill.id)) SendMessageW(buffSkillList_, LB_SETSEL, TRUE, row);
        }
    }

    void UpdateState(const LiveState& live) {
        const bool preserve = live.phase == L"Lỗi" || live.phase == L"Game đã đóng" ||
                              live.phase == L"Mất kết nối" ||
                              live.phase == L"TẠM DỪNG AN TOÀN";
        SetWindowTextW(stateLabel_, (live.running || preserve || live.phase == L"Sẵn sàng")
                                        ? live.phase.c_str() : L"ĐÃ DỪNG");
        const std::wstring coords = live.mapID > 0
            ? L"Map " + std::to_wstring(live.mapID) + L" • X " +
                  std::to_wstring(live.x) + L" • Y " + std::to_wstring(live.y)
            : L"Map -- • X -- • Y --";
        SetWindowTextW(coordsLabel_, coords.c_str());
        const std::wstring flags = !live.connected && live.mapID <= 0
            ? L"Ngựa: -- • Di chuyển: -- • Chết: -- • Map: -- • Túi đồ trống: -- ô"
            : L"Ngựa: " + std::wstring(live.riding ? L"CÓ" : L"KHÔNG") +
                  L" • Di chuyển: " + (live.moving ? L"CÓ" : L"KHÔNG") +
                  L" • Chết: " + (live.dead ? L"CÓ" : L"KHÔNG") +
                  L" • Map: " + (live.waitingChangeMap || !live.mapReady
                                      ? L"ĐANG NẠP" : L"SẴN SÀNG") +
                  L" • Túi đồ trống: " + (live.freeBagSpace >= 0
                                      ? std::to_wstring(live.freeBagSpace) : L"--") + L" ô";
        SetWindowTextW(flagsLabel_, flags.c_str());
        if (!live.detail.empty() && Clock::now() >= detailHoldUntil_) ShowDetail(live.detail);
    }

    bool ApplyIdentity(GameRuntimeV5& runtime, const LiveState& live,
                       bool loadProfile) {
        if (live.roleID <= 0) return false;
        runtime.game.roleID = live.roleID;
        if (!live.characterName.empty()) runtime.game.characterName = live.characterName;
        const std::wstring section = ProfileSection(live.roleID, runtime.game.pid);
        if (section == runtime.profileSection) return false;
        if (!loadProfile) return false;
        runtime.profileSection = section;
        runtime.config = LoadConfig(section);
        runtime.selectedSpot = IniText(section, L"SpotName");
        runtime.skills.clear();
        return true;
    }

    TrainConfig ReadConfigUi(GameRuntimeV5& runtime) {
        runtime.config.tolerance = std::clamp(
            _wtoi(Text(tolerance_).c_str()), 20, 2000);
        runtime.config.retrySeconds = std::clamp(
            _wtoi(Text(retry_).c_str()), 5, 60);
        const int activationIndex = static_cast<int>(SendMessageW(
            activationCombo_, CB_GETCURSEL, 0, 0));
        const LRESULT activationData = activationIndex == CB_ERR ? CB_ERR
            : SendMessageW(activationCombo_, CB_GETITEMDATA, activationIndex, 0);
        runtime.config.activation =
            activationData == static_cast<LRESULT>(TrainActivationMode::CtrlTabSkill)
                ? TrainActivationMode::CtrlTabSkill : TrainActivationMode::AutoFight;
        const int skillIndex = static_cast<int>(SendMessageW(
            skillCombo_, CB_GETCURSEL, 0, 0));
        const LRESULT skillData = skillIndex == CB_ERR ? 0
            : SendMessageW(skillCombo_, CB_GETITEMDATA, skillIndex, 0);
        runtime.config.skillID = skillData == CB_ERR
            ? 0 : std::max(static_cast<int>(skillData), 0);
        runtime.config.autoSell = SendMessageW(autoSellCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        runtime.config.bagCheckMinutes = std::clamp(_wtoi(Text(bagMinutes_).c_str()), 1, 180);
        const int npcIndex = static_cast<int>(SendMessageW(sellNpcCombo_, CB_GETCURSEL, 0, 0));
        runtime.config.sellNpcName.clear();
        if (npcIndex != CB_ERR) {
            const LRESULT data = SendMessageW(sellNpcCombo_, CB_GETITEMDATA, npcIndex, 0);
            if (data != CB_ERR && data >= 0 && static_cast<std::size_t>(data) < sellNpcs_.size())
                runtime.config.sellNpcName = sellNpcs_[static_cast<std::size_t>(data)].name;
        }
        runtime.config.healAtStart = SendMessageW(healAtStartCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        runtime.config.healNpcName.clear();
        const int healIndex = static_cast<int>(SendMessageW(healNpcCombo_, CB_GETCURSEL, 0, 0));
        if (healIndex != CB_ERR) {
            const LRESULT data = SendMessageW(healNpcCombo_, CB_GETITEMDATA, healIndex, 0);
            if (data != CB_ERR && data >= 0 && static_cast<std::size_t>(data) < healNpcs_.size())
                runtime.config.healNpcName = healNpcs_[static_cast<std::size_t>(data)].name;
        }
        runtime.config.autoBuff = SendMessageW(autoBuffCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        runtime.config.buffSkillIDs.clear();
        const int buffRows = static_cast<int>(SendMessageW(buffSkillList_, LB_GETCOUNT, 0, 0));
        for (int row = 0; row < buffRows; ++row) {
            if (SendMessageW(buffSkillList_, LB_GETSEL, row, 0) <= 0) continue;
            const LRESULT id = SendMessageW(buffSkillList_, LB_GETITEMDATA, row, 0);
            if (id != LB_ERR && id > 0) runtime.config.buffSkillIDs.push_back(static_cast<int>(id));
        }
        SetWindowTextW(bagMinutes_, std::to_wstring(runtime.config.bagCheckMinutes).c_str());
        SetWindowTextW(tolerance_, std::to_wstring(runtime.config.tolerance).c_str());
        SetWindowTextW(retry_, std::to_wstring(runtime.config.retrySeconds).c_str());
        return runtime.config;
    }

    void SaveRuntime(GameRuntimeV5& runtime) {
        if (runtime.profileSection.empty()) {
            runtime.profileSection = ProfileSection(runtime.game.roleID, runtime.game.pid);
        }
        SaveConfig(runtime.profileSection, runtime.config, runtime.selectedSpot);
    }

    void SaveSelectedUi() {
        if (!uiReady_ || loadingUi_) return;
        GameRuntimeV5* runtime = SelectedRuntime();
        if (!runtime) return;
        ReadConfigUi(*runtime);
        runtime->selectedSpot = CheckedSpotName();
        SaveRuntime(*runtime);
    }

    void LoadSelectedUi() {
        GameRuntimeV5* runtime = SelectedRuntime();
        loadingUi_ = true;
        if (!runtime) {
            PopulateSpots();
            PopulateNpcs();
            SetWindowTextW(spotName_, L"");
            SendMessageW(autoSellCheck_, BM_SETCHECK, BST_UNCHECKED, 0);
            SendMessageW(healAtStartCheck_, BM_SETCHECK, BST_UNCHECKED, 0);
            SendMessageW(autoBuffCheck_, BM_SETCHECK, BST_UNCHECKED, 0);
            SendMessageW(buffSkillList_, LB_RESETCONTENT, 0, 0);
            UpdateState(LiveState{});
            loadingUi_ = false;
            return;
        }
        int activationSelection = 0;
        const int activationCount = static_cast<int>(
            SendMessageW(activationCombo_, CB_GETCOUNT, 0, 0));
        for (int i = 0; i < activationCount; ++i) {
            if (SendMessageW(activationCombo_, CB_GETITEMDATA, i, 0) ==
                static_cast<LRESULT>(runtime->config.activation)) {
                activationSelection = i;
                break;
            }
        }
        SendMessageW(activationCombo_, CB_SETCURSEL, activationSelection, 0);
        SetWindowTextW(tolerance_, std::to_wstring(runtime->config.tolerance).c_str());
        SetWindowTextW(retry_, std::to_wstring(runtime->config.retrySeconds).c_str());
        SetWindowTextW(bagMinutes_, std::to_wstring(runtime->config.bagCheckMinutes).c_str());
        SendMessageW(autoSellCheck_, BM_SETCHECK,
                     runtime->config.autoSell ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(healAtStartCheck_, BM_SETCHECK,
                     runtime->config.healAtStart ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(autoBuffCheck_, BM_SETCHECK,
                     runtime->config.autoBuff ? BST_CHECKED : BST_UNCHECKED, 0);
        SetWindowTextW(spotName_, runtime->selectedSpot.c_str());
        PopulateSkills(*runtime);
        PopulateSpots(runtime->selectedSpot);
        PopulateNpcs(runtime->config.sellNpcName, runtime->config.healNpcName);
        UpdateState(runtime->last);
        loadingUi_ = false;
    }

    void SetGameCell(int row, int column, const std::wstring& value) {
        ListView_SetItemText(gamesList_, row, column,
                             const_cast<wchar_t*>(value.c_str()));
    }

    void UpdateGameRow(int row, GameRuntimeV5& runtime) {
        const LiveState& live = runtime.last;
        const std::wstring character = runtime.game.characterName.empty()
            ? L"(chưa đọc được nhân vật)"
            : runtime.game.characterName +
                  (runtime.game.roleID > 0
                       ? L"  [" + std::to_wstring(runtime.game.roleID) + L"]" : L"");
        SetGameCell(row, 1, character);
        SetGameCell(row, 2, live.phase.empty() ? L"Đã dừng" : live.phase);
        SetGameCell(row, 3, live.mapID > 0 ? std::to_wstring(live.mapID) : L"--");
        SetGameCell(row, 4, live.mapID > 0
            ? L"X " + std::to_wstring(live.x) + L"  •  Y " + std::to_wstring(live.y)
            : L"--");
    }

    void RebuildGameList(const std::set<DWORD>& checked, bool checkFirst) {
        changingGameList_ = true;
        ListView_DeleteAllItems(gamesList_);
        for (std::size_t i = 0; i < gameOrder_.size(); ++i) {
            GameRuntimeV5* runtime = Runtime(gameOrder_[i]);
            if (!runtime) continue;
            const std::wstring pid = std::to_wstring(runtime->game.pid);
            LVITEMW item{};
            item.mask = LVIF_TEXT | LVIF_PARAM;
            item.iItem = static_cast<int>(i);
            item.pszText = const_cast<wchar_t*>(pid.c_str());
            item.lParam = static_cast<LPARAM>(runtime->game.pid);
            ListView_InsertItem(gamesList_, &item);
            UpdateGameRow(static_cast<int>(i), *runtime);
            const bool shouldCheck = checked.count(runtime->game.pid) != 0 ||
                                     (checkFirst && i == 0);
            ListView_SetCheckState(gamesList_, static_cast<int>(i), shouldCheck);
            if (runtime->game.pid == selectedPid_) {
                ListView_SetItemState(gamesList_, static_cast<int>(i),
                                      LVIS_SELECTED | LVIS_FOCUSED,
                                      LVIS_SELECTED | LVIS_FOCUSED);
            }
        }
        changingGameList_ = false;
        LoadSelectedUi();
    }

    void RefreshGames() {
        SaveSelectedUi();
        const std::set<DWORD> checked = CheckedPids();
        const bool firstScan = gameOrder_.empty();
        const DWORD oldSelected = selectedPid_;
        ShowDetail(L"Đang quét tiến trình và đọc tên từng nhân vật...");
        const std::vector<GameProcess> found = FindGames();
        std::set<DWORD> livePids;
        for (GameProcess game : found) {
            livePids.insert(game.pid);
            auto existing = runtimes_.find(game.pid);
            if (existing == runtimes_.end()) {
                auto runtime = std::make_unique<GameRuntimeV5>(game);
                runtime->profileSection = ProfileSection(0, game.pid);
                runtime->config = LoadConfig(runtime->profileSection);
                runtime->selectedSpot = IniText(runtime->profileSection, L"SpotName");
                existing = runtimes_.emplace(game.pid, std::move(runtime)).first;
            } else {
                existing->second->game.window = game.window;
                existing->second->game.title = game.title;
            }
            GameRuntimeV5& runtime = *existing->second;
            LiveState live;
            std::wstring error;
            if (runtime.session->Running()) {
                live = runtime.session->State();
                ApplyIdentity(runtime, live, false);
            } else if (runtime.session->Probe(runtime.game, live, error)) {
                ApplyIdentity(runtime, live, true);
            } else {
                live.connected = false;
                live.phase = L"Chưa sẵn sàng";
                live.detail = error;
            }
            runtime.last = live;
        }
        std::vector<DWORD> stale;
        for (const auto& entry : runtimes_)
            if (!livePids.count(entry.first)) stale.push_back(entry.first);
        for (DWORD pid : stale) {
            runtimes_[pid]->session->Stop();
            runtimes_.erase(pid);
        }
        gameOrder_.assign(livePids.begin(), livePids.end());
        if (oldSelected && livePids.count(oldSelected)) selectedPid_ = oldSelected;
        else selectedPid_ = gameOrder_.empty() ? 0 : gameOrder_.front();
        RebuildGameList(checked, firstScan);
        ShowDetail(gameOrder_.empty()
            ? L"Không thấy game. Hãy mở game rồi quét lại."
            : L"Đã tìm thấy " + std::to_wstring(gameOrder_.size()) +
                  L" cửa sổ; chọn một dòng để chỉnh setting riêng.");
    }

    bool ProbeSelected(LiveState& output) {
        SaveSelectedUi();
        GameRuntimeV5* runtime = SelectedRuntime();
        if (!runtime) {
            ShowDetail(L"Chưa chọn cửa sổ game.");
            return false;
        }
        std::wstring error;
        if (runtime->session->Running()) {
            output = runtime->session->State();
        } else if (!runtime->session->Probe(runtime->game, output, error)) {
            ShowDetail(L"Không đọc được vị trí: " + error);
            return false;
        }
        const bool changed = ApplyIdentity(*runtime, output, true);
        runtime->last = output;
        if (changed) LoadSelectedUi();
        UpdateState(output);
        for (int i = 0; i < static_cast<int>(gameOrder_.size()); ++i)
            if (gameOrder_[static_cast<std::size_t>(i)] == runtime->game.pid)
                UpdateGameRow(i, *runtime);
        return true;
    }

    void SaveCurrentSpot() {
        const std::wstring requestedName = Trim(Text(spotName_));
        LiveState live;
        if (!ProbeSelected(live)) return;
        GameRuntimeV5* runtime = SelectedRuntime();
        if (!runtime || live.mapID <= 0) {
            ShowDetail(L"Chưa có MapID/X/Y hợp lệ để lưu.");
            return;
        }
        std::wstring name = requestedName;
        if (name.empty()) {
            name = L"Bãi Map " + std::to_wstring(live.mapID) + L" - " +
                   std::to_wstring(live.x) + L"," + std::to_wstring(live.y);
        }
        Spot value{name, live.mapID, live.x, live.y};
        const auto found = std::find_if(spots_.begin(), spots_.end(), [&](const Spot& item) {
            return _wcsicmp(item.name.c_str(), name.c_str()) == 0;
        });
        if (found == spots_.end()) spots_.push_back(value);
        else *found = value;
        if (!SaveSpots(spots_)) {
            MessageBoxW(window_, L"Không ghi được file ThanLongAutoTrain.spots.txt cạnh EXE.",
                        kTitle, MB_OK | MB_ICONERROR);
            return;
        }
        runtime->selectedSpot = name;
        SaveRuntime(*runtime);
        SetWindowTextW(spotName_, name.c_str());
        PopulateSpots(name);
        ShowDetail(L"Đã lưu “" + name + L"” vào file TXT dùng chung cho mọi cửa sổ.");
    }

    void DeleteSpot() {
        int index = ListView_GetNextItem(spotsList_, -1, LVNI_SELECTED);
        if (index < 0) {
            const auto checked = CheckedSpotIndex();
            if (checked) index = *checked;
        }
        if (index < 0 || index >= static_cast<int>(spots_.size())) {
            ShowDetail(L"Hãy chọn một bãi cần xóa.");
            return;
        }
        const std::wstring removed = spots_[static_cast<std::size_t>(index)].name;
        if (MessageBoxW(window_, (L"Xóa bãi “" + removed + L"”? ").c_str(),
                        kTitle, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return;
        spots_.erase(spots_.begin() + index);
        if (!SaveSpots(spots_)) {
            MessageBoxW(window_, L"Không cập nhật được file danh sách bãi.", kTitle,
                        MB_OK | MB_ICONERROR);
            return;
        }
        for (auto& entry : runtimes_) {
            if (_wcsicmp(entry.second->selectedSpot.c_str(), removed.c_str()) == 0) {
                entry.second->selectedSpot.clear();
                SaveRuntime(*entry.second);
            }
        }
        PopulateSpots(SelectedRuntime() ? SelectedRuntime()->selectedSpot : L"");
        ShowDetail(L"Đã xóa bãi khỏi danh sách TXT dùng chung.");
    }



    void ProbeSkillsSelected() {
        SaveSelectedUi();
        GameRuntimeV5* runtime = SelectedRuntime();
        if (!runtime) {
            ShowDetail(L"Chưa chọn cửa sổ game để đọc skill.");
            return;
        }
        ShowDetail(L"Đang đọc toàn bộ skill của nhân vật đang chọn...");
        std::vector<SkillOption> output;
        std::wstring error;
        if (!runtime->session->ProbeSkills(runtime->game, output, error)) {
            ShowDetail(L"Không đọc được skill: " + error);
            return;
        }
        runtime->skills = std::move(output);
        PopulateSkills(*runtime);
        ShowDetail(L"Đã đọc " + std::to_wstring(runtime->skills.size()) +
                   L" skill thật của " +
                   (runtime->game.characterName.empty() ? L"nhân vật" : runtime->game.characterName) +
                   L".");
    }

    void SaveNearestNpcTo(bool treatment) {
        SaveSelectedUi();
        GameRuntimeV5* runtime = SelectedRuntime();
        if (!runtime) {
            ShowDetail(L"Chưa chọn cửa sổ game để lưu NPC.");
            return;
        }
        SellNpc npc;
        std::wstring error;
        if (!runtime->session->ProbeNearestNpc(runtime->game, npc, error)) {
            ShowManualDetail(L"Không lưu NPC: " + error);
            return;
        }

        std::vector<SellNpc>& list = treatment ? healNpcs_ : sellNpcs_;
        const auto found = std::find_if(list.begin(), list.end(), [&](const SellNpc& item) {
            return item.roleID == npc.roleID || _wcsicmp(item.name.c_str(), npc.name.c_str()) == 0;
        });
        if (found == list.end()) list.push_back(npc);
        else *found = npc;

        const std::wstring path = treatment ? HealNpcsPath() : SellNpcsPath();
        if (!SaveNpcsTo(path, list, treatment ? L"NPC tri lieu" : L"NPC ban do")) {
            ShowManualDetail(std::wstring(L"Không ghi được file NPC ") +
                             (treatment ? L"trị liệu." : L"bán đồ."));
            return;
        }

        if (treatment) runtime->config.healNpcName = npc.name;
        else runtime->config.sellNpcName = npc.name;
        SaveRuntime(*runtime);
        PopulateNpcs(runtime->config.sellNpcName, runtime->config.healNpcName);
        ShowManualDetail(std::wstring(L"Đã lưu riêng NPC ") +
                         (treatment ? L"trị liệu: " : L"bán đồ: ") +
                         npc.name + L" • RoleID " + std::to_wstring(npc.roleID) +
                         L" • ResID " + std::to_wstring(npc.resID) + L" • Map " +
                         std::to_wstring(npc.mapID) + L" • " + std::to_wstring(npc.x) + L"," +
                         std::to_wstring(npc.y));
    }
    void SaveNearestSellNpc() { SaveNearestNpcTo(false); }
    void SaveNearestHealNpc() { SaveNearestNpcTo(true); }

    bool ValidateRuntime(GameRuntimeV5& runtime, std::wstring& error) {
        if (!FindSpot(runtime.selectedSpot)) {
            error = L"PID " + std::to_wstring(runtime.game.pid) + L": chưa chọn bãi";
            return false;
        }
        if (runtime.config.activation == TrainActivationMode::CtrlTabSkill &&
            runtime.config.skillID <= 0) {
            error = L"PID " + std::to_wstring(runtime.game.pid) + L": chưa chọn skill";
            return false;
        }
        if (runtime.config.autoSell && !FindSellNpc(runtime.config.sellNpcName)) {
            error = L"PID " + std::to_wstring(runtime.game.pid) +
                    L": đã bật Tự bán đồ nhưng chưa chọn/lưu NPC bán đồ";
            return false;
        }
        if (runtime.config.healAtStart && !FindHealNpc(runtime.config.healNpcName)) {
            error = L"PID " + std::to_wstring(runtime.game.pid) +
                    L": đã bật Trị liệu nhưng chưa chọn/lưu NPC trị liệu";
            return false;
        }
        if (runtime.config.autoBuff && runtime.config.buffSkillIDs.empty()) {
            error = L"PID " + std::to_wstring(runtime.game.pid) + L": Auto buff chưa chọn skill";
            return false;
        }
        return true;
    }

    void StartChecked() {
        SaveSelectedUi();
        const std::set<DWORD> checked = CheckedPids();
        if (checked.empty()) {
            ShowDetail(L"Hãy tick ít nhất một cửa sổ game cần bắt đầu.");
            return;
        }
        int started = 0;
        std::wstring failures;
        for (DWORD pid : checked) {
            GameRuntimeV5* runtime = Runtime(pid);
            if (!runtime) continue;
            std::wstring error;
            if (!ValidateRuntime(*runtime, error)) {
                if (!failures.empty()) failures += L" • ";
                failures += error;
                continue;
            }
            const Spot* spot = FindSpot(runtime->selectedSpot);
            if (!spot) continue;
            const SellNpc* npc = runtime->config.autoSell
                ? FindSellNpc(runtime->config.sellNpcName) : nullptr;
            const SellNpc* healNpc = runtime->config.healAtStart
                ? FindHealNpc(runtime->config.healNpcName) : nullptr;
            runtime->session->Start(runtime->game, *spot, runtime->config,
                                    npc ? *npc : SellNpc{}, healNpc ? *healNpc : SellNpc{});
            runtime->last = runtime->session->State();
            ++started;
        }
        std::wstring detail = L"Đã bắt đầu độc lập " + std::to_wstring(started) +
                              L" cửa sổ đã tick.";
        if (!failures.empty()) detail += L" Chưa chạy: " + failures;
        ShowDetail(detail);
    }

    void StopChecked() {
        const std::set<DWORD> checked = CheckedPids();
        if (checked.empty()) {
            ShowDetail(L"Hãy tick ít nhất một cửa sổ cần tạm dừng.");
            return;
        }
        int stopped = 0;
        for (DWORD pid : checked) {
            GameRuntimeV5* runtime = Runtime(pid);
            if (!runtime) continue;
            runtime->session->Stop();
            runtime->last = runtime->session->State();
            ++stopped;
        }
        UpdateAllStates();
        ShowDetail(L"Đã tạm dừng đúng " + std::to_wstring(stopped) +
                   L" cửa sổ được tick; các cửa sổ khác giữ nguyên.");
    }


    void OnGameItemChanged(const NMLISTVIEW& changed) {
        if (changingGameList_ || changed.iItem < 0 || !(changed.uChanged & LVIF_STATE)) return;
        const bool selectedNow = (changed.uNewState & LVIS_SELECTED) != 0;
        const bool selectedBefore = (changed.uOldState & LVIS_SELECTED) != 0;
        if (!selectedNow || selectedBefore) return;
        const DWORD pid = PidAt(changed.iItem);
        if (!pid || pid == selectedPid_) return;
        SaveSelectedUi();
        selectedPid_ = pid;
        LoadSelectedUi();
        GameRuntimeV5* runtime = SelectedRuntime();
        if (runtime) {
            ShowDetail(L"Đang chỉnh riêng cho " +
                       (runtime->game.characterName.empty()
                            ? L"PID " + std::to_wstring(pid)
                            : runtime->game.characterName) + L".");
        }
    }

    void OnSpotItemChanged(const NMLISTVIEW& changed) {
        if (changingSpotChecks_ || loadingUi_ || changed.iItem < 0 ||
            !(changed.uChanged & LVIF_STATE)) return;
        const bool nowChecked = ((changed.uNewState & LVIS_STATEIMAGEMASK) >> 12) == 2;
        const bool wasChecked = ((changed.uOldState & LVIS_STATEIMAGEMASK) >> 12) == 2;
        if (nowChecked && !wasChecked) {
            changingSpotChecks_ = true;
            for (int i = 0; i < static_cast<int>(spots_.size()); ++i)
                if (i != changed.iItem) ListView_SetCheckState(spotsList_, i, FALSE);
            changingSpotChecks_ = false;
        }
        GameRuntimeV5* runtime = SelectedRuntime();
        if (runtime) {
            runtime->selectedSpot = CheckedSpotName();
            SetWindowTextW(spotName_, runtime->selectedSpot.c_str());
            SaveSelectedUi();
        }
    }

    void UpdateAllStates() {
        for (std::size_t i = 0; i < gameOrder_.size(); ++i) {
            GameRuntimeV5* runtime = Runtime(gameOrder_[i]);
            if (!runtime) continue;
            runtime->last = runtime->session->State();
            if (runtime->last.roleID > 0) ApplyIdentity(*runtime, runtime->last, false);
            UpdateGameRow(static_cast<int>(i), *runtime);
        }
        GameRuntimeV5* selected = SelectedRuntime();
        if (selected) UpdateState(selected->last);
    }

    void Destroy() {
        if (destroyed_) return;
        destroyed_ = true;
        SaveSelectedUi();
        KillTimer(window_, 1);
        for (auto& entry : runtimes_) entry.second->session->Stop();
        for (HFONT font : {font_, smallFont_, boldFont_, buttonFont_, titleFont_})
            if (font) DeleteObject(font);
    }

    LRESULT Handle(UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_CREATE:
                CreateUi();
                return 0;
            case WM_COMMAND:
                switch (LOWORD(wParam)) {
                    case V5_REFRESH: RefreshGames(); return 0;
                    case V5_PROBE: { LiveState state; ProbeSelected(state); return 0; }
                    case V5_SAVE_SPOT: SaveCurrentSpot(); return 0;
                    case V5_DELETE_SPOT: DeleteSpot(); return 0;
                    case V5_SCAN_SKILLS: ProbeSkillsSelected(); return 0;
                    case V5_SAVE_NPC: SaveNearestSellNpc(); return 0;
                    case V5_SAVE_HEAL_NPC: SaveNearestHealNpc(); return 0;
                    case V5_START: StartChecked(); return 0;
                    case V5_STOP: StopChecked(); return 0;
                }
                break;
            case WM_NOTIFY: {
                const auto* header = reinterpret_cast<NMHDR*>(lParam);
                if (!header) break;
                if (header->code == LVN_ITEMCHANGED && header->hwndFrom == gamesList_)
                    OnGameItemChanged(*reinterpret_cast<NMLISTVIEW*>(lParam));
                else if (header->code == LVN_ITEMCHANGED && header->hwndFrom == spotsList_)
                    OnSpotItemChanged(*reinterpret_cast<NMLISTVIEW*>(lParam));
                else if (header->code == TCN_SELCHANGE && header->hwndFrom == tab_)
                    SelectPage(TabCtrl_GetCurSel(tab_));
                break;
            }
            case WM_TIMER:
                UpdateAllStates();
                return 0;
            case WM_CLOSE:
                DestroyWindow(window_);
                return 0;
            case WM_DESTROY:
                Destroy();
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(window_, message, wParam, lParam);
    }

    HINSTANCE instance_ = nullptr;
    HWND window_ = nullptr;
    HWND tab_ = nullptr;
    HWND gamesList_ = nullptr;
    HWND stateLabel_ = nullptr;
    HWND coordsLabel_ = nullptr;
    HWND flagsLabel_ = nullptr;
    HWND spotName_ = nullptr;
    HWND spotsList_ = nullptr;
    HWND activationCombo_ = nullptr;
    HWND skillCombo_ = nullptr;
    HWND tolerance_ = nullptr;
    HWND retry_ = nullptr;
    HWND autoSellCheck_ = nullptr;
    HWND bagMinutes_ = nullptr;
    HWND sellNpcCombo_ = nullptr;
    HWND healAtStartCheck_ = nullptr;
    HWND healNpcCombo_ = nullptr;
    HWND autoBuffCheck_ = nullptr;
    HWND buffSkillList_ = nullptr;
    HWND detailLabel_ = nullptr;
    HWND startButton_ = nullptr;
    HWND stopButton_ = nullptr;
    HFONT font_ = nullptr;
    HFONT smallFont_ = nullptr;
    HFONT boldFont_ = nullptr;
    HFONT buttonFont_ = nullptr;
    HFONT titleFont_ = nullptr;
    std::map<DWORD, std::unique_ptr<GameRuntimeV5>> runtimes_;
    std::vector<DWORD> gameOrder_;
    std::vector<Spot> spots_;
    std::vector<SellNpc> sellNpcs_;
    std::vector<SellNpc> healNpcs_;
    std::vector<HWND> mainPageControls_;
    std::vector<HWND> aboutPageControls_;
    DWORD selectedPid_ = 0;
    int buildingPage_ = 0;
    Clock::time_point detailHoldUntil_{};
    bool uiReady_ = false;
    bool loadingUi_ = false;
    bool changingGameList_ = false;
    bool changingSpotChecks_ = false;
    bool destroyed_ = false;
};

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    Application app;
    return app.Run(instance, show);
}
