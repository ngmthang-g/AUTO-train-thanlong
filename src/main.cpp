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
constexpr wchar_t kTitle[] = L"Thần Long Mobile - Auto Train v0.8.2";
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
constexpr std::uint64_t LuaGetBuffs = 0x66F020;
constexpr std::uint64_t LuaHasBuff = 0x677610;
constexpr std::uint64_t LuaRequestUsingSkill = 0x6791C0;
constexpr std::uint64_t LuaBuffGetBuffID = 0x41F000;
constexpr std::uint64_t LuaBuffGetDurationTick = 0x41F3F0;
constexpr std::uint64_t UIInputSetText = 0x63C270;
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
constexpr std::uint64_t LuaGetNearbyEnemyIDs = 0x673620;
constexpr std::uint64_t LuaSelectTarget = 0x6791D0;
constexpr std::uint64_t LuaGetSelectedTarget = 0x67B610;
constexpr std::uint64_t SelectedTargetGetRoleID = 0x526830;
constexpr std::uint64_t LuaIsSelectTargetDie = 0x678260;
constexpr std::uint64_t LuaIsMapReady = 0x677E60;
constexpr std::uint64_t LuaMainFindUI = 0x6A5F90;
constexpr std::uint64_t UIObjectGetName = 0x530240;
constexpr std::uint64_t UIObjectGetParent = 0x530270;
constexpr std::uint64_t UIObjectCoreParents = 0x52FF10;
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
constexpr std::uint64_t UIRectGetPointerClickHandler = 0x644B50;
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
        case L'Đ': case L'đ': return L'd';
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

struct NormalizedPoint {
    int x = -1;
    int y = -1;
    bool Valid() const { return x >= 0 && x <= 10000 && y >= 0 && y <= 10000; }
};

enum class NavigationMode { DirectAutoPath = 0, ChatPing = 1 };
enum class TrainActivationMode { SelectedSkill = 1, AutoMenu = 2 };

struct TrainConfig {
    NavigationMode mode = NavigationMode::DirectAutoPath;
    TrainActivationMode activation = TrainActivationMode::AutoMenu;
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
    bool autoChat = false;
    int autoChatMinutes = 5;
    std::wstring autoChatMessage;
    NormalizedPoint chatClear;
    NormalizedPoint chatInput;
    NormalizedPoint chatLatestCoordinate;
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
    c.mode = IniInt(section, L"Mode", 0) == 1
                 ? NavigationMode::ChatPing : NavigationMode::DirectAutoPath;
    const int rawActivation = IniInt(section, L"TrainActivation", 2);
    c.activation = rawActivation == static_cast<int>(TrainActivationMode::SelectedSkill)
        ? TrainActivationMode::SelectedSkill : TrainActivationMode::AutoMenu;
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
    c.autoChat = IniInt(section, L"AutoChat", 0) != 0;
    c.autoChatMinutes = std::clamp(IniInt(section, L"AutoChatMinutes", 5), 1, 180);
    c.autoChatMessage = IniText(section, L"AutoChatMessage");
    auto point = [&](const wchar_t* x, const wchar_t* y) {
        return NormalizedPoint{IniInt(section, x, -1), IniInt(section, y, -1)};
    };
    c.chatClear = point(L"ChatClearX", L"ChatClearY");
    c.chatInput = point(L"ChatInputX", L"ChatInputY");
    c.chatLatestCoordinate = point(L"ChatLatestX", L"ChatLatestY");
    return c;
}

static void SaveConfig(const std::wstring& section, const TrainConfig& c,
                       const std::wstring& spotName = L"") {
    WriteIniInt(section, L"Mode", c.mode == NavigationMode::ChatPing ? 1 : 0);
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
    WriteIniInt(section, L"AutoChat", c.autoChat ? 1 : 0);
    WriteIniInt(section, L"AutoChatMinutes", c.autoChatMinutes);
    WriteIniText(section, L"AutoChatMessage", c.autoChatMessage);
    auto point = [&](const wchar_t* x, const wchar_t* y, const NormalizedPoint& p) {
        WriteIniInt(section, x, p.x); WriteIniInt(section, y, p.y);
    };
    point(L"ChatClearX", L"ChatClearY", c.chatClear);
    point(L"ChatInputX", L"ChatInputY", c.chatInput);
    point(L"ChatLatestX", L"ChatLatestY", c.chatLatestCoordinate);
    WriteIniText(section, L"SpotName", spotName);
}

static bool ActivateGame(HWND game) {
    if (!game || !IsWindow(game)) return false;
    if (IsIconic(game)) ShowWindow(game, SW_RESTORE);
    BringWindowToTop(game);
    SetForegroundWindow(game);
    Sleep(120);
    return GetForegroundWindow() == game || IsChild(game, GetForegroundWindow());
}

static bool ClientPoint(HWND window, const NormalizedPoint& point, POINT& client) {
    if (!point.Valid() || !window || !IsWindow(window)) return false;
    RECT rect{};
    if (!GetClientRect(window, &rect) || rect.right <= 0 || rect.bottom <= 0) return false;
    client.x = static_cast<LONG>((static_cast<long long>(rect.right) * point.x) / 10000);
    client.y = static_cast<LONG>((static_cast<long long>(rect.bottom) * point.y) / 10000);
    return true;
}

static bool ClickGamePoint(HWND game, const NormalizedPoint& point) {
    POINT client{};
    if (!ClientPoint(game, point, client)) return false;
    const LPARAM position = MAKELPARAM(static_cast<WORD>(client.x),
                                      static_cast<WORD>(client.y));
    // Send directly to the selected Unity window. Do not restore it, move the
    // real cursor or steal the foreground window from the user.
    const bool moved = PostMessageW(game, WM_MOUSEMOVE, 0, position) != FALSE;
    const bool down = PostMessageW(game, WM_LBUTTONDOWN, MK_LBUTTON, position) != FALSE;
    const bool up = PostMessageW(game, WM_LBUTTONUP, 0, position) != FALSE;
    return moved && down && up;
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

static bool TypeUnicode(HWND game, const std::wstring& text) {
    if (!game || !IsWindow(game)) return false;
    bool ok = true;
    for (wchar_t c : text) {
        ok = (PostMessageW(game, WM_CHAR, static_cast<WPARAM>(c), 1) != FALSE) && ok;
    }
    return ok;
}

static int ChatCoordinate(int x, int y, bool returnX) {
    const int value = returnX ? x : y;
    // RoleData uses fixed-point coordinates on the supplied game build
    // (for example 24800,6000 while chat displays 248,60). Keep a fallback
    // for an already-unscaled coordinate list.
    if (std::abs(x) > 1000 || std::abs(y) > 1000) return value / 100;
    return value;
}

static bool SendChatPing(HWND game, const TrainConfig& c, const Spot& target) {
    if (!c.chatClear.Valid() || !c.chatInput.Valid() || !c.chatLatestCoordinate.Valid() ||
        !game || !IsWindow(game)) return false;
    PressGameKey(game, VK_RETURN);
    Sleep(250);
    if (!ClickGamePoint(game, c.chatClear)) return false;
    Sleep(150);
    if (!ClickGamePoint(game, c.chatInput)) return false;
    Sleep(120);
    // Do not depend on Ctrl+A: PostMessage does not change Windows' global key
    // state and some Unity input fields query that state for modifiers.
    for (int i = 0; i < 64; ++i) PressGameKey(game, VK_BACK);
    const std::wstring ping = L"@GOTO_" + std::to_wstring(target.mapID) + L"_" +
        std::to_wstring(ChatCoordinate(target.x, target.y, true)) + L"_" +
        std::to_wstring(ChatCoordinate(target.x, target.y, false));
    if (!TypeUnicode(game, ping)) return false;
    PressGameKey(game, VK_RETURN);
    Sleep(600);
    return ClickGamePoint(game, c.chatLatestCoordinate);
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
        std::int32_t currentResID = 0;
        std::wstring resDetail;
        if (!ResolveNpcResIDByRoleID(currentRoleID, currentResID, resDetail)) {
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
            {rva::LuaGetBuffs,           {0x48,0x83,0xEC,0x28,0x80,0x3D,0x98,0x8F,0x15,0x03,0x00,0x75}},
            {rva::LuaHasBuff,            {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xA8,0x09,0x15,0x03}},
            {rva::LuaRequestUsingSkill,  {0x33,0xD2,0xE9,0x09,0x45,0x05,0x00,0xCC,0xCC,0xCC,0xCC,0xCC}},
            {rva::UIInputSetText,         {0x48,0x83,0xEC,0x28,0x48,0x8B,0x89,0xE0,0x00,0x00,0x00,0x48}},
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
            {rva::LuaGetNearbyEnemyIDs, {0x45,0x33,0xC9,0xE9,0x88,0x8F,0xEA,0xFF,0xCC,0xCC,0xCC,0xCC}},
            {rva::LuaSelectTarget,      {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0xDD,0xED,0x14,0x03}},
            {rva::LuaGetSelectedTarget, {0x48,0x83,0xEC,0x28,0x80,0x3D,0x55,0xCA,0x14,0x03,0x00,0x75}},
            {rva::SelectedTargetGetRoleID,{0x48,0x83,0xEC,0x28,0x80,0x3D,0xD7,0x0C,0x2A,0x03,0x00,0x75}},
            {rva::LuaIsSelectTargetDie, {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x58,0xF3,0x14,0x03}},
            {rva::LuaIsMapReady,        {0x48,0x83,0xEC,0x28,0x80,0x3D,0xC8,0x01,0x15,0x03,0x00,0x75}},
            {rva::LuaMainFindUI,        {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x1F,0x16,0x12,0x03}},
            {rva::UIObjectGetName,      {0x48,0x83,0xEC,0x28,0x48,0x8B,0x49,0x30,0x48,0x85,0xC9,0x74}},
            {rva::UIObjectGetParent,    {0x40,0x53,0x48,0x83,0xEC,0x20,0x80,0x3D,0x09,0x73,0x29,0x03}},
            {rva::UIObjectCoreParents,   {0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83,0xEC,0x20,0x80,0x3D}},
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
            {rva::UIRectGetPointerClickHandler,{0x48,0x8B,0x81,0x90,0x00,0x00,0x00,0xC3,0xCC,0xCC,0xCC,0xCC}},
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
        scratchBuffer_ = process_.Allocate(1024);
        staticValueBuffer_ = process_.Allocate(sizeof(std::uint64_t));
        if (!il2cppDomainGet_ || !il2cppDomainAssemblyOpen_ || !il2cppAssemblyGetImage_ ||
            !il2cppClassFromName_ || !il2cppClassIsAssignableFrom_ ||
            !il2cppClassGetField_ || !il2cppFieldStaticGetValue_ ||
            !il2cppGcHandleNew_ || !il2cppGcHandleGetTarget_ || !il2cppGcHandleFree_ ||
            !il2cppGetCorlib_ || !il2cppArrayNew_ || !scratchBuffer_ || !staticValueBuffer_) {
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
            !ResolveClass("FGStudio.LuaSystem.GUI", "UIRectTransform", uiRectTransformClass_) ||
            !ResolveClass("FGStudio.LuaSystem.GUI", "UIInput", uiInputClass_) ||
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
                            0, systemObjectClass_, 1000) || !systemObjectClass_) {
            error = L"Không định vị được System.Object để gọi callback Lua an toàn";
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
        il2cppGetCorlib_ = il2cppArrayNew_ = 0;
        assemblyImage_ = uiObjectClass_ = uiButtonClass_ = uiToggleClass_ = uiRectTransformClass_ =
            uiInputClass_ = uiInstancesField_ = systemObjectClass_ = 0;
        uiButtonClassCache_.clear();
        uiToggleClassCache_.clear();
        uiRectClassCache_.clear();
        uiInputClassCache_.clear();
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
    bool IsUiRect(std::uint64_t object) {
        return IsAssignableControl(object, uiRectTransformClass_, uiRectClassCache_);
    }
    bool IsUiInput(std::uint64_t object) {
        return IsAssignableControl(object, uiInputClass_, uiInputClassCache_);
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
    bool ReadAllUiToggles(std::vector<std::uint64_t>& toggles) {
        toggles.clear();
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
            if (process_.Read(entry + off::EntryValue, object) && IsUiToggle(object))
                toggles.push_back(object);
        }
        return true;
    }
    bool ReadAllUiRects(std::vector<std::uint64_t>& rects) {
        rects.clear();
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
            if (process_.Read(entry + off::EntryValue, object) && IsUiRect(object))
                rects.push_back(object);
        }
        return true;
    }

    bool ReadAllUiInputs(std::vector<std::uint64_t>& inputs) {
        inputs.clear();
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
            if (process_.Read(entry + off::EntryValue, object) && IsUiInput(object))
                inputs.push_back(object);
        }
        return true;
    }

    bool HasLikelyBagUiRect() {
        const std::uint64_t zero = 0;
        if (!process_.WriteBytes(staticValueBuffer_, &zero, sizeof(zero))) return false;
        std::uint64_t ignored = 0;
        if (!RemoteAbsolute(il2cppFieldStaticGetValue_, uiInstancesField_,
                            staticValueBuffer_, 0, 0, ignored, 1000)) return false;
        std::uint64_t dictionary = 0, entries = 0;
        std::int32_t count = 0;
        if (!process_.Read(staticValueBuffer_, dictionary) || !dictionary ||
            !process_.Read(dictionary + off::DictionaryEntries, entries) || !entries ||
            !process_.Read(dictionary + off::DictionaryCount, count) || count < 0 || count > 32768)
            return false;
        for (std::int32_t i = 0; i < count; ++i) {
            std::uint64_t object = 0;
            const std::uint64_t entry = entries + off::ArrayData +
                static_cast<std::uint64_t>(i) * off::EntrySize;
            if (!process_.Read(entry + off::EntryValue, object) || !IsUiRect(object)) continue;
            std::uint64_t active = 0, namePointer = 0;
            if (!Remote(rva::UIObjectActiveInHierarchy, object, 0, 0, 0, active, 500) ||
                (active & 0xFFu) == 0 ||
                !Remote(rva::UIObjectGetName, object, 0, 0, 0, namePointer, 500)) continue;
            const std::wstring nameKey = CompactMatch(process_.ReadIl2CppString(namePointer));
            if (!ContainsCompact(nameKey, {L"item", L"slot", L"cell"})) continue;
            std::wstring parentKey;
            if (ReadAncestorKey(object, parentKey) &&
                ContainsCompact(parentKey, {L"bag", L"inventory", L"package", L"itempack",
                                             L"bagitem", L"itemgrid", L"itemlist"}))
                return true;
        }
        return false;
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
        DauThai, AutoRoot, Fight, Stop, ShopEntry, SellTab, QuickSell, EquipmentTab,
        Treatment, TreatmentConfirm, TreatmentAck, ChatOpen, ChatSend, ChatClose
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
    bool DescribeRect(std::uint64_t rect, bool includeDescendants, ButtonInfo& info) {
        info = {};
        std::uint64_t active = 0, pointer = 0;
        if (!Remote(rva::UIObjectActiveInHierarchy, rect, 0, 0, 0, active, 650) ||
            (active & 0xFFu) == 0) return false;
        info.object = rect;
        if (Remote(rva::UIObjectGetName, rect, 0, 0, 0, pointer, 650))
            info.name = process_.ReadIl2CppString(pointer);
        pointer = 0;
        if (!Remote(rva::UIRectGetPointerClickHandler, rect, 0, 0, 0, pointer, 650) || !pointer)
            return false;
        info.handler = process_.ReadIl2CppString(pointer);
        if (info.handler.empty()) return false;
        if (includeDescendants) CollectDescendantLabels(rect, info.descendants);
        if (!info.name.empty()) info.label = L"Name=“" + info.name + L"”";
        if (!info.handler.empty()) {
            if (!info.label.empty()) info.label += L" • ";
            info.label += L"Lua=“" + info.handler + L"”";
        }
        if (!info.descendants.empty()) {
            if (!info.label.empty()) info.label += L" • ";
            info.label += L"Child=“" + info.descendants + L"”";
        }
        if (info.label.empty()) info.label = L"UIRectTransform không có nhãn";
        info.nameKey = CompactMatch(info.name);
        info.textKey = CompactMatch(info.text);
        info.allKey = CompactMatch(info.name + L" " + info.handler + L" " + info.descendants);
        return true;
    }
    bool ReadActiveRectInfos(bool includeDescendants, std::vector<ButtonInfo>& output) {
        output.clear();
        std::vector<std::uint64_t> rects;
        if (!ReadAllUiRects(rects)) return false;
        for (const std::uint64_t rect : rects) {
            ButtonInfo info;
            if (DescribeRect(rect, includeDescendants, info)) output.push_back(std::move(info));
        }
        return true;
    }

    bool ReadActiveToggleInfos(bool includeDescendants,
                               std::vector<ButtonInfo>& output) {
        output.clear();
        std::vector<std::uint64_t> toggles;
        if (!ReadAllUiToggles(toggles)) return false;
        for (const std::uint64_t toggle : toggles) {
            ButtonInfo info;
            if (DescribeToggle(toggle, includeDescendants, info))
                output.push_back(std::move(info));
        }
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
        const bool unwantedMenuItem = ContainsCompact(
            key, {L"phuban", L"nhiemvu", L"quest", L"thietlap", L"setting",
                  L"dung", L"stop", L"cancel", L"close", L"thoat", L"pk"});
        switch (role) {
            case ButtonRole::DauThai: {
                int score = 0;
                if (info.textKey == L"dauthai") score = 600;
                else if (ContainsCompact(key, {L"dauthai"})) score = 520;
                if (ContainsCompact(key, {L"cancel", L"close", L"huy", L"thoat"}))
                    score -= 1000;
                return score;
            }
            case ButtonRole::Fight: {
                if (info.textKey == L"danhquai") return 650;
                if (unwantedMenuItem) return -1000;
                if (ContainsCompact(key, {L"danhquai"})) return 560;
                if (ContainsCompact(key, {L"autofight", L"fightmonster", L"monsterfight",
                                          L"autoattack", L"autobattle", L"rangerauto",
                                          L"btnfightmonster"})) return 390;
                return 0;
            }
            case ButtonRole::AutoRoot: {
                if (info.textKey == L"auto") return 650;
                if (unwantedMenuItem || ContainsCompact(key, {L"danhquai", L"autofight",
                                                               L"autoattack", L"autobattle"}))
                    return -1000;
                if (info.nameKey == L"auto" || info.nameKey == L"btnauto" ||
                    info.nameKey == L"buttonauto") return 560;
                if (ContainsCompact(key, {L"openautomenu", L"automenu", L"btnopenauto",
                                          L"mainauto", L"buttonauto"})) return 390;
                return 0;
            }
            case ButtonRole::Stop: {
                if (info.textKey == L"dung" || info.textKey == L"stop") return 650;
                if (ContainsCompact(key, {L"stopauto", L"autostop", L"stopfight",
                                          L"dungdanh", L"dungauto"})) return 500;
                if (ContainsCompact(key, {L"danhquai", L"phuban", L"nhiemvu", L"quest",
                                          L"thietlap", L"setting", L"pk"})) return -1000;
                return 0;
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
            case ButtonRole::QuickSell: {
                if (info.textKey == L"banvatphamnhanh") return 720;
                if (ContainsCompact(key, {L"banvatphamnhanh", L"quicksell",
                                          L"fastsell", L"sellquick"})) return 560;
                return 0;
            }
            case ButtonRole::EquipmentTab: {
                if (info.textKey == L"trangbi") return 760;
                if (ContainsCompact(key, {L"trangbi", L"equipmenttab", L"bagequipment",
                                          L"equipequipment", L"tabquip", L"equiptab"})) return 570;
                if (ContainsCompact(key, {L"muavatpham", L"buyitem", L"shopitem"})) return -1000;
                return 0;
            }
            case ButtonRole::Treatment: {
                if (info.textKey == L"trilieu" || info.textKey == L"trithuong" ||
                    info.textKey == L"hoiphuc") return 760;
                if (ContainsCompact(key, {L"trilieu", L"trithuong", L"hoiphuc", L"treatment",
                                          L"treat", L"heal", L"recoverhp", L"recover"})) return 560;
                return 0;
            }
            case ButtonRole::TreatmentConfirm: {
                if (info.textKey == L"xacnhan") return 800;
                if (ContainsCompact(key, {L"xacnhan", L"confirm", L"buttonok", L"btnok"})) return 600;
                if (ContainsCompact(key, {L"suynghi", L"cancel", L"huy", L"khong"})) return -1000;
                return 0;
            }
            case ButtonRole::TreatmentAck: {
                if (info.textKey == L"tabietroi") return 820;
                if (ContainsCompact(key, {L"tabietroi", L"iknow", L"understand", L"gotit"})) return 620;
                return 0;
            }
            case ButtonRole::ChatOpen: {
                if (info.textKey == L"chat") return 820;
                if (info.nameKey == L"chat" || info.nameKey == L"btnchat" ||
                    info.nameKey == L"buttonchat") return 760;
                if (ContainsCompact(key, {L"openchat", L"chatbutton", L"btnopenchat",
                                          L"mainchat"})) return 620;
                if (ContainsCompact(key, {L"send", L"gui", L"close", L"dong", L"voice", L"live"}))
                    return -1000;
                return 0;
            }
            case ButtonRole::ChatSend: {
                if (info.textKey == L"guitinnhan") return 900;
                if (ContainsCompact(key, {L"guitinnhan", L"sendmessage", L"sendchat",
                                          L"btnsend", L"chatsend"})) return 700;
                return 0;
            }
            case ButtonRole::ChatClose: {
                if (info.textKey == L"dong" || info.textKey == L"close") return 500;
                if (info.nameKey == L"close" || info.nameKey == L"btnclose" ||
                    info.nameKey == L"buttonclose" || info.nameKey == L"x") return 480;
                if (ContainsCompact(key, {L"closechat", L"chatclose", L"btnclosechat"})) return 760;
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
    bool FindToggle(ButtonRole role, ButtonInfo& selected, std::wstring& reason,
                    bool inspectDescendants) {
        std::vector<ButtonInfo> toggles;
        if (!ReadActiveToggleInfos(false, toggles)) {
            reason = L"Không đọc được UIObject.instances/UIToggle";
            return false;
        }
        if (ChooseButton(toggles, role, selected, reason)) return true;
        if (!inspectDescendants) return false;
        if (!ReadActiveToggleInfos(true, toggles)) {
            reason = L"Không đọc được nhãn con của UIToggle";
            return false;
        }
        return ChooseButton(toggles, role, selected, reason);
    }
    bool FindRect(ButtonRole role, ButtonInfo& selected, std::wstring& reason,
                  bool inspectDescendants) {
        std::vector<ButtonInfo> rects;
        if (!ReadActiveRectInfos(false, rects)) {
            reason = L"Không đọc được UIRectTransform có Lua click handler";
            return false;
        }
        if (ChooseButton(rects, role, selected, reason)) return true;
        if (!inspectDescendants) return false;
        if (!ReadActiveRectInfos(true, rects)) {
            reason = L"Không đọc được nhãn con UIRectTransform";
            return false;
        }
        return ChooseButton(rects, role, selected, reason);
    }
    enum class ActionKind { Button, Toggle, RectLua };
    struct ActionControl { ActionKind kind = ActionKind::Button; ButtonInfo info; };
    bool FindAction(ButtonRole role, ActionControl& selected, std::wstring& reason,
                    bool inspectDescendants) {
        ButtonInfo info;
        std::wstring buttonReason;
        if (FindButton(role, info, buttonReason, inspectDescendants)) {
            selected = {ActionKind::Button, std::move(info)};
            return true;
        }
        std::wstring toggleReason;
        if (FindToggle(role, info, toggleReason, inspectDescendants)) {
            selected = {ActionKind::Toggle, std::move(info)};
            return true;
        }
        std::wstring rectReason;
        if (FindRect(role, info, rectReason, inspectDescendants)) {
            selected = {ActionKind::RectLua, std::move(info)};
            return true;
        }
        reason = L"UIButton: " + buttonReason + L" • UIToggle: " + toggleReason +
                 L" • UIRect/Lua: " + rectReason;
        return false;
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
    bool CreateUiArgArray(std::uint64_t uiObject, std::uint64_t& array,
                          std::uint64_t& handle) {
        array = handle = 0;
        if (!systemObjectClass_ || !il2cppArrayNew_ ||
            !RemoteAbsolute(il2cppArrayNew_, systemObjectClass_, 3, 0, 0, array, 1200) || !array ||
            !RemoteAbsolute(il2cppGcHandleNew_, array, 0, 0, 0, handle, 900) || !handle)
            return false;
        if (!process_.WriteBytes(array + off::ArrayData, &uiObject, sizeof(uiObject))) {
            std::uint64_t ignored = 0;
            RemoteAbsolute(il2cppGcHandleFree_, handle, 0, 0, 0, ignored, 500);
            array = handle = 0;
            return false;
        }
        return true;
    }
    bool InvokeRectLua(std::uint64_t rect) {
        std::uint64_t handler = 0, executor = 0, args = 0, argsHandle = 0, ignored = 0;
        if (!Remote(rva::UIRectGetPointerClickHandler, rect, 0, 0, 0, handler, 900) || !handler ||
            process_.ReadIl2CppString(handler).empty() ||
            !Remote(rva::MonoBehaviourExecutorGetInstance, 0, 0, 0, 0, executor, 900) || !executor ||
            !CreateUiArgArray(rect, args, argsHandle)) return false;
        // UIRectTransform.HandlePointerClick builds object[3]: [uiObject, pointerData, pointerId].
        // Use the same arity, but leave the two pointer-only values null instead of fabricating PointerEventData.
        // Native ABI: this + uiObject + functionName + object[] + hidden MethodInfo*.
        const bool called = Remote5(rva::MonoBehaviourExecutorExecuteUiObject,
                                    executor, rect, handler, args, 0, ignored, 2200);
        std::uint64_t freeIgnored = 0;
        RemoteAbsolute(il2cppGcHandleFree_, argsHandle, 0, 0, 0, freeIgnored, 600);
        return called;
    }
    bool InvokeAction(const ActionControl& action) {
        if (action.kind == ActionKind::Toggle) return InvokeToggle(action.info.object);
        if (action.kind == ActionKind::RectLua) return InvokeRectLua(action.info.object);
        return InvokeButton(action.info.object);
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
    bool FindButtonOrToggle(ButtonRole role, ActionControl& selected,
                            std::wstring& reason, bool inspectDescendants) {
        ButtonInfo info;
        std::wstring buttonReason;
        if (FindButton(role, info, buttonReason, inspectDescendants)) {
            selected = {ActionKind::Button, std::move(info)};
            return true;
        }
        std::wstring toggleReason;
        if (FindToggle(role, info, toggleReason, inspectDescendants)) {
            selected = {ActionKind::Toggle, std::move(info)};
            return true;
        }
        reason = L"UIButton: " + buttonReason + L" • UIToggle: " + toggleReason;
        return false;
    }
    bool InvokeButtonOrToggle(const ActionControl& action) {
        return action.kind == ActionKind::Toggle
            ? InvokeToggle(action.info.object)
            : action.kind == ActionKind::Button && InvokeButton(action.info.object);
    }
    bool OpenAutoRootButton(std::wstring& detail) {
        // Preserve the only part known from older versions to work reliably:
        // AUTO root is a UIButton. Do not scan/invoke UIRect Lua here; doing so made
        // v0.8.0 expensive and could call unrelated active rectangles.
        ButtonInfo autoRoot;
        std::wstring reason;
        if (!FindButton(ButtonRole::AutoRoot, autoRoot, reason, false) &&
            !FindButton(ButtonRole::AutoRoot, autoRoot, reason, true)) {
            detail = L"Không định vị được UIButton AUTO • " + reason;
            return false;
        }
        if (!InvokeButton(autoRoot.object)) {
            detail = L"UIButton.HandleClickEvent() của AUTO không phản hồi • " + autoRoot.label;
            return false;
        }
        detail = L"Đã mở AUTO bằng UIButton.HandleClickEvent() • " + autoRoot.label;
        return true;
    }
    bool ClickInternalAutoFight(std::wstring& detail) {
        // Deterministic sequence only: AUTO root -> wait for the newly-created submenu -> Đánh quái.
        // Do not accept a globally-visible/stale "fight" control before opening AUTO, because that
        // was able to skip the known-good root click and could call an unrelated recycled UI object.
        std::wstring rootDetail;
        if (!OpenAutoRootButton(rootDetail)) {
            detail = rootDetail;
            return false;
        }

        ActionControl fight;
        std::wstring fightReason;
        bool foundFight = false;
        for (int attempt = 0; attempt < 7 && !foundFight; ++attempt) {
            Sleep(180);
            foundFight = FindButtonOrToggle(ButtonRole::Fight, fight, fightReason,
                                            attempt >= 5);
        }
        if (!foundFight) {
            detail = rootDetail +
                     L" • AUTO đã mở nhưng chưa thấy Đánh quái dạng UIButton/UIToggle • " +
                     fightReason;
            return false;
        }
        if (!InvokeButtonOrToggle(fight)) {
            detail = L"Đã thấy Đánh quái nhưng callback Button/Toggle không phản hồi • " +
                     fight.info.label;
            return false;
        }

        bool verified = false;
        for (int i = 0; i < 5 && !verified; ++i) {
            Sleep(180);
            std::uint64_t enabled = 0;
            verified = Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enabled, 900) &&
                       (enabled & 0xFFu) != 0;
        }
        if (!verified && fight.kind == ActionKind::Toggle) {
            // set_Selected can change the visual toggle without executing the Lua select handler.
            // Fire the game's bool select handler once, never a PointerEventData/null click.
            std::uint64_t ignored = 0;
            Remote(rva::UIToggleHandleSelectEvent, fight.info.object, 1, 0, 0, ignored, 1600);
            for (int i = 0; i < 4 && !verified; ++i) {
                Sleep(180);
                std::uint64_t enabled = 0;
                verified = Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enabled, 900) &&
                           (enabled & 0xFFu) != 0;
            }
        }
        const std::wstring kind = fight.kind == ActionKind::Toggle
            ? L"UIToggle.set_Selected/HandleSelectEvent"
            : L"UIButton.HandleClickEvent()";
        detail = verified
            ? L"Đã bật AUTO → Đánh quái bằng " + kind + L" • AutoFight=ON • " + fight.info.label
            : L"AUTO đã mở và đã gọi Đánh quái bằng " + kind +
              L" nhưng AutoFight vẫn OFF; không coi là đã bật";
        return verified;
    }
    bool ClickInternalAutoStop(std::wstring& detail) {
        ActionControl stop;
        std::wstring stopReason;
        bool foundStop = FindButtonOrToggle(ButtonRole::Stop, stop, stopReason, false);
        if (!foundStop) {
            std::wstring rootDetail;
            if (!OpenAutoRootButton(rootDetail)) {
                detail = L"Không mở được AUTO để Dừng • " + rootDetail;
                return false;
            }
            for (int attempt = 0; attempt < 6 && !foundStop; ++attempt) {
                Sleep(150);
                foundStop = FindButtonOrToggle(ButtonRole::Stop, stop, stopReason,
                                               attempt == 5);
            }
            if (!foundStop) {
                detail = rootDetail +
                         L" • AUTO đã mở nhưng chưa thấy Dừng dạng UIButton/UIToggle • " +
                         stopReason;
                return false;
            }
        }
        if (!InvokeButtonOrToggle(stop)) {
            detail = L"Control Dừng Button/Toggle không phản hồi • " + stop.info.label;
            return false;
        }
        bool stopped = false;
        for (int i = 0; i < 5 && !stopped; ++i) {
            Sleep(160);
            std::uint64_t enabled = 1;
            stopped = Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enabled, 900) &&
                      (enabled & 0xFFu) == 0;
        }
        detail = stopped
            ? L"Đã AUTO → Dừng và xác nhận AutoFight=OFF • " + stop.info.label
            : L"Đã gọi Dừng nhưng AutoFight vẫn ON; không coi là đã dừng • " + stop.info.label;
        return stopped;
    }
    bool ReadAncestorKey(std::uint64_t object, std::wstring& key) {
        key.clear();
        // Fast path: the game exposes all core parents in one managed array.
        std::uint64_t parents = 0;
        if (Remote(rva::UIObjectCoreParents, object, 0, 0, 0, parents, 700) && parents) {
            std::uint64_t length = 0;
            if (process_.Read(parents + off::ArrayLength, length) && length <= 64) {
                for (std::uint64_t i = 0; i < length; ++i) {
                    std::uint64_t parent = 0, namePointer = 0;
                    if (!process_.Read(parents + off::ArrayData + i * sizeof(std::uint64_t), parent) ||
                        !parent) continue;
                    if (Remote(rva::UIObjectGetName, parent, 0, 0, 0, namePointer, 500))
                        key += CompactMatch(process_.ReadIl2CppString(namePointer));
                }
                if (!key.empty()) return true;
            }
        }
        // Compatibility fallback if a server build returns no CoreParents array.
        std::uint64_t current = object;
        std::set<std::uint64_t> seen;
        for (int depth = 0; depth < 10 && current && seen.insert(current).second; ++depth) {
            std::uint64_t parent = 0;
            if (!Remote(rva::UIObjectGetParent, current, 0, 0, 0, parent, 600) || !parent) break;
            std::uint64_t namePointer = 0;
            if (Remote(rva::UIObjectGetName, parent, 0, 0, 0, namePointer, 600))
                key += CompactMatch(process_.ReadIl2CppString(namePointer));
            current = parent;
        }
        return !key.empty();
    }
    bool IsSafeBagItemButton(const ButtonInfo& info) {
        // Only click an item control when both the control name and its parent path
        // say it belongs to the player's bag. Never infer a slot from screen position.
        if (!ContainsCompact(info.nameKey,
                {L"item", L"slot", L"cell", L"griditem", L"bagitem", L"packitem"}))
            return false;
        std::wstring parentKey;
        if (!ReadAncestorKey(info.object, parentKey)) return false;
        if (!ContainsCompact(parentKey,
                {L"bag", L"inventory", L"package", L"itempack", L"packitem",
                 L"bagitem", L"itemgrid", L"itemlist"}))
            return false;
        if (ContainsCompact(info.nameKey + parentKey,
                {L"buyitem", L"productitem", L"shoplistitem", L"npcitem"}))
            return false;
        return true;
    }
    bool IsSafeBagItemRect(const ButtonInfo& info) {
        if (info.handler.empty() ||
            !ContainsCompact(info.nameKey, {L"item", L"slot", L"cell", L"griditem",
                                            L"bagitem", L"packitem"})) return false;
        std::wstring parentKey;
        if (!ReadAncestorKey(info.object, parentKey) ||
            !ContainsCompact(parentKey, {L"bag", L"inventory", L"package", L"itempack",
                                         L"packitem", L"bagitem", L"itemgrid", L"itemlist"}))
            return false;
        if (ContainsCompact(info.nameKey + parentKey,
                            {L"buyitem", L"productitem", L"shoplistitem", L"npcitem"}))
            return false;
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
    bool ResolveNpcResIDByRoleID(std::int32_t roleID, std::int32_t& resID,
                                 std::wstring& detail) {
        resID = 0;
        if (roleID <= 0) {
            detail = L"RoleID NPC không hợp lệ";
            return false;
        }
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
        detail = L"Không thấy RoleID " + std::to_wstring(roleID) +
                 L" trong SessionData.NPCs/MovingNPCs";
        return false;
    }
    bool ResolveSavedNpcResID(const SellNpc& saved, std::int32_t& resID,
                              std::wstring& detail) {
        resID = saved.resID;
        if (resID <= 0) {
            // Backward compatibility for files saved by v0.7.x/v0.8.1: resolve the current
            // nearby object by name/RoleID, then translate RoleID -> GNPC.ResID.
            std::int32_t nearestRoleID = 0;
            std::wstring nearestName, nearestDetail;
            if (!ReadNearestNpcIdentity(nearestRoleID, nearestName, nearestDetail)) {
                detail = L"NPC cũ chưa có ResID; hãy đứng gần và lưu lại • " + nearestDetail;
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
            if (!ResolveNpcResIDByRoleID(nearestRoleID, resID, detail)) return false;
        }

        // Verify that this NPC resource actually exists close enough for the game's own
        // GetNearestNPC(resID) query.  This prevents sending ClickNPC with a stale/wrong ID and
        // avoids the game's “Không tìm thấy NPC tương ứng!” popup.
        std::uint64_t matchingNpc = 0;
        if (!Remote(rva::LuaGetNearestNPCByResID, static_cast<std::uint32_t>(resID),
                    0, 0, 0, matchingNpc, 1000) || !matchingNpc) {
            detail = L"Không thấy NPC ResID " + std::to_wstring(resID) +
                     L" trong phạm vi tương tác; hãy đứng sát NPC hoặc lưu lại NPC";
            return false;
        }
        detail = L"Đã xác minh NPC ResID " + std::to_wstring(resID);
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
    bool WaitAction(ButtonRole role, ActionControl& action, std::wstring& reason,
                    int attempts = 10, int sleepMs = 160) {
        for (int i = 0; i < attempts; ++i) {
            if (i) Sleep(static_cast<DWORD>(sleepMs));
            if (FindAction(role, action, reason, i == attempts - 1)) return true;
        }
        return false;
    }
    bool OpenSellUi(std::wstring& detail) {
        if (!OpenNpc(sellNpc_, detail)) return false;

        std::wstring reason;
        ActionControl shopEntry;
        // The NPC dialog in this client exposes "Mua thú cưỡi" before the shop page.
        // If the shop is already open (e.g. server/UI remembers the previous page), accept
        // a visible SellTab instead of blindly clicking an unrelated control.
        if (WaitAction(ButtonRole::ShopEntry, shopEntry, reason, 8, 150)) {
            if (!InvokeAction(shopEntry)) {
                detail = L"Đã mở NPC nhưng ‘Mua thú cưỡi’ không phản hồi • " + shopEntry.info.label;
                return false;
            }
            Sleep(260);
        } else {
            ActionControl alreadySell;
            std::wstring sellProbe;
            if (!FindAction(ButtonRole::SellTab, alreadySell, sellProbe, true)) {
                detail = L"Đã ClickNPC nhưng chưa thấy ‘Mua thú cưỡi’/shop • " + reason;
                return false;
            }
        }

        ActionControl sellTab;
        if (!WaitAction(ButtonRole::SellTab, sellTab, reason, 12, 150)) {
            detail = L"Đã vào shop nhưng chưa định vị được ‘Bán vật phẩm’ • " + reason;
            return false;
        }
        if (!InvokeAction(sellTab)) {
            detail = L"Control ‘Bán vật phẩm’ không phản hồi • " + sellTab.info.label;
            return false;
        }
        Sleep(220);

        ActionControl quickSell;
        if (!WaitAction(ButtonRole::QuickSell, quickSell, reason, 8, 140)) {
            detail = L"Đã vào Bán vật phẩm nhưng chưa thấy ‘Bán vật phẩm nhanh’ • " + reason;
            return false;
        }
        if (!InvokeAction(quickSell)) {
            detail = L"Không bật được ‘Bán vật phẩm nhanh’ • " + quickSell.info.label;
            return false;
        }
        Sleep(180);

        ActionControl equipment;
        if (!WaitAction(ButtonRole::EquipmentTab, equipment, reason, 8, 140)) {
            detail = L"Đã bật bán nhanh nhưng chưa thấy tab ‘Trang bị’ trong tay nải • " + reason;
            return false;
        }
        if (!InvokeAction(equipment)) {
            detail = L"Không chuyển được sang tab ‘Trang bị’ • " + equipment.info.label;
            return false;
        }
        Sleep(220);
        detail = L"NPC → Mua thú cưỡi → Bán vật phẩm → Bán nhanh → Trang bị";
        return true;
    }
    bool IsCloseControl(const ButtonInfo& info, int& score) {
        score = 0;
        const std::wstring closeKey = CompactMatch(info.name + L" " + info.text + L" " +
                                                   info.handler + L" " + info.descendants);
        if (info.textKey == L"dong" || info.textKey == L"close" || info.textKey == L"x") score = 720;
        else if (ContainsCompact(closeKey, {L"btnclose", L"buttonclose", L"closebutton",
                                            L"shopclose", L"storeclose", L"bagclose",
                                            L"inventoryclose", L"packageclose"})) score = 600;
        if (score <= 0) return false;
        std::wstring parentKey;
        if (!ReadAncestorKey(info.object, parentKey)) return false;
        if (!ContainsCompact(parentKey, {L"shop", L"store", L"sell", L"trade", L"npc",
                                         L"business", L"bag", L"inventory", L"package",
                                         L"itempack", L"pack"})) return false;
        if (ContainsCompact(parentKey, {L"bag", L"inventory", L"package", L"itempack"})) score += 40;
        return true;
    }
    bool CloseOneTradeOrBagUi(std::wstring& closedLabel) {
        struct Candidate { ActionControl action; int score; };
        std::vector<Candidate> candidates;
        auto append = [&](const std::vector<ButtonInfo>& infos, ActionKind kind) {
            for (const ButtonInfo& info : infos) {
                int score = 0;
                if (IsCloseControl(info, score)) candidates.push_back({{kind, info}, score});
            }
        };
        std::vector<ButtonInfo> infos;
        if (ReadActiveButtonInfos(true, infos)) append(infos, ActionKind::Button);
        if (ReadActiveRectInfos(true, infos)) append(infos, ActionKind::RectLua);
        if (ReadActiveToggleInfos(true, infos)) append(infos, ActionKind::Toggle);
        if (candidates.empty()) return false;
        std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
            return a.score > b.score;
        });
        if (candidates.size() > 1 && candidates[0].score == candidates[1].score &&
            candidates[0].action.info.object != candidates[1].action.info.object) return false;
        if (!InvokeAction(candidates.front().action)) return false;
        closedLabel = candidates.front().action.info.label;
        return true;
    }
    void CloseTradeAndBagUi(std::wstring& detail) {
        int closed = 0;
        for (int i = 0; i < 4; ++i) {
            std::wstring label;
            if (!CloseOneTradeOrBagUi(label)) break;
            ++closed;
            Sleep(140);
        }
        if (closed > 0) detail += L" • đã đóng " + std::to_wstring(closed) + L" cửa sổ shop/tay nải";
    }
    static std::wstring ItemAttemptKey(const ActionControl& item) {
        return std::to_wstring(item.info.object) + L"|" + item.info.nameKey + L"|" +
               CompactMatch(item.info.handler) + L"|" + item.info.textKey;
    }
    bool CollectSafeBagItems(std::vector<ActionControl>& candidates) {
        candidates.clear();
        std::vector<ButtonInfo> buttons;
        if (ReadActiveButtonInfos(false, buttons)) {
            for (const ButtonInfo& info : buttons)
                if (IsSafeBagItemButton(info)) candidates.push_back({ActionKind::Button, info});
        }
        std::vector<ButtonInfo> rects;
        if (ReadActiveRectInfos(false, rects)) {
            for (const ButtonInfo& info : rects)
                if (IsSafeBagItemRect(info)) candidates.push_back({ActionKind::RectLua, info});
        }
        std::sort(candidates.begin(), candidates.end(), [](const ActionControl& a,
                                                            const ActionControl& b) {
            return a.info.object < b.info.object;
        });
        return true;
    }
    bool TrySellAtNpc(int& freeAfter, std::wstring& detail) {
        freeAfter = -1;
        int freeBefore = -1;
        if (!ReadFreeBagSpace(freeBefore)) {
            detail = L"Không đọc được GetFreeBagSpace trước khi bán";
            return false;
        }
        if (!OpenSellUi(detail)) return false;

        std::map<std::wstring, int> failedAttempts;
        std::set<std::wstring> permanentlySkipped;
        int bestFree = freeBefore;
        int callbacks = 0;
        int soldItems = 0;
        int skippedItems = 0;
        while (callbacks < 90) {
            std::vector<ActionControl> candidates;
            CollectSafeBagItems(candidates);
            if (candidates.empty()) break;

            const ActionControl* chosen = nullptr;
            std::wstring chosenKey;
            for (const ActionControl& item : candidates) {
                const std::wstring key = ItemAttemptKey(item);
                if (!permanentlySkipped.count(key)) {
                    chosen = &item;
                    chosenKey = key;
                    break;
                }
            }
            if (!chosen) break;  // every visible equipment slot has already failed 3 times.

            if (!InvokeAction(*chosen)) {
                const int failures = ++failedAttempts[chosenKey];
                ++callbacks;
                if (failures >= 3 && permanentlySkipped.insert(chosenKey).second) ++skippedItems;
                Sleep(100);
                continue;
            }
            ++callbacks;
            Sleep(180);
            int currentFree = -1;
            if (ReadFreeBagSpace(currentFree) && currentFree > bestFree) {
                bestFree = currentFree;
                ++soldItems;
                failedAttempts.erase(chosenKey);
                // Sold cells can be recycled/reordered by the UI; rescan from scratch next loop.
                Sleep(80);
                continue;
            }
            const int failures = ++failedAttempts[chosenKey];
            if (failures >= 3 && permanentlySkipped.insert(chosenKey).second) ++skippedItems;
        }

        int finalFree = bestFree;
        int readFinal = -1;
        if (ReadFreeBagSpace(readFinal)) finalFree = std::max(finalFree, readFinal);
        freeAfter = finalFree;
        std::wstring closeDetail;
        CloseTradeAndBagUi(closeDetail);

        if (soldItems <= 0 && finalFree <= freeBefore) {
            detail = L"Không bán được trang bị nào • đã thử " + std::to_wstring(callbacks) +
                     L" callback • bỏ qua " + std::to_wstring(skippedItems) +
                     L" món sau 3 lần thất bại" + closeDetail;
            return false;
        }
        detail = L"Đã bán " + std::to_wstring(soldItems) + L" món trang bị • " +
                 std::to_wstring(callbacks) + L" callback • bỏ qua " +
                 std::to_wstring(skippedItems) + L" món lỗi 3 lần • tay nải " +
                 std::to_wstring(freeBefore) + L" → " + std::to_wstring(finalFree) +
                 L" ô trống" + closeDetail;
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
        state_.autoFight = (autoFight & 0xFFu) != 0;
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
    bool SelectNearestEnemy(std::int32_t& roleID) {
        roleID = 0;
        std::uint64_t array = 0;
        if (!Remote(rva::LuaGetNearbyEnemyIDs, 1, 1, 1, 0, array, 1000) || !array) return false;
        std::uint64_t length = 0;
        if (!process_.Read(array + off::ArrayLength, length) || length == 0 || length > 1024 ||
            !process_.Read(array + off::ArrayData, roleID) || roleID <= 0) return false;
        std::uint64_t ignored = 0;
        return Remote(rva::LuaSelectTarget, static_cast<std::uint32_t>(roleID),
                      0, 0, 0, ignored, 900);
    }
    bool EnsureSkillTarget(std::int32_t& roleID, std::wstring& detail) {
        bool selectedRead = ReadSelectedTargetRoleID(roleID);
        bool targetDead = false;
        if (selectedRead && roleID > 0) {
            std::uint64_t dead = 0;
            if (Remote(rva::LuaIsSelectTargetDie, static_cast<std::uint32_t>(roleID),
                       0, 0, 0, dead, 800)) targetDead = (dead & 0xFFu) != 0;
        }
        if (!selectedRead || roleID <= 0 || targetDead) {
            if (SelectNearestEnemy(roleID)) {
                Sleep(120);
                return true;
            }
            // Equivalent to the user's Ctrl+Tab operation, delivered only to the
            // selected game window and without foreground activation.
            PressGameKey(game_.window, VK_TAB, true);
            Sleep(180);
            if (!ReadSelectedTargetRoleID(roleID) || roleID <= 0) {
                detail = L"Chưa tìm thấy mục tiêu địch quanh nhân vật";
                return false;
            }
        }
        return true;
    }
    bool TriggerSelectedSkill(std::wstring& detail) {
        if (config_.skillID <= 0) {
            detail = L"Chưa chọn skill";
            return false;
        }
        std::int32_t targetRoleID = 0;
        if (!EnsureSkillTarget(targetRoleID, detail)) return false;
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
    bool FindChatInput(std::uint64_t& input, std::wstring& detail) {
        input = 0;
        std::vector<std::uint64_t> inputs;
        if (!ReadAllUiInputs(inputs)) {
            detail = L"Không đọc được danh sách UIInput";
            return false;
        }
        struct Candidate { std::uint64_t object; int score; std::wstring label; };
        std::vector<Candidate> candidates;
        for (const std::uint64_t item : inputs) {
            std::uint64_t active = 0, namePointer = 0;
            if (!Remote(rva::UIObjectActiveInHierarchy, item, 0, 0, 0, active, 600) ||
                (active & 0xFFu) == 0) continue;
            std::wstring name;
            if (Remote(rva::UIObjectGetName, item, 0, 0, 0, namePointer, 600))
                name = process_.ReadIl2CppString(namePointer);
            const std::wstring nameKey = CompactMatch(name);
            std::wstring parentKey;
            ReadAncestorKey(item, parentKey);
            int score = 0;
            if (ContainsCompact(nameKey, {L"chatinput", L"inputchat", L"chatmessage",
                                          L"messageinput", L"inputmessage", L"talkinput"}))
                score = 900;
            else if (ContainsCompact(parentKey, {L"chat", L"talk", L"message", L"channel"}) &&
                     ContainsCompact(nameKey, {L"input", L"edit", L"text"}))
                score = 700;
            else if (ContainsCompact(parentKey, {L"chat", L"talk", L"message", L"channel"}))
                score = 520;
            if (ContainsCompact(nameKey + parentKey, {L"search", L"rename", L"filter"}))
                score -= 1000;
            if (score > 0)
                candidates.push_back({item, score, name.empty() ? L"UIInput" : name});
        }
        std::sort(candidates.begin(), candidates.end(),
                  [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
        if (candidates.empty()) {
            detail = L"Bảng chat đã mở nhưng chưa nhận diện được UIInput nhập tin nhắn";
            return false;
        }
        if (candidates.size() > 1 && candidates[0].score == candidates[1].score &&
            candidates[0].object != candidates[1].object) {
            detail = L"Có nhiều UIInput chat cùng mức khớp; không tự chọn mù";
            return false;
        }
        input = candidates[0].object;
        detail = L"UIInput=" + candidates[0].label;
        return true;
    }
    bool SetUiInputText(std::uint64_t input, const std::wstring& message,
                        std::wstring& detail) {
        if (!input || message.empty()) return false;
        const std::string utf8 = Utf8(message);
        if (utf8.empty() || utf8.size() > 800 || !il2cppStringNew_ || !WriteScratch(0, utf8)) {
            detail = L"Nội dung Auto chat rỗng hoặc quá dài";
            return false;
        }
        std::uint64_t managed = 0, handle = 0, ignored = 0;
        if (!RemoteAbsolute(il2cppStringNew_, scratchBuffer_, 0, 0, 0, managed, 1000) || !managed ||
            !RemoteAbsolute(il2cppGcHandleNew_, managed, 0, 0, 0, handle, 900) || !handle) {
            detail = L"Không tạo được chuỗi managed để nhập chat";
            return false;
        }
        const bool written = Remote(rva::UIInputSetText, input, managed, 0, 0, ignored, 1600);
        RemoteAbsolute(il2cppGcHandleFree_, handle, 0, 0, 0, ignored, 600);
        detail = written ? L"Đã điền nội dung trực tiếp vào UIInput của chat"
                         : L"UIInput.set_Text() không phản hồi";
        return written;
    }
    bool FindChatOpen(ActionControl& action, std::wstring& reason) {
        if (FindButtonOrToggle(ButtonRole::ChatOpen, action, reason, false) ||
            FindButtonOrToggle(ButtonRole::ChatOpen, action, reason, true))
            return true;
        // Some HUD builds implement the Chat icon as a Lua-backed rect. This fallback
        // is used only for the uniquely scored Chat icon, never for AUTO combat.
        ButtonInfo rect;
        std::wstring rectReason;
        if (FindRect(ButtonRole::ChatOpen, rect, rectReason, true)) {
            action = {ActionKind::RectLua, std::move(rect)};
            return true;
        }
        reason += L" • UIRect/Lua: " + rectReason;
        return false;
    }
    bool FindChatPanelAction(ButtonRole role, ActionControl& action,
                             std::wstring& reason, int attempts = 8) {
        for (int i = 0; i < attempts; ++i) {
            if (FindAction(role, action, reason, i == attempts - 1)) {
                if (role != ButtonRole::ChatClose) return true;
                std::wstring parentKey;
                if (ReadAncestorKey(action.info.object, parentKey) &&
                    ContainsCompact(parentKey, {L"chat", L"talk", L"message", L"channel"}))
                    return true;
                if (ContainsCompact(action.info.allKey, {L"closechat", L"chatclose"}))
                    return true;
            }
            Sleep(120);
        }
        return false;
    }
    bool SendChatViaUiInternal(const std::wstring& message, std::wstring& detail) {
        // Mirror the actual client workflow from the supplied video:
        // open Chat -> fill the chat UIInput -> press "Gửi tin nhắn" -> close Chat.
        std::uint64_t input = 0;
        std::wstring inputReason;
        if (!FindChatInput(input, inputReason)) {
            ActionControl open;
            std::wstring openReason;
            if (!FindChatOpen(open, openReason)) {
                detail = L"Không mở được kênh Chat bằng control nội bộ • " + openReason;
                return false;
            }
            if (!InvokeAction(open)) {
                detail = L"Control Chat không phản hồi • " + open.info.label;
                return false;
            }
            bool foundInput = false;
            for (int i = 0; i < 10 && !foundInput; ++i) {
                Sleep(120);
                foundInput = FindChatInput(input, inputReason);
            }
            if (!foundInput) {
                detail = L"Đã gọi mở Chat nhưng không tìm thấy ô nhập • " + inputReason;
                return false;
            }
        }

        std::wstring setDetail;
        if (!SetUiInputText(input, message, setDetail)) {
            detail = setDetail;
            return false;
        }

        ActionControl send;
        std::wstring sendReason;
        if (!FindChatPanelAction(ButtonRole::ChatSend, send, sendReason, 10) ||
            !InvokeAction(send)) {
            detail = setDetail + L" • không gọi được Gửi tin nhắn • " + sendReason;
            return false;
        }
        Sleep(180);

        ActionControl close;
        std::wstring closeReason;
        const bool foundClose = FindChatPanelAction(ButtonRole::ChatClose, close, closeReason, 6);
        const bool closed = foundClose && InvokeAction(close);
        detail = L"Đã mở Chat → nhập nội dung → Gửi tin nhắn" +
                 std::wstring(closed ? L" → đóng bảng Chat"
                                     : L"; gửi xong nhưng chưa xác nhận được nút đóng Chat");
        return true;
    }
    bool TryTreatmentAtNpc(std::wstring& detail) {
        if (!OpenNpc(healNpc_, detail)) return false;

        ActionControl treatment;
        std::wstring reason;
        if (!WaitAction(ButtonRole::Treatment, treatment, reason, 12, 150) ||
            !InvokeAction(treatment)) {
            detail = L"Đã mở NPC trị liệu nhưng không gọi được Trị liệu • " + reason;
            CloseTradeAndBagUi(detail);
            return false;
        }

        ActionControl confirm;
        reason.clear();
        if (!WaitAction(ButtonRole::TreatmentConfirm, confirm, reason, 12, 150) ||
            !InvokeAction(confirm)) {
            detail = L"Đã bấm Trị liệu nhưng chưa gọi được Xác nhận • " + reason;
            CloseTradeAndBagUi(detail);
            return false;
        }

        ActionControl ack;
        reason.clear();
        if (!WaitAction(ButtonRole::TreatmentAck, ack, reason, 12, 150) ||
            !InvokeAction(ack)) {
            detail = L"Đã Xác nhận trị liệu nhưng chưa gọi được Ta biết rồi • " + reason;
            CloseTradeAndBagUi(detail);
            return false;
        }

        Sleep(180);
        detail = L"Đã hoàn tất NPC trị liệu: Trị liệu → Xác nhận → Ta biết rồi • " + healNpc_.name;
        CloseTradeAndBagUi(detail);
        return true;
    }

    bool TriggerAutoMenu(std::wstring& detail) {
        // Do not replace the real menu action with set_AutoFight.  Static
        // analysis shows that setter only writes PlayZone+0x20; the game's
        // Auto -> Đánh quái callbacks perform additional setup.
        return ClickInternalAutoFight(detail);
    }
    bool StartPathTo(const Spot& destination) {
        if (config_.mode == NavigationMode::ChatPing)
            return SendChatPing(game_.window, config_, destination);
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
        if (!executor_) return;
        if (config_.activation == TrainActivationMode::AutoMenu) {
            std::uint64_t enabled = 0;
            // Never launch an expensive AUTO UI scan merely because the state read failed.
            // During map load/hang the old code did exactly that and could race UI reconstruction.
            if (Remote(rva::LuaGetAutoFightEnabled, 0, 0, 0, 0, enabled, 700) &&
                (enabled & 0xFFu) != 0) {
                std::wstring stopDetail;
                ClickInternalAutoStop(stopDetail);
            }
        }
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
        bool awaitingRideCheck = false;
        auto nextTrainAction = Clock::now();
        auto nextAutoStopRetry = Clock::now();
        auto dauThaiRetryAfter = Clock::time_point{};
        auto transitionDeadline = Clock::time_point{};
        auto nextBagCheck = Clock::now();
        auto sellRetryAfter = Clock::time_point{};
        auto healRetryAfter = Clock::now();
        auto nextBuffCheck = Clock::now();
        auto nextBuffAction = Clock::now();
        auto nextAutoChat = Clock::now();
        bool trainTriggered = false;
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
                const bool windowHung = game_.window && IsWindow(game_.window) &&
                                        IsHungAppWindow(game_.window);
                if (windowHung) {
                    recovering = true;
                    healthyScans = 0;
                    trainTriggered = false;
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
                        trainTriggered = false;
                        autoCombatConfirmed = false;
                        UpdateLive(true, L"TẠM DỪNG AN TOÀN",
                                   error + (executor_->Idle()
                                       ? L" • đang chờ hai lần đọc ổn định"
                                       : L" • lệnh cũ còn bận, tuyệt đối không ghi đè"));
                    }
                } else {
                    LiveState live = State();
                    const auto now = Clock::now();
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

                    if (!recovering && live.mapReady && !live.waitingChangeMap && !live.dead &&
                        config_.autoChat && !config_.autoChatMessage.empty() &&
                        !sellingTrip && !healingTrip && now >= nextAutoChat) {
                        std::wstring chatDetail;
                        SendChatViaUiInternal(config_.autoChatMessage, chatDetail);
                        nextAutoChat = now + std::chrono::minutes(config_.autoChatMinutes);
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

                    if (!recovering) {
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
                            trainTriggered = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
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
                            trainTriggered = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
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
                            trainTriggered = false;
                            sellingTrip = true;
                            outsideTarget = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
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
                                std::wstring detail;
                                const bool healed = TryTreatmentAtNpc(detail);
                                if (healed || ++healFailures >= 3) {
                                    healingTrip = false;
                                    outsideTarget = false;
                                    nextNavigate = now;
                                    bestDistance = LLONG_MAX;
                                    lastProgress = now;
                                    UpdateLive(true, healed ? L"ĐÃ TRỊ LIỆU" : L"BỎ QUA TRỊ LIỆU",
                                               detail + (healed ? L" • quay lại bãi " + target_.name
                                                                : L" • thất bại 3 lần; tiếp tục ra bãi"));
                                } else {
                                    healRetryAfter = now + std::chrono::seconds(4);
                                    UpdateLive(true, L"TRỊ LIỆU CHƯA THÀNH CÔNG",
                                               detail + L" • thử " + std::to_wstring(healFailures) + L"/3");
                                }
                            }
                        } else if (sellingTrip &&
                                   AtSpot(live, Spot{sellNpc_.name, sellNpc_.mapID,
                                                    sellNpc_.x, sellNpc_.y})) {
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
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
                                int freeAfter = -1;
                                std::wstring detail;
                                const bool sold = TrySellAtNpc(freeAfter, detail);
                                if (sold && freeAfter > 0) {
                                    sellFailures = 0;
                                    UpdateFreeBagSpace(freeAfter);
                                    sellingTrip = false;
                                    outsideTarget = false;
                                    awaitingRideCheck = false;
                                    mountFightUntil = Clock::time_point{};
                                    nextBagCheck = now + std::chrono::minutes(config_.bagCheckMinutes);
                                    nextNavigate = now;
                                    bestDistance = LLONG_MAX;
                                    lastProgress = now;
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
                                        sellRetryAfter = now + std::chrono::seconds(8);
                                        UpdateLive(true, L"BÁN ĐỒ TẠM DỪNG AN TOÀN",
                                                   detail + L" • lỗi " +
                                                   std::to_wstring(sellFailures) + L"/3");
                                    }
                                }
                            } else {
                                UpdateLive(true, L"ĐÃ TỚI NPC", L"Đang chờ thử lại chuỗi bán an toàn");
                            }
                        } else if (!sellingTrip && !healingTrip && AtTarget(live)) {
                            outsideTarget = false;
                            awaitingRideCheck = false;
                            mountFightUntil = Clock::time_point{};
                            bestDistance = 0;
                            if (live.autoPathing) {
                                std::uint64_t ignored = 0;
                                Remote(rva::AutoPathStop, autoPathManager_, 0, 0, 0, ignored);
                            }
                            if (live.riding) {
                                // Keep arrival identical to SelectedSkill: down-mount first.  Never open
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
                                    if (!buffCycle && config_.activation == TrainActivationMode::AutoMenu &&
                                        live.autoFight) {
                                        std::wstring stopDetail;
                                        const bool stopped = ClickInternalAutoStop(stopDetail);
                                        nextBuffAction = now + std::chrono::milliseconds(350);
                                        UpdateLive(true, L"CHUẨN BỊ BUFF",
                                                   stopped ? L"Đã AUTO → Dừng trước chu kỳ buff"
                                                           : L"Chưa tắt được AUTO; sẽ thử lại trước khi buff");
                                    } else {
                                        if (!buffCycle) {
                                            buffCycle = true;
                                            buffIndex = 0;
                                            nextBuffAction = now;
                                            trainTriggered = false;
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
                                        case TrainActivationMode::SelectedSkill:
                                            sent = TriggerSelectedSkill(detail);
                                            nextTrainAction = now +
                                                std::chrono::milliseconds(sent ? 900 : 700);
                                            break;
                                        case TrainActivationMode::AutoMenu:
                                            if (autoCombatConfirmed && live.autoFight) {
                                                sent = true;
                                                detail = L"AUTO → Đánh quái đã được xác minh sau khi tới bãi";
                                            } else if (!autoCombatConfirmed && live.autoFight) {
                                                // A true flag inherited from an earlier/dead/navigation state is not
                                                // proof that the current AUTO combat loop was armed at this spot.
                                                // Clear it first, then activate once from a clean state next cycle.
                                                std::wstring stopDetail;
                                                const bool stopped = ClickInternalAutoStop(stopDetail);
                                                sent = false;
                                                detail = stopped
                                                    ? L"Đã xóa trạng thái AUTO cũ trước khi bật lại tại bãi"
                                                    : L"AutoFight đang ON từ trạng thái cũ nhưng chưa Dừng được • " + stopDetail;
                                                nextTrainAction = now + std::chrono::seconds(stopped ? 1 : 6);
                                            } else {
                                                sent = TriggerAutoMenu(detail);
                                                if (sent) {
                                                    autoCombatConfirmed = true;
                                                    autoActivationFailures = 0;
                                                } else {
                                                    autoCombatConfirmed = false;
                                                    ++autoActivationFailures;
                                                }
                                                // Avoid hammering the menu/UI if activation cannot be resolved.
                                                nextTrainAction = now + std::chrono::seconds(
                                                    sent ? 2 : (autoActivationFailures >= 3 ? 15 : 5));
                                            }
                                            break;
                                    }
                                    trainTriggered = trainTriggered || sent;
                                    const bool confirmed =
                                        config_.activation == TrainActivationMode::SelectedSkill
                                            ? trainTriggered
                                            : autoCombatConfirmed && live.autoFight;
                                    UpdateLive(true, confirmed ? L"ĐANG TRAIN"
                                                               : L"Đang xác nhận train",
                                               target_.name + L" • " + detail);
                                } else {
                                    if (config_.activation == TrainActivationMode::AutoMenu &&
                                        autoCombatConfirmed && !live.autoFight) {
                                        autoCombatConfirmed = false;
                                        nextTrainAction = now;
                                    }
                                    const bool confirmed =
                                        config_.activation == TrainActivationMode::SelectedSkill
                                            ? trainTriggered
                                            : autoCombatConfirmed && live.autoFight;
                                    UpdateLive(true, confirmed ? L"ĐANG TRAIN"
                                                               : L"Đang xác nhận train",
                                               target_.name);
                                }
                            }
                        } else {
                            autoCombatConfirmed = false;
                            const Spot destination = healingTrip
                                ? Spot{healNpc_.name, healNpc_.mapID, healNpc_.x, healNpc_.y}
                                : sellingTrip
                                    ? Spot{sellNpc_.name, sellNpc_.mapID, sellNpc_.x, sellNpc_.y}
                                    : target_;
                            trainTriggered = false;
                            buffCycle = false;
                            nextTrainAction = now;
                            if (!outsideTarget) {
                                // Stop combat/path once before returning to the destination.  From this
                                // point until arrival, navigation owns the session and AUTO may not start.
                                DisableActions();
                                outsideTarget = true;
                                nextNavigate = now;
                                nextAutoStopRetry = now + std::chrono::seconds(2);
                            }
                            bool navigationAutoPending = false;
                            if (config_.activation == TrainActivationMode::AutoMenu && live.autoFight) {
                                // Strict serialization: never let AutoPath/portal/mount commands run while
                                // an AUTO combat state is still active.  Stop path first, then retry AUTO →
                                // Dừng at a low rate.  This removes the command overlap seen in v0.8.1.
                                navigationAutoPending = true;
                                StopPathOnly();
                                if (now >= nextAutoStopRetry) {
                                    std::wstring stopDetail;
                                    const bool stopped = ClickInternalAutoStop(stopDetail);
                                    nextAutoStopRetry = now + std::chrono::seconds(stopped ? 1 : 5);
                                    UpdateLive(true, L"ĐANG TÁCH AUTO KHỎI ĐI MAP",
                                               stopped ? L"Đã AUTO → Dừng; chờ state OFF trước khi đi tiếp"
                                                       : L"Chưa Dừng được AUTO • " + stopDetail);
                                } else {
                                    UpdateLive(true, L"ĐANG TÁCH AUTO KHỎI ĐI MAP",
                                               L"AutoFight còn ON • chưa gửi AutoPath/ngựa/cổng song song");
                                }
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
                            const bool portalConfirmationReady =
                                mountFightUntil == Clock::time_point{} &&
                                live.mapID != destination.mapID && mountSettled && stalled &&
                                live.messageBoxVisible && now >= nextConfirm;
                            if (navigationAutoPending) {
                                // Status already updated above.  Wait for a fresh scan showing AutoFight=OFF.
                            } else if (portalConfirmationReady) {
                                // Combat was already stopped when this navigation trip began.
                                // Do not reopen/scan AUTO while a portal UI is being rebuilt.
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
                            } else if (mountFightUntil > now) {
                                std::wstring fightDetail;
                                if (now >= nextMountFightAction) {
                                    // Navigation/mount recovery must never open the AUTO menu.  Reuse the
                                    // proven selected-skill cleanup path when a skill is configured; AUTO
                                    // activation is reserved strictly for the final train spot.
                                    bool sent = false;
                                    if (config_.skillID > 0)
                                        sent = TriggerSelectedSkill(fightDetail);
                                    else
                                        fightDetail = L"Không có skill dọn quái; chỉ chờ rồi thử ngựa lại";
                                    nextMountFightAction = now +
                                        std::chrono::milliseconds(sent ? 900 : 700);
                                }
                                const auto remain = std::chrono::duration_cast<std::chrono::seconds>(
                                    mountFightUntil - now).count();
                                UpdateLive(true, L"DỌN QUÁI TRƯỚC KHI LÊN NGỰA",
                                           L"Sau 4 giây vẫn chưa lên ngựa • tiếp tục khoảng " +
                                           std::to_wstring(std::max<long long>(1, remain)) +
                                           L" giây rồi thử lại" +
                                           (fightDetail.empty() ? L"" : L" • " + fightDetail));
                            } else if (mountFightUntil != Clock::time_point{}) {
                                mountFightUntil = Clock::time_point{};
                                nextRideDecision = now;
                                nextNavigate = now;
                                UpdateLive(true, L"Đang ra bãi",
                                           L"Kết thúc pha dọn quái • chuẩn bị thử lên ngựa lại");
                            } else if (awaitingRideCheck && now < nextRideDecision) {
                                UpdateLive(true, L"Đang ra bãi",
                                           L"Đã gọi ngựa • chờ 4 giây kiểm tra lại");
                            } else if (!live.riding) {
                                if (awaitingRideCheck) {
                                    awaitingRideCheck = false;
                                    mountFightUntil = now + std::chrono::seconds(10);
                                    nextMountFightAction = now;
                                    nextRideDecision = mountFightUntil;
                                    nextNavigate = nextRideDecision;
                                    std::wstring fightDetail;
                                    bool cleanupSent = false;
                                    if (config_.skillID > 0)
                                        cleanupSent = TriggerSelectedSkill(fightDetail);
                                    else
                                        fightDetail = L"Không có skill dọn quái; không mở AUTO khi đang đi map";
                                    nextMountFightAction = now +
                                        std::chrono::milliseconds(cleanupSent ? 900 : 700);
                                    UpdateLive(true, L"Đang ra bãi",
                                               L"Sau 4 giây vẫn chưa lên ngựa • dọn/chờ 10 giây rồi thử lại" +
                                               (fightDetail.empty() ? L"" : L" • " + fightDetail));
                                } else {
                                    const bool sent = ToggleRide(true);
                                    awaitingRideCheck = sent;
                                    nextRideDecision = now + std::chrono::seconds(4);
                                    nextNavigate = nextRideDecision;
                                    mountFightUntil = Clock::time_point{};
                                    UpdateLive(true, L"Đang ra bãi",
                                               sent ? L"Đã gọi ngựa • chờ 4 giây"
                                                    : L"Gọi ngựa thất bại • thử lại sau 4 giây");
                                }
                            } else if (now >= nextNavigate) {
                                awaitingRideCheck = false;
                                nextRideDecision = Clock::time_point{};
                                const bool sent = StartPathTo(destination);
                                UpdateLive(true, L"Đang ra bãi",
                                           sent ? (config_.mode == NavigationMode::ChatPing
                                                       ? L"Đã ping tọa độ qua chat"
                                                       : L"Đã gửi AutoPath trực tiếp")
                                                : L"Gửi lệnh đi bãi thất bại; sẽ thử lại");
                                nextNavigate = now +
                                    std::chrono::seconds(config_.retrySeconds);
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
    std::uint64_t assemblyImage_ = 0;
    std::uint64_t uiObjectClass_ = 0;
    std::uint64_t uiButtonClass_ = 0;
    std::uint64_t uiToggleClass_ = 0;
    std::uint64_t uiRectTransformClass_ = 0;
    std::uint64_t uiInputClass_ = 0;
    std::uint64_t uiInstancesField_ = 0;
    std::uint64_t systemObjectClass_ = 0;
    std::map<std::uint64_t, bool> uiButtonClassCache_;
    std::map<std::uint64_t, bool> uiToggleClassCache_;
    std::map<std::uint64_t, bool> uiRectClassCache_;
    std::map<std::uint64_t, bool> uiInputClassCache_;
    std::atomic<bool> running_{false};
    std::thread worker_;
    mutable std::mutex stateLock_;
    LiveState state_;
    std::mutex operation_;
};

enum ControlIdV5 {
    V5_GAMES = 3001, V5_REFRESH, V5_PROBE, V5_SPOT_NAME, V5_SAVE_SPOT,
    V5_DELETE_SPOT, V5_SPOTS, V5_MODE, V5_TOLERANCE, V5_RETRY,
    V5_CAL_CHAT, V5_ACTIVATION, V5_SKILL, V5_SCAN_SKILLS, V5_START,
    V5_STOP, V5_STATE, V5_COORDS, V5_FLAGS, V5_DETAIL, V5_TAB,
    V5_AUTO_SELL, V5_BAG_MINUTES, V5_SELL_NPC, V5_SAVE_NPC,
    V5_HEAL_AT_START, V5_HEAL_NPC, V5_SAVE_HEAL_NPC,
    V5_AUTO_BUFF, V5_BUFF_SKILLS, V5_AUTO_CHAT, V5_CHAT_MESSAGE, V5_CHAT_MINUTES
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

        Label(L"THẦN LONG MOBILE • AUTO TRAIN v0.8.2", 16, 5, 650, 31, titleFont_);
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

        Label(L"CÁCH ĐI BÃI", 16, 468, 105, 25, boldFont_);
        modeCombo_ = Make(WC_COMBOBOXW, L"",
                          CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                          122, 466, 248, 145, V5_MODE);
        SendMessageW(modeCombo_, CB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(L"AutoPath nội bộ (khuyên dùng)"));
        SendMessageW(modeCombo_, CB_ADDSTRING, 0,
                     reinterpret_cast<LPARAM>(L"Ping tọa độ qua chat"));
        Label(L"Sai số", 382, 468, 53, 25, smallFont_);
        tolerance_ = Make(L"EDIT", L"120", WS_BORDER | ES_NUMBER | WS_TABSTOP,
                          435, 466, 60, 27, V5_TOLERANCE, font_, WS_EX_CLIENTEDGE);
        Label(L"Lặp sau", 508, 468, 61, 25, smallFont_);
        retry_ = Make(L"EDIT", L"12", WS_BORDER | ES_NUMBER | WS_TABSTOP,
                      570, 466, 55, 27, V5_RETRY, font_, WS_EX_CLIENTEDGE);
        Label(L"giây", 631, 468, 35, 25, smallFont_);
        Button(L"HIỆU CHỈNH CHAT", 845, 465, 199, 29, V5_CAL_CHAT);

        Label(L"BẬT TRAIN", 16, 503, 105, 25, boldFont_);
        activationCombo_ = Make(WC_COMBOBOXW, L"",
                                CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
                                122, 501, 260, 160, V5_ACTIVATION);
        int activationItem = static_cast<int>(SendMessageW(
            activationCombo_, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(L"Dùng skill đã chọn")));
        SendMessageW(activationCombo_, CB_SETITEMDATA, activationItem,
                     static_cast<LPARAM>(TrainActivationMode::SelectedSkill));
        activationItem = static_cast<int>(SendMessageW(
            activationCombo_, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(L"AUTO → Đánh quái")));
        SendMessageW(activationCombo_, CB_SETITEMDATA, activationItem,
                     static_cast<LPARAM>(TrainActivationMode::AutoMenu));
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

        autoChatCheck_ = Make(L"BUTTON", L"Auto chat", BS_AUTOCHECKBOX | WS_TABSTOP,
                              16, 684, 95, 28, V5_AUTO_CHAT, font_);
        chatMessage_ = Make(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL | WS_TABSTOP,
                            116, 684, 600, 27, V5_CHAT_MESSAGE, font_, WS_EX_CLIENTEDGE);
        Label(L"mỗi", 726, 684, 32, 27, smallFont_);
        chatMinutes_ = Make(L"EDIT", L"5", WS_BORDER | ES_NUMBER | WS_TABSTOP,
                            758, 684, 44, 27, V5_CHAT_MINUTES, font_, WS_EX_CLIENTEDGE);
        Label(L"phút", 807, 684, 36, 27, smallFont_);

        detailLabel_ = Label(L"Sẵn sàng.", 16, 718, 1028, 34, font_, V5_DETAIL);
        startButton_ = Button(L"▶  BẮT ĐẦU CỬA SỔ ĐÃ TICK",
                              16, 758, 646, 52, V5_START);
        stopButton_ = Button(L"■  TẠM DỪNG CỬA SỔ ĐÃ TICK",
                             675, 758, 369, 52, V5_STOP);

        buildingPage_ = 2;
        Make(L"STATIC", L"GIỚI THIỆU", SS_CENTER | SS_CENTERIMAGE,
             150, 145, 780, 46, 0, titleFont_);
        Make(L"STATIC", L"Phần mềm được thiết kế bởi Thắng Nguyễn - ĐỒ LONG",
             SS_CENTER | SS_CENTERIMAGE, 150, 205, 780, 48, 0, boldFont_);
        Make(L"STATIC", L"Phiên bản 0.8.2",
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
        runtime.config.mode = SendMessageW(modeCombo_, CB_GETCURSEL, 0, 0) == 1
            ? NavigationMode::ChatPing : NavigationMode::DirectAutoPath;
        runtime.config.tolerance = std::clamp(
            _wtoi(Text(tolerance_).c_str()), 20, 2000);
        runtime.config.retrySeconds = std::clamp(
            _wtoi(Text(retry_).c_str()), 5, 60);
        const int activationIndex = static_cast<int>(SendMessageW(
            activationCombo_, CB_GETCURSEL, 0, 0));
        const LRESULT activationData = activationIndex == CB_ERR ? CB_ERR
            : SendMessageW(activationCombo_, CB_GETITEMDATA, activationIndex, 0);
        runtime.config.activation =
            activationData == static_cast<LRESULT>(TrainActivationMode::SelectedSkill)
                ? TrainActivationMode::SelectedSkill : TrainActivationMode::AutoMenu;
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
        runtime.config.autoChat = SendMessageW(autoChatCheck_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        runtime.config.autoChatMessage = Text(chatMessage_);
        runtime.config.autoChatMinutes = std::clamp(_wtoi(Text(chatMinutes_).c_str()), 1, 180);
        SetWindowTextW(chatMinutes_, std::to_wstring(runtime.config.autoChatMinutes).c_str());
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
            SendMessageW(autoChatCheck_, BM_SETCHECK, BST_UNCHECKED, 0);
            SetWindowTextW(chatMessage_, L"");
            SetWindowTextW(chatMinutes_, L"5");
            SendMessageW(buffSkillList_, LB_RESETCONTENT, 0, 0);
            UpdateState(LiveState{});
            loadingUi_ = false;
            return;
        }
        SendMessageW(modeCombo_, CB_SETCURSEL,
                     static_cast<int>(runtime->config.mode), 0);
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
        SendMessageW(autoChatCheck_, BM_SETCHECK,
                     runtime->config.autoChat ? BST_CHECKED : BST_UNCHECKED, 0);
        SetWindowTextW(chatMessage_, runtime->config.autoChatMessage.c_str());
        SetWindowTextW(chatMinutes_, std::to_wstring(runtime->config.autoChatMinutes).c_str());
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

    std::optional<NormalizedPoint> CapturePoint(const GameProcess& game,
                                                 const std::wstring& instruction) {
        if (!game.window || !IsWindow(game.window)) {
            MessageBoxW(window_, L"Không tìm thấy cửa sổ hiển thị của game.", kTitle,
                        MB_OK | MB_ICONERROR);
            return std::nullopt;
        }
        MessageBoxW(window_, (instruction +
            L"\n\nSau khi bấm OK, chuyển sang game và click đúng vị trí. Nhấn ESC để hủy.").c_str(),
            kTitle, MB_OK | MB_ICONINFORMATION);
        while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) Sleep(20);
        ActivateGame(game.window);
        const ULONGLONG begin = GetTickCount64();
        while (GetTickCount64() - begin < 20000) {
            if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
                SetForegroundWindow(window_);
                return std::nullopt;
            }
            if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
                POINT cursor{};
                GetCursorPos(&cursor);
                POINT client = cursor;
                ScreenToClient(game.window, &client);
                RECT rect{};
                GetClientRect(game.window, &rect);
                while (GetAsyncKeyState(VK_LBUTTON) & 0x8000) Sleep(20);
                SetForegroundWindow(window_);
                if (rect.right <= 0 || rect.bottom <= 0 || client.x < 0 || client.y < 0 ||
                    client.x >= rect.right || client.y >= rect.bottom) {
                    MessageBoxW(window_, L"Điểm click nằm ngoài vùng game; chưa lưu.", kTitle,
                                MB_OK | MB_ICONWARNING);
                    return std::nullopt;
                }
                return NormalizedPoint{
                    static_cast<int>((static_cast<long long>(client.x) * 10000) / rect.right),
                    static_cast<int>((static_cast<long long>(client.y) * 10000) / rect.bottom)};
            }
            Sleep(20);
        }
        SetForegroundWindow(window_);
        MessageBoxW(window_, L"Hết 20 giây nhưng chưa nhận được click.", kTitle,
                    MB_OK | MB_ICONWARNING);
        return std::nullopt;
    }

    void CalibrateChat() {
        SaveSelectedUi();
        GameRuntimeV5* runtime = SelectedRuntime();
        if (!runtime) {
            ShowDetail(L"Chưa chọn cửa sổ game.");
            return;
        }
        auto clear = CapturePoint(runtime->game,
            L"Mở chat trước, rồi click nút XÓA LỊCH SỬ CHAT.");
        if (!clear) return;
        auto input = CapturePoint(runtime->game,
            L"Click vào giữa Ô NHẬP NỘI DUNG CHAT.");
        if (!input) return;
        auto latest = CapturePoint(runtime->game,
            L"Gửi thử tọa độ, rồi click vào DÒNG TỌA ĐỘ MỚI NHẤT.");
        if (!latest) return;
        runtime->config.chatClear = *clear;
        runtime->config.chatInput = *input;
        runtime->config.chatLatestCoordinate = *latest;
        SaveRuntime(*runtime);
        ShowDetail(L"Đã lưu ba điểm chat theo tỷ lệ riêng của cửa sổ đang chọn.");
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
                         npc.name + L" • ID " + std::to_wstring(npc.roleID) + L" • Map " +
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
        if (runtime.config.mode == NavigationMode::ChatPing &&
            (!runtime.config.chatClear.Valid() || !runtime.config.chatInput.Valid() ||
             !runtime.config.chatLatestCoordinate.Valid())) {
            error = L"PID " + std::to_wstring(runtime.game.pid) +
                    L": chưa hiệu chỉnh đủ ba điểm chat";
            return false;
        }
        if (runtime.config.activation == TrainActivationMode::SelectedSkill &&
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
        if (runtime.config.autoChat && runtime.config.autoChatMessage.empty()) {
            error = L"PID " + std::to_wstring(runtime.game.pid) + L": Auto chat chưa có nội dung";
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
                    case V5_CAL_CHAT: CalibrateChat(); return 0;
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
    HWND modeCombo_ = nullptr;
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
    HWND autoChatCheck_ = nullptr;
    HWND chatMessage_ = nullptr;
    HWND chatMinutes_ = nullptr;
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
