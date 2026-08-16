#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <memory>
#include "protocol.h"
#include "route_logic.h"
#include "rotation_logic.h"

using namespace cleanroute;
using namespace cleanroute_logic;
using namespace cleanroute_rotation;

namespace {

constexpr wchar_t kTitle[] = L"Thần Long Clean Route v1.5.10 • Internal Confirm + Revive v0.8.7";
constexpr wchar_t kGameModule[] = L"GameAssembly.dll";
constexpr UINT_PTR kTimer = 1;
constexpr int kCaptureHotkeyId = 9001;
constexpr int kPauseHotkeyId = 9002;
constexpr DWORD kClientStableResumeMs = 2000;
constexpr DWORD kBridgeNudgeMs = 750;
constexpr DWORD kReadFailLogIntervalMs = 2000;
constexpr UINT kWindowResponsiveProbeMs = 120;
constexpr DWORD kMouseSettleMs = 100;
constexpr DWORD kMouseHoldMs = 140;
constexpr DWORD kMouseAfterMs = 90;
constexpr DWORD kTrainPositionCheckMs = 180000;
constexpr DWORD kAutoFightRecheckMs = 60000;
constexpr DWORD kMountRetryWaitMs = 5000;
constexpr DWORD kFootWalkMaxMs = 15000;
constexpr int kUnderworldMapId = 87;
constexpr DWORD kUnderworldStopStepMs = 700;
constexpr DWORD kUnderworldVerifyMs = 1200;
constexpr int kUnderworldMaxStopAttempts = 3;
constexpr DWORD kRouteOwnershipStopRetryMs = 1200;
constexpr int kRouteOwnershipStopMaxAttempts = 3;
constexpr int kRotateDeathLimitDefault = 10;
constexpr int kRotateDeathWindowMinDefault = 10;
constexpr int kRotateNoFullBagMinDefault = 15;
constexpr int kRotateDeathLimitMin = 1;
constexpr int kRotateDeathLimitMax = 100;
constexpr int kRotateWindowMin = 1;
constexpr int kRotateWindowMax = 180;






constexpr int IDC_CLIENT_LIST = 100;
constexpr int IDC_SCAN = 101;
constexpr int IDC_START_CHECKED = 102;
constexpr int IDC_STOP_CHECKED = 103;
constexpr int IDC_SELECTED = 104;
constexpr int IDC_LIVE = 105;
constexpr int IDC_TARGET_NAME = 110;
constexpr int IDC_SAVE_TARGET = 111;
constexpr int IDC_TARGET_TEXT = 112;
constexpr int IDC_TOLERANCE = 113;
constexpr int IDC_SPOT_COMBO = 114;
constexpr int IDC_DELETE_SPOT = 115;
constexpr int IDC_ENABLE_REVIVE = 120;
constexpr int IDC_ENABLE_FIGHT = 122;
constexpr int IDC_ENABLE_SELL = 123;
constexpr int IDC_SELL_NPC = 124;
constexpr int IDC_SELL_NPC_X = 125;
constexpr int IDC_SELL_NPC_Y = 126;
constexpr int IDC_SELL_NPC_CAPTURE = 127;
constexpr int IDC_SELL_NPC_POS = 128;
constexpr int IDC_CAPTURE_AUTO = 132;
constexpr int IDC_CAPTURE_ATTACK = 133;
constexpr int IDC_CAPTURE_STOP_AUTO_1 = 134;
constexpr int IDC_CAPTURE_STOP_AUTO_2 = 135;
constexpr int IDC_POINT_AUTO = 142;
constexpr int IDC_POINT_ATTACK = 143;
constexpr int IDC_POINT_STOP_AUTO_1 = 144;
constexpr int IDC_POINT_STOP_AUTO_2 = 145;
constexpr int IDC_TEST_AUTO = 152;
constexpr int IDC_TEST_ATTACK = 153;
constexpr int IDC_TEST_STOP_AUTO_1 = 154;
constexpr int IDC_TEST_STOP_AUTO_2 = 155;
constexpr int IDC_SELL_MACRO_LIST = 170;
constexpr int IDC_SELL_ADD = 171;
constexpr int IDC_SELL_DELETE = 172;
constexpr int IDC_SELL_DESC = 173;
constexpr int IDC_SELL_DELAY = 174;
constexpr int IDC_SELL_REPEAT = 175;
constexpr int IDC_SELL_SAVE = 176;
constexpr int IDC_SELL_CAPTURE = 177;
constexpr int IDC_SELL_TEST = 178;
constexpr int IDC_LOG = 160;
constexpr int IDC_SHUTDOWN_ENABLE = 180;
constexpr int IDC_SHUTDOWN_HOUR = 181;
constexpr int IDC_SHUTDOWN_MINUTE = 182;
constexpr int IDC_SHUTDOWN_APPLY = 183;
constexpr int IDC_SHUTDOWN_STATUS = 184;
constexpr int IDC_ROTATION_LIST = 186;
constexpr int IDC_ROTATE_DEATH_LIMIT = 187;
constexpr int IDC_ROTATE_DEATH_WINDOW = 188;
constexpr int IDC_ROTATE_NO_BAG = 189;

constexpr std::array<const wchar_t*, 4> kClickKeys = {
    L"AutoMenu", L"Attack", L"StopAuto1", L"StopAuto2"
};
constexpr std::array<const wchar_t*, 4> kClickLabels = {
    L"AUTO", L"ĐÁNH QUÁI", L"DỪNG AUTO 1", L"DỪNG AUTO 2"
};

enum class ClickSlot : int {
    None = -1,
    AutoMenu = 0,
    Attack = 1,
    StopAuto1 = 2,
    StopAuto2 = 3,
};

struct ClickPoint {
    int x = 0;
    int y = 0;
    int baseW = 0;
    int baseH = 0;
    bool valid = false;
};

struct SellMacroStep {
    std::wstring description;
    ClickPoint point{};
    int delayMs = 600;
    int repeat = 1;
};

struct SellNpcPreset {
    const wchar_t* name;
    int mapID;
    int npcID;
};

constexpr std::array<SellNpcPreset, 2> kSellNpcs = {{
    {L"Mã Kiêu Minh • M5 • ID 373", 5, 373},
    {L"Dược Đại Phu • Hỏa Diệm Sơn M55 • ID 279", 55, 279},
}};

struct SellNpcPosition {
    int x = 0;
    int y = 0;
    bool valid = false;
};

struct TargetProfile {
    std::wstring name;
    int mapID = 0;
    int x = 0;
    int y = 0;
    bool valid = false;
};

struct AccountProfile {
    std::wstring section;
    std::wstring selectedSpot;
    int tolerance = 120;
    bool enableRevive = true;
    bool enableFight = true;
    bool enableSell = false;
    int sellNpcPreset = 0;
    std::vector<std::wstring> rotationSpots{};
    int rotateDeathLimit = kRotateDeathLimitDefault;
    int rotateDeathWindowMin = kRotateDeathWindowMinDefault;
    int rotateNoFullBagMin = kRotateNoFullBagMinDefault;
    TargetProfile target{};
    std::array<ClickPoint, 4> points{};
    std::vector<SellMacroStep> sellMacro{};
};

struct GameClient {
    DWORD pid = 0;
    DWORD threadId = 0;
    HWND window = nullptr;
    std::wstring title;
};

struct RuntimeState {
    bool running = false;
    std::wstring status = L"Đã dừng";
    int qualifiedMap = 0;
    int candidateMap = 0;
    int candidateCount = 0;
    DWORD lastActionTick = 0;
    Action lastAction = Action::Wait;

    DWORD deadSinceTick = 0;
    int revivePhase = 0;
    DWORD revivePhaseTick = 0;
    DWORD lastReviveClickTick = 0;
    DWORD lastConfirmClickTick = 0;
    DWORD lastRealInputTick = 0;

    int lastObservedMap = 0;
    int lastObservedX = 0;
    int lastObservedY = 0;
    DWORD lastMovementTick = 0;
    DWORD confirmUiFirstSeenTick = 0;
    DWORD confirmStopPathTick = 0;
    bool crossMapSeenAutoPath = false;
    DWORD stallSinceTick = 0;
    int confirmAttempts = 0;
    DWORD suppressRouteSinceTick = 0;

    int fightPhase = 0;
    DWORD fightPhaseTick = 0;
    int fightAttempts = 0;
    bool wasAtTarget = false;

    // Once AUTO fight is confirmed at the training spot, position is intentionally
    // checked only every 3 minutes. Death/bag state are still observed every tick.
    bool trainPositionMonitorArmed = false;
    DWORD lastTrainPositionCheckTick = 0;
    DWORD lastAutoFightCheckTick = 0;
    int trainRecoveryPhase = 0;
    DWORD trainRecoveryTick = 0;
    int trainRecoveryStopAttempts = 0;

    // Shared robust-travel helper state: mount #1 -> 5s -> mount #2 -> 5s ->
    // foot AutoPath for at most 15s -> repeat mount cycle.
    int travelMountAttempts = 0;
    DWORD travelMountTick = 0;
    bool travelFootFallback = false;
    DWORD travelFootTick = 0;

    bool crossMapRouteArmed = false;
    bool crossMapRouteMoved = false;

    int sellPhase = 0;
    DWORD sellPhaseTick = 0;
    int sellStopAttempts = 0;
    int sellOpenAttempts = 0;
    int sellMacroIndex = 0;
    int sellMacroRepeatDone = 0;
    DWORD sellMacroNextTick = 0;
    int sellMacroPass = 0;
    int sellLastFreeBag = -1;
    DWORD sellBagStableSince = 0;
    bool sellTriggeredByFullBag = false;

    // Global per-PID transition/unresponsive safety gate. While active, no mutable
    // gameplay/window action may be dispatched. Read-only state polling continues
    // until the client is continuously healthy for kClientStableResumeMs.
    bool clientFreezeActive = false;
    DWORD clientFreezeSinceTick = 0;
    DWORD clientStableSinceTick = 0;
    int readStateFailStreak = 0;
    DWORD lastReadFailureLogTick = 0;

    // Map 87 = Địa Phủ. After revive, the built-in AutoFight may remain ON and
    // compete with return-to-train pathing. Stop it with the two user-captured
    // stop-Auto clicks and verify AutoFight OFF before any route resumes.
    int underworldStopPhase = 0;
    DWORD underworldStopTick = 0;
    int underworldStopAttempts = 0;
    bool underworldGuardLogged = false;

    // A tool-runtime reset does not stop the game client's real AutoPath. After a
    // fresh Start or revive cold-start, reacquire route ownership by forcing any
    // stale AutoPath OFF and verifying it before a new StartPath may arm Confirm.
    bool routeOwnershipResetPending = false;
    DWORD routeOwnershipStopTick = 0;
    int routeOwnershipStopAttempts = 0;
    bool routeOwnershipResetLogged = false;
};

template <typename T>
bool ResolveProc(HMODULE module, const char* name, T& out) {
    out = nullptr;
    FARPROC raw = GetProcAddress(module, name);
    if (!raw) return false;
    static_assert(sizeof(raw) == sizeof(out), "pointer size mismatch");
    std::memcpy(&out, &raw, sizeof(out));
    return out != nullptr;
}

std::wstring ExeDir() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, _countof(path));
    if (wchar_t* slash = wcsrchr(path, L'\\')) *slash = 0;
    return path;
}

std::wstring LegacyConfigPath() { return ExeDir() + L"\\ThanLongCleanRoute.accounts.ini"; }

std::wstring ConfigDir() {
    wchar_t localAppData[4096]{};
    const DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, _countof(localAppData));
    if (n > 0 && n < _countof(localAppData)) {
        std::wstring dir = std::wstring(localAppData) + L"\\ThanLongCleanRoute";
        (void)CreateDirectoryW(dir.c_str(), nullptr);
        return dir;
    }
    return ExeDir();
}

std::wstring ConfigPath() {
    static const std::wstring path = ConfigDir() + L"\\ThanLongCleanRoute.accounts.ini";
    return path;
}

void MigrateLegacyConfigIfNeeded() {
    const std::wstring current = ConfigPath();
    const std::wstring legacy = LegacyConfigPath();
    if (current == legacy) return;
    if (GetFileAttributesW(current.c_str()) != INVALID_FILE_ATTRIBUTES) return;
    if (GetFileAttributesW(legacy.c_str()) == INVALID_FILE_ATTRIBUTES) return;
    (void)CopyFileW(legacy.c_str(), current.c_str(), TRUE);
}

void FlushIni() {
    (void)WritePrivateProfileStringW(nullptr, nullptr, nullptr, ConfigPath().c_str());
}

void EnsureUnicodeIni() {
    const std::wstring path = ConfigPath();
    if (GetFileAttributesW(path.c_str()) != INVALID_FILE_ATTRIBUTES) return;
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    const BYTE bom[2] = {0xFF, 0xFE};
    DWORD done = 0;
    (void)WriteFile(h, bom, 2, &done, nullptr);
    CloseHandle(h);
}

int ReadIniInt(const std::wstring& section, const std::wstring& key, int fallback) {
    return static_cast<int>(GetPrivateProfileIntW(section.c_str(), key.c_str(), fallback, ConfigPath().c_str()));
}

void WriteIniInt(const std::wstring& section, const std::wstring& key, int value) {
    wchar_t text[32]{};
    wsprintfW(text, L"%d", value);
    WritePrivateProfileStringW(section.c_str(), key.c_str(), text, ConfigPath().c_str());
}

std::wstring ReadIniText(const std::wstring& section, const std::wstring& key) {
    wchar_t text[512]{};
    GetPrivateProfileStringW(section.c_str(), key.c_str(), L"", text, _countof(text), ConfigPath().c_str());
    return text;
}

void WriteIniText(const std::wstring& section, const std::wstring& key, const std::wstring& value) {
    WritePrivateProfileStringW(section.c_str(), key.c_str(), value.c_str(), ConfigPath().c_str());
}

void DeleteIniKey(const std::wstring& section, const std::wstring& key) {
    // Passing nullptr as the value deletes the key. v1.5.10 uses this only to
    // migrate away the removed coordinate/timer Confirm + coordinate Revive fields.
    WritePrivateProfileStringW(section.c_str(), key.c_str(), nullptr, ConfigPath().c_str());
}

void RemoveLegacyConfirmReviveKeys(const std::wstring& section) {
    static constexpr const wchar_t* kLegacyKeys[] = {
        L"EnableConfirm", L"ConfirmIntervalSec",
        L"ConfirmX", L"ConfirmY", L"ConfirmW", L"ConfirmH",
        L"ReviveX", L"ReviveY", L"ReviveW", L"ReviveH"
    };
    for (const wchar_t* key : kLegacyKeys) DeleteIniKey(section, key);
}

std::array<SellNpcPosition, kSellNpcs.size()> LoadSharedSellNpcPositions() {
    std::array<SellNpcPosition, kSellNpcs.size()> positions{};
    const std::wstring section = L"SellNpcPositions";
    for (std::size_t i = 0; i < kSellNpcs.size(); ++i) {
        const std::wstring prefix = L"SellNpcPos_" + std::to_wstring(i) + L"_";
        SellNpcPosition& pos = positions[i];
        pos.x = ReadIniInt(section, prefix + L"X", -1);
        pos.y = ReadIniInt(section, prefix + L"Y", -1);
        pos.valid = pos.x >= 0 && pos.y >= 0 && ReadIniInt(section, prefix + L"Valid", 0) != 0;
    }
    return positions;
}

void SaveSharedSellNpcPositions(const std::array<SellNpcPosition, kSellNpcs.size()>& positions) {
    EnsureUnicodeIni();
    const std::wstring section = L"SellNpcPositions";
    for (std::size_t i = 0; i < kSellNpcs.size(); ++i) {
        const std::wstring prefix = L"SellNpcPos_" + std::to_wstring(i) + L"_";
        const SellNpcPosition& pos = positions[i];
        WriteIniInt(section, prefix + L"X", pos.valid ? pos.x : -1);
        WriteIniInt(section, prefix + L"Y", pos.valid ? pos.y : -1);
        WriteIniInt(section, prefix + L"Valid", pos.valid ? 1 : 0);
    }
    FlushIni();
}

AccountProfile LoadProfile(const std::wstring& section) {
    AccountProfile p{};
    p.section = section;
    p.tolerance = ReadIniInt(section, L"Tolerance", 120);
    if (p.tolerance < 20) p.tolerance = 20;
    if (p.tolerance > 2000) p.tolerance = 2000;
    p.enableRevive = ReadIniInt(section, L"EnableRevive", 1) != 0;
    p.enableFight = ReadIniInt(section, L"EnableFight", 1) != 0;
    p.enableSell = ReadIniInt(section, L"EnableSell", 0) != 0;
    p.sellNpcPreset = ReadIniInt(section, L"SellNpcPreset", 0);
    if (p.sellNpcPreset < 0 || p.sellNpcPreset >= static_cast<int>(kSellNpcs.size())) p.sellNpcPreset = 0;
    p.selectedSpot = ReadIniText(section, L"SelectedSpot");
    p.rotateDeathLimit = ReadIniInt(section, L"RotateDeathLimit", kRotateDeathLimitDefault);
    if (p.rotateDeathLimit < kRotateDeathLimitMin) p.rotateDeathLimit = kRotateDeathLimitMin;
    if (p.rotateDeathLimit > kRotateDeathLimitMax) p.rotateDeathLimit = kRotateDeathLimitMax;
    p.rotateDeathWindowMin = ReadIniInt(section, L"RotateDeathWindowMin", kRotateDeathWindowMinDefault);
    if (p.rotateDeathWindowMin < kRotateWindowMin) p.rotateDeathWindowMin = kRotateWindowMin;
    if (p.rotateDeathWindowMin > kRotateWindowMax) p.rotateDeathWindowMin = kRotateWindowMax;
    p.rotateNoFullBagMin = ReadIniInt(section, L"RotateNoFullBagMin", kRotateNoFullBagMinDefault);
    if (p.rotateNoFullBagMin < kRotateWindowMin) p.rotateNoFullBagMin = kRotateWindowMin;
    if (p.rotateNoFullBagMin > kRotateWindowMax) p.rotateNoFullBagMin = kRotateWindowMax;
    int rotationCount = ReadIniInt(section, L"RotationCount", 0);
    if (rotationCount < 0) rotationCount = 0;
    if (rotationCount > 64) rotationCount = 64;
    for (int i = 0; i < rotationCount; ++i) {
        std::wstring name = ReadIniText(section, L"RotationSpot_" + std::to_wstring(i));
        if (!name.empty() && std::none_of(p.rotationSpots.begin(), p.rotationSpots.end(), [&](const std::wstring& x){ return _wcsicmp(x.c_str(), name.c_str()) == 0; })) {
            p.rotationSpots.push_back(std::move(name));
        }
    }
    p.target.name = ReadIniText(section, L"TargetName");
    p.target.mapID = ReadIniInt(section, L"TargetMap", 0);
    p.target.x = ReadIniInt(section, L"TargetX", 0);
    p.target.y = ReadIniInt(section, L"TargetY", 0);
    p.target.valid = p.target.mapID > 0 && ReadIniInt(section, L"TargetValid", 0) != 0;
    if (p.selectedSpot.empty() && p.target.valid) p.selectedSpot = p.target.name;
    for (int i = 0; i < 4; ++i) {
        const std::wstring prefix = kClickKeys[static_cast<std::size_t>(i)];
        ClickPoint& c = p.points[static_cast<std::size_t>(i)];
        c.x = ReadIniInt(section, prefix + L"X", -1);
        c.y = ReadIniInt(section, prefix + L"Y", -1);
        c.baseW = ReadIniInt(section, prefix + L"W", 0);
        c.baseH = ReadIniInt(section, prefix + L"H", 0);
        c.valid = c.x >= 0 && c.y >= 0 && c.baseW > 0 && c.baseH > 0;
    }
    int macroCount = ReadIniInt(section, L"SellMacroCount", 0);
    if (macroCount < 0) macroCount = 0;
    if (macroCount > 64) macroCount = 64;
    for (int i = 0; i < macroCount; ++i) {
        SellMacroStep step{};
        const std::wstring prefix = L"Sell_" + std::to_wstring(i) + L"_";
        step.description = ReadIniText(section, prefix + L"Desc");
        step.point.x = ReadIniInt(section, prefix + L"X", -1);
        step.point.y = ReadIniInt(section, prefix + L"Y", -1);
        step.point.baseW = ReadIniInt(section, prefix + L"W", 0);
        step.point.baseH = ReadIniInt(section, prefix + L"H", 0);
        step.point.valid = step.point.x >= 0 && step.point.y >= 0 && step.point.baseW > 0 && step.point.baseH > 0;
        step.delayMs = ReadIniInt(section, prefix + L"Delay", 600);
        if (step.delayMs < 50) step.delayMs = 50;
        if (step.delayMs > 60000) step.delayMs = 60000;
        step.repeat = ReadIniInt(section, prefix + L"Repeat", 1);
        if (step.repeat < 1) step.repeat = 1;
        if (step.repeat > 999) step.repeat = 999;
        p.sellMacro.push_back(step);
    }
    return p;
}

void SaveProfile(const AccountProfile& p) {
    EnsureUnicodeIni();
    RemoveLegacyConfirmReviveKeys(p.section);
    WriteIniInt(p.section, L"Tolerance", p.tolerance);
    WriteIniInt(p.section, L"EnableRevive", p.enableRevive ? 1 : 0);
    WriteIniInt(p.section, L"EnableFight", p.enableFight ? 1 : 0);
    WriteIniInt(p.section, L"EnableSell", p.enableSell ? 1 : 0);
    WriteIniInt(p.section, L"SellNpcPreset", p.sellNpcPreset);
    WriteIniText(p.section, L"SelectedSpot", p.selectedSpot);
    WriteIniInt(p.section, L"RotateDeathLimit", p.rotateDeathLimit);
    WriteIniInt(p.section, L"RotateDeathWindowMin", p.rotateDeathWindowMin);
    WriteIniInt(p.section, L"RotateNoFullBagMin", p.rotateNoFullBagMin);
    WriteIniInt(p.section, L"RotationCount", static_cast<int>(p.rotationSpots.size()));
    for (std::size_t i = 0; i < p.rotationSpots.size(); ++i) {
        WriteIniText(p.section, L"RotationSpot_" + std::to_wstring(i), p.rotationSpots[i]);
    }
    WriteIniText(p.section, L"TargetName", p.target.name);
    WriteIniInt(p.section, L"TargetMap", p.target.mapID);
    WriteIniInt(p.section, L"TargetX", p.target.x);
    WriteIniInt(p.section, L"TargetY", p.target.y);
    WriteIniInt(p.section, L"TargetValid", p.target.valid ? 1 : 0);
    for (int i = 0; i < 4; ++i) {
        const std::wstring prefix = kClickKeys[static_cast<std::size_t>(i)];
        const ClickPoint& c = p.points[static_cast<std::size_t>(i)];
        WriteIniInt(p.section, prefix + L"X", c.valid ? c.x : -1);
        WriteIniInt(p.section, prefix + L"Y", c.valid ? c.y : -1);
        WriteIniInt(p.section, prefix + L"W", c.valid ? c.baseW : 0);
        WriteIniInt(p.section, prefix + L"H", c.valid ? c.baseH : 0);
    }
    WriteIniInt(p.section, L"SellMacroCount", static_cast<int>(p.sellMacro.size()));
    for (std::size_t i = 0; i < p.sellMacro.size(); ++i) {
        const SellMacroStep& step = p.sellMacro[i];
        const std::wstring prefix = L"Sell_" + std::to_wstring(i) + L"_";
        WriteIniText(p.section, prefix + L"Desc", step.description);
        WriteIniInt(p.section, prefix + L"X", step.point.valid ? step.point.x : -1);
        WriteIniInt(p.section, prefix + L"Y", step.point.valid ? step.point.y : -1);
        WriteIniInt(p.section, prefix + L"W", step.point.valid ? step.point.baseW : 0);
        WriteIniInt(p.section, prefix + L"H", step.point.valid ? step.point.baseH : 0);
        WriteIniInt(p.section, prefix + L"Delay", step.delayMs);
        WriteIniInt(p.section, prefix + L"Repeat", step.repeat);
    }
    FlushIni();
}


std::wstring SpotsPath() { return ExeDir() + L"\\ThanLongCleanRoute.spots.tsv"; }

std::wstring Utf8ToWide(const std::string& input) {
    if (input.empty()) return {};
    const int needed = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(),
                                           static_cast<int>(input.size()), nullptr, 0);
    if (needed <= 0) return {};
    std::wstring out(static_cast<std::size_t>(needed), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, input.data(), static_cast<int>(input.size()),
                        out.data(), needed);
    return out;
}

std::string WideToUtf8(const std::wstring& input) {
    if (input.empty()) return {};
    const int needed = WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()),
                                           nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return {};
    std::string out(static_cast<std::size_t>(needed), '\0');
    WideCharToMultiByte(CP_UTF8, 0, input.data(), static_cast<int>(input.size()), out.data(), needed,
                        nullptr, nullptr);
    return out;
}

std::wstring SanitizeSpotName(std::wstring name) {
    for (wchar_t& c : name) {
        if (c == L'\t' || c == L'\r' || c == L'\n') c = L' ';
    }
    while (!name.empty() && name.front() == L' ') name.erase(name.begin());
    while (!name.empty() && name.back() == L' ') name.pop_back();
    return name;
}

std::vector<std::wstring> SplitSpotLine(const std::wstring& line) {
    wchar_t separator = L'\t';
    if (line.find(L'\t') == std::wstring::npos) {
        if (line.find(L'|') != std::wstring::npos) separator = L'|';
        else if (line.find(L';') != std::wstring::npos) separator = L';';
        else return {};
    }
    std::vector<std::wstring> fields;
    std::size_t start = 0;
    while (start <= line.size()) {
        const std::size_t pos = line.find(separator, start);
        if (pos == std::wstring::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    return fields;
}

int FindSpotIndex(const std::vector<TargetProfile>& spots, const std::wstring& name);

std::vector<TargetProfile> LoadSharedSpots() {
    std::vector<TargetProfile> out;
    const std::wstring path = SpotsPath();
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return out;
    LARGE_INTEGER size{};
    if (!GetFileSizeEx(h, &size) || size.QuadPart <= 0 || size.QuadPart > 4 * 1024 * 1024) {
        CloseHandle(h);
        return out;
    }
    std::string bytes(static_cast<std::size_t>(size.QuadPart), '\0');
    DWORD read = 0;
    if (!ReadFile(h, bytes.data(), static_cast<DWORD>(bytes.size()), &read, nullptr)) {
        CloseHandle(h);
        return out;
    }
    CloseHandle(h);
    bytes.resize(read);

    std::wstring text;
    if (bytes.size() >= 2 && static_cast<unsigned char>(bytes[0]) == 0xFF &&
        static_cast<unsigned char>(bytes[1]) == 0xFE) {
        const std::size_t wcharCount = (bytes.size() - 2) / 2;
        text.resize(wcharCount);
        for (std::size_t i = 0; i < wcharCount; ++i) {
            const unsigned char lo = static_cast<unsigned char>(bytes[2 + i * 2]);
            const unsigned char hi = static_cast<unsigned char>(bytes[3 + i * 2]);
            text[i] = static_cast<wchar_t>(lo | (static_cast<unsigned int>(hi) << 8));
        }
    } else {
        if (bytes.size() >= 3 && static_cast<unsigned char>(bytes[0]) == 0xEF &&
            static_cast<unsigned char>(bytes[1]) == 0xBB && static_cast<unsigned char>(bytes[2]) == 0xBF) {
            bytes.erase(0, 3);
        }
        text = Utf8ToWide(bytes);
    }
    std::size_t lineStart = 0;
    while (lineStart <= text.size()) {
        std::size_t lineEnd = text.find(L'\n', lineStart);
        if (lineEnd == std::wstring::npos) lineEnd = text.size();
        std::wstring line = text.substr(lineStart, lineEnd - lineStart);
        if (!line.empty() && line.back() == L'\r') line.pop_back();
        if (!line.empty() && line[0] != L'#') {
            const auto f = SplitSpotLine(line);
            if (f.size() >= 4) {
                TargetProfile t{};
                const bool firstNumeric = !f[0].empty() && (f[0][0] >= L'0' && f[0][0] <= L'9');
                if (firstNumeric) {
                    t.mapID = _wtoi(f[0].c_str());
                    t.x = _wtoi(f[1].c_str());
                    t.y = _wtoi(f[2].c_str());
                    t.name = SanitizeSpotName(f[3]);
                } else {
                    t.name = SanitizeSpotName(f[0]);
                    t.mapID = _wtoi(f[1].c_str());
                    t.x = _wtoi(f[2].c_str());
                    t.y = _wtoi(f[3].c_str());
                }
                t.valid = !t.name.empty() && t.mapID > 0;
                if (t.valid && FindSpotIndex(out, t.name) < 0) out.push_back(std::move(t));
            }
        }
        if (lineEnd == text.size()) break;
        lineStart = lineEnd + 1;
    }
    return out;
}

void SaveSharedSpots(const std::vector<TargetProfile>& spots) {
    std::wstring wide;
    for (const auto& spot : spots) {
        if (!spot.valid || spot.mapID <= 0 || spot.name.empty()) continue;
        wide += SanitizeSpotName(spot.name) + L"\t" + std::to_wstring(spot.mapID) + L"\t" +
                std::to_wstring(spot.x) + L"\t" + std::to_wstring(spot.y) + L"\r\n";
    }
    const std::string bytes = WideToUtf8(wide);
    const std::wstring path = SpotsPath();
    HANDLE h = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD written = 0;
    if (!bytes.empty()) (void)WriteFile(h, bytes.data(), static_cast<DWORD>(bytes.size()), &written, nullptr);
    CloseHandle(h);
}

int FindSpotIndex(const std::vector<TargetProfile>& spots, const std::wstring& name) {
    if (name.empty()) return -1;
    for (std::size_t i = 0; i < spots.size(); ++i) {
        if (_wcsicmp(spots[i].name.c_str(), name.c_str()) == 0) return static_cast<int>(i);
    }
    return -1;
}

std::wstring GetText(HWND h) {
    const int n = GetWindowTextLengthW(h);
    std::wstring out(static_cast<std::size_t>(n) + 1, L'\0');
    if (n > 0) GetWindowTextW(h, out.data(), n + 1);
    out.resize(static_cast<std::size_t>(n));
    return out;
}

void SetText(HWND h, const std::wstring& s) { SetWindowTextW(h, s.c_str()); }

bool HasModule(DWORD pid, const wchar_t* name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) return false;
    MODULEENTRY32W e{};
    e.dwSize = sizeof(e);
    bool found = false;
    if (Module32FirstW(snap, &e)) {
        do {
            if (_wcsicmp(e.szModule, name) == 0) { found = true; break; }
        } while (Module32NextW(snap, &e));
    }
    CloseHandle(snap);
    return found;
}

BOOL CALLBACK EnumGameWindows(HWND hwnd, LPARAM param) {
    if (!IsWindowVisible(hwnd) || GetWindowTextLengthW(hwnd) <= 0) return TRUE;
    DWORD pid = 0;
    const DWORD tid = GetWindowThreadProcessId(hwnd, &pid);
    if (!pid || !tid || !HasModule(pid, kGameModule)) return TRUE;
    auto* out = reinterpret_cast<std::vector<GameClient>*>(param);
    for (const auto& g : *out) if (g.pid == pid) return TRUE;
    wchar_t title[512]{};
    GetWindowTextW(hwnd, title, _countof(title));
    out->push_back({pid, tid, hwnd, title});
    return TRUE;
}

std::vector<GameClient> FindClients() {
    std::vector<GameClient> out;
    EnumWindows(EnumGameWindows, reinterpret_cast<LPARAM>(&out));
    std::sort(out.begin(), out.end(), [](const GameClient& a, const GameClient& b){ return a.pid < b.pid; });
    return out;
}

class BridgeClient {
public:
    BridgeClient() = default;
    BridgeClient(const BridgeClient&) = delete;
    BridgeClient& operator=(const BridgeClient&) = delete;
    ~BridgeClient() { Close(); }

    bool Attach(const GameClient& game, std::wstring& error) {
        Close();
        game_ = game;
        wchar_t mappingName[96]{};
        MappingName(game.pid, mappingName, _countof(mappingName));
        mapping_ = CreateFileMappingW(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                      sizeof(SharedBlock), mappingName);
        if (!mapping_) { error = L"Không tạo được shared memory"; return false; }
        shared_ = reinterpret_cast<SharedBlock*>(MapViewOfFile(mapping_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
        if (!shared_) { error = L"Không map được shared memory"; Close(); return false; }
        ZeroMemory(shared_, sizeof(*shared_));
        shared_->magic = kMagic;
        shared_->protocolVersion = kProtocolVersion;
        shared_->targetPid = game.pid;
        shared_->targetWindowThreadId = game.threadId;
        shared_->targetHwnd = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(game.window));

        const std::wstring path = ExeDir() + L"\\ThanLongCleanRouteBridge.dll";
        if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
            error = L"Thiếu ThanLongCleanRouteBridge.dll cạnh EXE";
            Close();
            return false;
        }
        SetLastError(ERROR_SUCCESS);
        localDll_ = LoadLibraryW(path.c_str());
        const DWORD loadError = GetLastError();
        if (!localDll_) {
            error = L"Có Bridge DLL nhưng LoadLibrary thất bại Win32=" + std::to_wstring(loadError);
            Close();
            return false;
        }
        HOOKPROC proc = nullptr;
        if (!ResolveProc(localDll_, "TlcGetMessageHook", proc)) {
            error = L"Bridge DLL thiếu TlcGetMessageHook";
            Close();
            return false;
        }
        hook_ = SetWindowsHookExW(WH_GETMESSAGE, proc, localDll_, game.threadId);
        if (!hook_) {
            error = L"Không hook được game; hãy chạy tool cùng quyền với game";
            Close();
            return false;
        }
        if (!PostThreadMessageW(game.threadId, kWakeMessage, 0, 0)) {
            error = L"Không đánh thức được message thread game";
            Close();
            return false;
        }
        attached_ = true;
        return true;
    }

    void Close() {
        if (hook_) UnhookWindowsHookEx(hook_);
        if (localDll_) FreeLibrary(localDll_);
        if (shared_) UnmapViewOfFile(shared_);
        if (mapping_) CloseHandle(mapping_);
        hook_ = nullptr;
        localDll_ = nullptr;
        shared_ = nullptr;
        mapping_ = nullptr;
        attached_ = false;
        pendingSeq_ = 0;
        pendingWakeTick_ = 0;
    }

    bool AttachedTo(DWORD pid) const { return attached_ && game_.pid == pid; }
    bool Attached() const { return attached_; }

    bool Call(Command command, int a0, int a1, int a2, Response& out,
              std::wstring& error, DWORD timeoutMs = 1000) {
        if (!attached_ || !shared_) { error = L"Bridge chưa attach"; return false; }

        // Never overwrite a request that timed out on the controller side but may still
        // be executing on the game thread. A late completion is discarded safely; until
        // then only re-post the SAME wake message when the bridge is not busy.
        if (pendingSeq_ > 0) {
            if (shared_->completedSeq == pendingSeq_) {
                MemoryBarrier();
                pendingSeq_ = 0;
                pendingWakeTick_ = 0;
            } else {
                const DWORD now = GetTickCount();
                if (shared_->bridgeBusy == 0 &&
                    (pendingWakeTick_ == 0 || now - pendingWakeTick_ >= kBridgeNudgeMs)) {
                    (void)PostThreadMessageW(game_.threadId, kWakeMessage, 0, 0);
                    pendingWakeTick_ = now;
                }
                error = L"Bridge còn bận sau timeout; không gửi chồng request";
                return false;
            }
        }
        if (shared_->bridgeBusy != 0) {
            error = L"Bridge busy; không gửi chồng request";
            return false;
        }

        const LONG next = shared_->requestSeq + 1;
        shared_->request = {};
        shared_->request.command = static_cast<std::uint32_t>(command);
        shared_->request.arg0 = a0;
        shared_->request.arg1 = a1;
        shared_->request.arg2 = a2;
        MemoryBarrier();
        InterlockedExchange(&shared_->requestSeq, next);
        if (!PostThreadMessageW(game_.threadId, kWakeMessage, 0, 0)) {
            error = L"Không đánh thức được game thread";
            return false;
        }
        const DWORD begin = GetTickCount();
        while (GetTickCount() - begin < timeoutMs) {
            if (shared_->completedSeq == next) {
                MemoryBarrier();
                pendingSeq_ = 0;
                pendingWakeTick_ = 0;
                out = shared_->response;
                if (!out.ok) {
                    error = out.detail[0] ? out.detail : L"Bridge trả lỗi";
                    return false;
                }
                return true;
            }
            Sleep(2);
        }
        pendingSeq_ = next;
        pendingWakeTick_ = GetTickCount();
        error = L"Bridge timeout; fail-closed";
        return false;
    }

private:
    GameClient game_{};
    HANDLE mapping_ = nullptr;
    SharedBlock* shared_ = nullptr;
    HMODULE localDll_ = nullptr;
    HHOOK hook_ = nullptr;
    bool attached_ = false;
    LONG pendingSeq_ = 0;
    DWORD pendingWakeTick_ = 0;
};

struct Account {
    GameClient game{};
    BridgeClient bridge{};
    Snapshot snapshot{};
    bool snapshotValid = false;
    std::wstring displayName;
    AccountProfile profile{};
    RuntimeState runtime{};

    // Lifecycle latch intentionally lives OUTSIDE RuntimeState. ResetRuntime() may wipe
    // every automation phase at death/alive boundaries without forgetting that both
    // snapshots still belong to the same death session.
    bool deathSessionLatched = false;

    // Rotation metrics intentionally live OUTSIDE RuntimeState so death/alive cold
    // resets do not erase the rolling death window or productive-train timer.
    std::vector<DWORD> rotationDeathTicks{};
    DWORD rotationMetricTick = 0;
    std::uint64_t rotationActiveTrainMs = 0;
    bool rotationBagWasFull = false;
};

std::wstring PointDescription(const ClickPoint& p) {
    if (!p.valid) return L"CHƯA LẤY";
    return std::to_wstring(p.x) + L"," + std::to_wstring(p.y) + L" @ " +
           std::to_wstring(p.baseW) + L"x" + std::to_wstring(p.baseH);
}

bool ScaleClickPoint(const GameClient& game, const ClickPoint& saved, POINT& point, std::wstring& error) {
    if (!saved.valid) { error = L"Chưa lấy tọa độ click"; return false; }
    if (!game.window || !IsWindow(game.window)) { error = L"Cửa sổ game không còn tồn tại"; return false; }
    RECT rc{};
    if (!GetClientRect(game.window, &rc)) { error = L"Không đọc được client rect"; return false; }
    const int width = rc.right - rc.left;
    const int height = rc.bottom - rc.top;
    if (width <= 0 || height <= 0 || saved.baseW <= 0 || saved.baseH <= 0) {
        error = L"Kích thước cửa sổ không hợp lệ";
        return false;
    }
    point.x = MulDiv(saved.x, width, saved.baseW);
    point.y = MulDiv(saved.y, height, saved.baseH);
    if (point.x < 0 || point.y < 0 || point.x >= width || point.y >= height) {
        error = L"Tọa độ sau scale nằm ngoài cửa sổ";
        return false;
    }
    return true;
}

bool Elapsed(DWORD now, DWORD since, DWORD delay) {
    return since != 0 && now - since >= delay;
}

void ResetRuntime(RuntimeState& r) {
    const bool running = r.running;
    r = RuntimeState{};
    r.running = running;
    r.status = running ? L"Đang giám sát" : L"Đã dừng";
}

class App {
public:
    bool Create(HINSTANCE instance) {
        MigrateLegacyConfigIfNeeded();
        EnsureUnicodeIni();
        LoadShutdownSettings();
        spots_ = LoadSharedSpots();
        INITCOMMONCONTROLSEX ic{sizeof(ic), ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES};
        InitCommonControlsEx(&ic);
        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = instance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"ThanLongCleanRouteMultiWindow";
        if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
        hwnd_ = CreateWindowExW(0, wc.lpszClassName, kTitle,
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                                CW_USEDEFAULT, CW_USEDEFAULT, 1060, 933,
                                nullptr, nullptr, instance, this);
        return hwnd_ != nullptr;
    }

    void Show(int cmd) {
        ShowWindow(hwnd_, cmd);
        UpdateWindow(hwnd_);
    }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        App* self = reinterpret_cast<App*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
            self = reinterpret_cast<App*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
            self->hwnd_ = hwnd;
        }
        return self ? self->Handle(msg, wp, lp) : DefWindowProcW(hwnd, msg, wp, lp);
    }

    HWND Make(const wchar_t* cls, const wchar_t* text, DWORD style,
              int x, int y, int w, int h, int id) {
        return CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style,
                               x, y, w, h, hwnd_,
                               reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), nullptr, nullptr);
    }

    void AddListColumn(int index, int width, const wchar_t* text) {
        LVCOLUMNW c{};
        c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        c.pszText = const_cast<wchar_t*>(text);
        c.cx = width;
        c.iSubItem = index;
        ListView_InsertColumn(clientList_, index, &c);
    }

    void AddMacroColumn(int index, int width, const wchar_t* text) {
        LVCOLUMNW c{};
        c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        c.pszText = const_cast<wchar_t*>(text);
        c.cx = width;
        c.iSubItem = index;
        ListView_InsertColumn(sellMacroList_, index, &c);
    }

    void AddRotationColumn(int index, int width, const wchar_t* text) {
        LVCOLUMNW c{};
        c.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
        c.pszText = const_cast<wchar_t*>(text);
        c.cx = width;
        c.iSubItem = index;
        ListView_InsertColumn(rotationList_, index, &c);
    }

    void BuildUi() {
        HFONT font = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
        auto addFont = [font](HWND h){ if (h) SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE); };

        addFont(Make(L"STATIC", L"DANH SÁCH ACC — F4 tạm dừng/tiếp tục toàn bộ acc đang chạy", 0,
                     18, 10, 1005, 20, 0));
        clientList_ = Make(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER,
                           18, 32, 1005, 165, IDC_CLIENT_LIST);
        addFont(clientList_);
        ListView_SetExtendedListViewStyle(clientList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
        AddListColumn(0, 185, L"Nhân vật / RoleID");
        AddListColumn(1, 80, L"PID");
        AddListColumn(2, 205, L"Trạng thái");
        AddListColumn(3, 180, L"Map / X,Y / Túi");
        AddListColumn(4, 315, L"Bãi chung acc đang chọn");

        addFont(Make(L"BUTTON", L"QUÉT CLIENT", BS_PUSHBUTTON, 18, 205, 150, 30, IDC_SCAN));
        addFont(Make(L"BUTTON", L"BẮT ĐẦU ACC ĐÃ TICK", BS_DEFPUSHBUTTON, 180, 205, 235, 30, IDC_START_CHECKED));
        addFont(Make(L"BUTTON", L"DỪNG ACC ĐÃ TICK", BS_PUSHBUTTON, 427, 205, 220, 30, IDC_STOP_CHECKED));
        selected_ = Make(L"STATIC", L"ACC ĐANG CHỈNH: chưa chọn", SS_LEFT | SS_CENTERIMAGE | WS_BORDER,
                         660, 205, 363, 30, IDC_SELECTED); addFont(selected_);

        live_ = Make(L"STATIC", L"STATE: chưa có", SS_LEFT | SS_CENTERIMAGE | WS_BORDER,
                     18, 243, 610, 38, IDC_LIVE); addFont(live_);
        shutdownEnable_ = Make(L"BUTTON", L"Hẹn tắt tool", BS_AUTOCHECKBOX, 638, 250, 105, 24, IDC_SHUTDOWN_ENABLE); addFont(shutdownEnable_);
        shutdownHourEdit_ = Make(L"EDIT", L"00", WS_BORDER | ES_NUMBER | ES_CENTER, 746, 247, 38, 27, IDC_SHUTDOWN_HOUR); addFont(shutdownHourEdit_);
        addFont(Make(L"STATIC", L":", SS_CENTER | SS_CENTERIMAGE, 786, 247, 14, 27, 0));
        shutdownMinuteEdit_ = Make(L"EDIT", L"00", WS_BORDER | ES_NUMBER | ES_CENTER, 802, 247, 38, 27, IDC_SHUTDOWN_MINUTE); addFont(shutdownMinuteEdit_);
        addFont(Make(L"BUTTON", L"ÁP DỤNG", BS_PUSHBUTTON, 848, 247, 80, 27, IDC_SHUTDOWN_APPLY));
        shutdownStatus_ = Make(L"STATIC", L"OFF", SS_CENTER | SS_CENTERIMAGE | WS_BORDER, 936, 247, 87, 27, IDC_SHUTDOWN_STATUS); addFont(shutdownStatus_);
        SendMessageW(shutdownEnable_, BM_SETCHECK, shutdownEnabled_ ? BST_CHECKED : BST_UNCHECKED, 0);
        wchar_t shutdownHourText[8]{}, shutdownMinuteText[8]{};
        wsprintfW(shutdownHourText, L"%02d", shutdownHour_); wsprintfW(shutdownMinuteText, L"%02d", shutdownMinute_);
        SetWindowTextW(shutdownHourEdit_, shutdownHourText); SetWindowTextW(shutdownMinuteEdit_, shutdownMinuteText);
        UpdateShutdownStatusText();

        addFont(Make(L"STATIC", L"SETTING RIÊNG ACC", 0, 18, 290, 150, 20, 0));
        addFont(Make(L"STATIC", L"Bãi:", 0, 18, 316, 45, 22, 0));
        spotCombo_ = Make(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL, 63, 312, 220, 240, IDC_SPOT_COMBO); addFont(spotCombo_);
        addFont(Make(L"STATIC", L"Tên lưu:", 0, 292, 316, 60, 22, 0));
        targetName_ = Make(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 352, 312, 135, 27, IDC_TARGET_NAME); addFont(targetName_);
        addFont(Make(L"BUTTON", L"LƯU/CẬP NHẬT", BS_PUSHBUTTON, 497, 312, 150, 28, IDC_SAVE_TARGET));
        addFont(Make(L"BUTTON", L"XÓA BÃI", BS_PUSHBUTTON, 657, 312, 90, 28, IDC_DELETE_SPOT));
        targetText_ = Make(L"STATIC", L"CHƯA CHỌN", SS_LEFT | SS_CENTERIMAGE | WS_BORDER, 757, 312, 266, 28, IDC_TARGET_TEXT); addFont(targetText_);

        addFont(Make(L"STATIC", L"Sai số:", 0, 18, 350, 55, 22, 0));
        tolerance_ = Make(L"EDIT", L"120", WS_BORDER | ES_NUMBER | ES_CENTER, 73, 346, 70, 27, IDC_TOLERANCE); addFont(tolerance_);
        enableRevive_ = Make(L"BUTTON", L"Tự Đầu thai (nội bộ v0.8.7)", BS_AUTOCHECKBOX, 160, 347, 205, 24, IDC_ENABLE_REVIVE); addFont(enableRevive_);
        enableFight_ = Make(L"BUTTON", L"AUTO → Đánh quái", BS_AUTOCHECKBOX, 380, 347, 145, 24, IDC_ENABLE_FIGHT); addFont(enableFight_);
        addFont(Make(L"STATIC", L"Xác nhận ra map: UIButton nội bộ v0.8.7 • không tọa độ / không timer", 0, 540, 350, 483, 22, 0));

        addFont(Make(L"STATIC", L"XOAY BÃI TRAIN — tick các bãi muốn chạy vòng; thứ tự theo danh sách bãi chung", 0, 18, 382, 1005, 20, 0));
        rotationList_ = Make(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SHOWSELALWAYS | WS_BORDER,
                             18, 404, 1005, 90, IDC_ROTATION_LIST);
        addFont(rotationList_);
        ListView_SetExtendedListViewStyle(rotationList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES);
        AddRotationColumn(0, 515, L"Bãi train");
        AddRotationColumn(1, 95, L"Map");
        AddRotationColumn(2, 160, L"X,Y");
        AddRotationColumn(3, 220, L"Ghi chú");

        addFont(Make(L"STATIC", L"Đổi bãi nếu chết quá", 0, 18, 500, 120, 22, 0));
        rotateDeathLimit_ = Make(L"EDIT", L"10", WS_BORDER | ES_NUMBER | ES_CENTER, 140, 497, 45, 27, IDC_ROTATE_DEATH_LIMIT); addFont(rotateDeathLimit_);
        addFont(Make(L"STATIC", L"lần /", 0, 190, 500, 38, 22, 0));
        rotateDeathWindow_ = Make(L"EDIT", L"10", WS_BORDER | ES_NUMBER | ES_CENTER, 230, 497, 45, 27, IDC_ROTATE_DEATH_WINDOW); addFont(rotateDeathWindow_);
        addFont(Make(L"STATIC", L"phút", 0, 280, 500, 38, 22, 0));
        addFont(Make(L"STATIC", L"• Đổi bãi nếu chưa FULL túi trong", 0, 335, 500, 190, 22, 0));
        rotateNoFullBag_ = Make(L"EDIT", L"15", WS_BORDER | ES_NUMBER | ES_CENTER, 530, 497, 45, 27, IDC_ROTATE_NO_BAG); addFont(rotateNoFullBag_);
        addFont(Make(L"STATIC", L"phút train thực • 1 bãi = không đổi • nhiều bãi = vòng lại bãi 1", 0, 580, 500, 443, 22, 0));

        addFont(Make(L"STATIC", L"4 CLICK RIÊNG ACC — chỉ giữ AUTO / ĐÁNH QUÁI / DỪNG 1 / DỪNG 2", 0, 18, 530, 1005, 20, 0));
        const int rowY[4] = {552, 578, 604, 630};
        const int pointIds[4] = {IDC_POINT_AUTO, IDC_POINT_ATTACK, IDC_POINT_STOP_AUTO_1, IDC_POINT_STOP_AUTO_2};
        const int captureIds[4] = {IDC_CAPTURE_AUTO, IDC_CAPTURE_ATTACK, IDC_CAPTURE_STOP_AUTO_1, IDC_CAPTURE_STOP_AUTO_2};
        const int testIds[4] = {IDC_TEST_AUTO, IDC_TEST_ATTACK, IDC_TEST_STOP_AUTO_1, IDC_TEST_STOP_AUTO_2};
        for (int i = 0; i < 4; ++i) {
            addFont(Make(L"STATIC", kClickLabels[static_cast<std::size_t>(i)], SS_LEFT | SS_CENTERIMAGE, 18, rowY[i], 150, 24, 0));
            pointLabels_[static_cast<std::size_t>(i)] = Make(L"STATIC", L"CHƯA LẤY", SS_LEFT | SS_CENTERIMAGE | WS_BORDER, 172, rowY[i], 430, 24, pointIds[i]);
            addFont(pointLabels_[static_cast<std::size_t>(i)]);
            addFont(Make(L"BUTTON", L"LẤY (F8)", BS_PUSHBUTTON, 612, rowY[i], 115, 24, captureIds[i]));
            addFont(Make(L"BUTTON", L"TEST", BS_PUSHBUTTON, 737, rowY[i], 90, 24, testIds[i]));
        }

        enableSell_ = Make(L"BUTTON", L"AUTO BÁN ĐỒ KHI TÚI FULL", BS_AUTOCHECKBOX, 18, 660, 220, 25, IDC_ENABLE_SELL); addFont(enableSell_);
        addFont(Make(L"STATIC", L"NPC bán:", 0, 250, 663, 65, 22, 0));
        sellNpcCombo_ = Make(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_VSCROLL, 315, 658, 250, 180, IDC_SELL_NPC); addFont(sellNpcCombo_);
        for (const auto& npc : kSellNpcs) SendMessageW(sellNpcCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(npc.name));
        addFont(Make(L"STATIC", L"X:", 0, 574, 663, 18, 22, 0));
        sellNpcX_ = Make(L"EDIT", L"", WS_BORDER | ES_NUMBER | ES_CENTER, 592, 658, 58, 27, IDC_SELL_NPC_X); addFont(sellNpcX_);
        addFont(Make(L"STATIC", L"Y:", 0, 658, 663, 18, 22, 0));
        sellNpcY_ = Make(L"EDIT", L"", WS_BORDER | ES_NUMBER | ES_CENTER, 676, 658, 58, 27, IDC_SELL_NPC_Y); addFont(sellNpcY_);
        addFont(Make(L"BUTTON", L"LẤY VỊ TRÍ", BS_PUSHBUTTON, 742, 658, 112, 27, IDC_SELL_NPC_CAPTURE));
        sellNpcPosText_ = Make(L"STATIC", L"CHƯA LẤY", SS_LEFT | SS_CENTERIMAGE | WS_BORDER, 862, 658, 161, 27, IDC_SELL_NPC_POS); addFont(sellNpcPosText_);

        sellMacroList_ = Make(WC_LISTVIEWW, L"", LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS | WS_BORDER, 18, 690, 1005, 72, IDC_SELL_MACRO_LIST);
        addFont(sellMacroList_);
        ListView_SetExtendedListViewStyle(sellMacroList_, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        AddMacroColumn(0, 36, L"#");
        AddMacroColumn(1, 400, L"Mô tả bước bán");
        AddMacroColumn(2, 230, L"Tọa độ");
        AddMacroColumn(3, 110, L"Delay ms");
        AddMacroColumn(4, 90, L"Lặp");

        addFont(Make(L"BUTTON", L"+ THÊM", BS_PUSHBUTTON, 18, 766, 82, 27, IDC_SELL_ADD));
        addFont(Make(L"BUTTON", L"- XÓA", BS_PUSHBUTTON, 108, 766, 82, 27, IDC_SELL_DELETE));
        sellDesc_ = Make(L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 202, 766, 260, 27, IDC_SELL_DESC); addFont(sellDesc_);
        sellDelay_ = Make(L"EDIT", L"600", WS_BORDER | ES_NUMBER | ES_CENTER, 470, 766, 75, 27, IDC_SELL_DELAY); addFont(sellDelay_);
        sellRepeat_ = Make(L"EDIT", L"1", WS_BORDER | ES_NUMBER | ES_CENTER, 553, 766, 55, 27, IDC_SELL_REPEAT); addFont(sellRepeat_);
        addFont(Make(L"BUTTON", L"LƯU DÒNG", BS_PUSHBUTTON, 616, 766, 100, 27, IDC_SELL_SAVE));
        addFont(Make(L"BUTTON", L"LẤY DÒNG (F8)", BS_PUSHBUTTON, 724, 766, 130, 27, IDC_SELL_CAPTURE));
        addFont(Make(L"BUTTON", L"TEST DÒNG", BS_PUSHBUTTON, 862, 766, 112, 27, IDC_SELL_TEST));

        addFont(Make(L"STATIC", L"LOG", 0, 18, 798, 50, 20, 0));
        log_ = Make(L"EDIT", L"", WS_BORDER | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | WS_VSCROLL, 18, 818, 1005, 53, IDC_LOG); addFont(log_);

        if (!RegisterHotKey(hwnd_, kCaptureHotkeyId, MOD_NOREPEAT, VK_F8)) {
            Log(L"CẢNH BÁO: không đăng ký được F8 global.");
        }
        if (!RegisterHotKey(hwnd_, kPauseHotkeyId, MOD_NOREPEAT, VK_F4)) {
            Log(L"CẢNH BÁO: không đăng ký được F4 global.");
        }
        SetTimer(hwnd_, kTimer, 250, nullptr);
        ScanClients();
    }

    void Log(const std::wstring& text) {
        if (!log_) return;
        SYSTEMTIME st{};
        GetLocalTime(&st);
        wchar_t prefix[32]{};
        wsprintfW(prefix, L"[%02u:%02u:%02u] ", st.wHour, st.wMinute, st.wSecond);
        std::wstring line = prefix + text + L"\r\n";
        const int len = GetWindowTextLengthW(log_);
        SendMessageW(log_, EM_SETSEL, len, len);
        SendMessageW(log_, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(line.c_str()));
        SendMessageW(log_, EM_SCROLLCARET, 0, 0);
    }

    std::wstring AccountTag(const Account& a) const {
        if (!a.displayName.empty()) return a.displayName + L"/PID " + std::to_wstring(a.game.pid);
        return L"PID " + std::to_wstring(a.game.pid);
    }

    void LogAccount(const Account& a, const std::wstring& text) {
        Log(L"[" + AccountTag(a) + L"] " + text);
    }

    int SelectedIndex() const {
        if (!clientList_) return -1;
        return ListView_GetNextItem(clientList_, -1, LVNI_SELECTED);
    }

    Account* SelectedAccount() {
        const int i = SelectedIndex();
        if (i < 0 || i >= static_cast<int>(accounts_.size())) return nullptr;
        return accounts_[static_cast<std::size_t>(i)].get();
    }

    Account* AccountByPid(DWORD pid) {
        for (auto& a : accounts_) if (a->game.pid == pid) return a.get();
        return nullptr;
    }

    static std::wstring ProfileSection(const Snapshot& s, DWORD pid) {
        if ((s.validMask & ValidIdentity) && s.roleID > 0) return L"Role_" + std::to_wstring(s.roleID);
        return L"PID_" + std::to_wstring(pid);
    }

    static std::wstring DisplayName(const Snapshot& s, DWORD pid) {
        std::wstring name = s.characterName[0] ? s.characterName : L"?";
        if ((s.validMask & ValidIdentity) && s.roleID > 0) {
            return name + L" • " + std::to_wstring(s.roleID);
        }
        return name + L" • PID " + std::to_wstring(pid);
    }

    bool EnsureAttach(Account& a, std::wstring& error) {
        if (a.bridge.AttachedTo(a.game.pid)) return true;
        if (!IsWindow(a.game.window)) { error = L"Cửa sổ game đã mất"; return false; }
        return a.bridge.Attach(a.game, error);
    }

    bool ReadSnapshot(Account& a, std::wstring& error, DWORD timeout = 850) {
        if (!EnsureAttach(a, error)) return false;
        Response r{};
        if (!a.bridge.Call(Command::ReadState, 0, 0, 0, r, error, timeout)) return false;
        a.snapshot = r.snapshot;
        a.snapshotValid = true;
        return true;
    }

    void ScanClients() {
        captureSlot_ = ClickSlot::None;
        captureMacroIndex_ = -1;
        capturePid_ = 0;
        for (auto& a : accounts_) a->bridge.Close();
        accounts_.clear();
        ListView_DeleteAllItems(clientList_);

        const auto found = FindClients();
        for (const auto& game : found) {
            auto a = std::make_unique<Account>();
            a->game = game;
            std::wstring error;
            if (a->bridge.Attach(game, error)) {
                Response r{};
                if (a->bridge.Call(Command::ReadState, 0, 0, 0, r, error, 1200)) {
                    a->snapshot = r.snapshot;
                    a->snapshotValid = true;
                }
            }
            if (!a->snapshotValid) {
                a->snapshot = {};
                a->displayName = L"? • PID " + std::to_wstring(game.pid);
                Log(L"PID " + std::to_wstring(game.pid) + L": chưa đọc được identity: " + error);
            } else {
                a->displayName = DisplayName(a->snapshot, game.pid);
            }
            a->profile = LoadProfile(ProfileSection(a->snapshot, game.pid));
            MigrateLegacySpot(a->profile);
            a->runtime.status = L"Đã dừng";
            accounts_.push_back(std::move(a));
        }

        for (std::size_t i = 0; i < accounts_.size(); ++i) InsertAccountRow(static_cast<int>(i), *accounts_[i]);
        RefreshSpotCombo();
        if (!accounts_.empty()) {
            ListView_SetItemState(clientList_, 0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ListView_EnsureVisible(clientList_, 0, FALSE);
            LoadSelectedProfileToUi();
        } else {
            ClearEditor();
        }
        Log(L"Quét thấy " + std::to_wstring(accounts_.size()) + L" client GameAssembly.dll.");
    }

    void InsertAccountRow(int row, const Account& a) {
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_PARAM;
        item.iItem = row;
        item.iSubItem = 0;
        item.pszText = const_cast<wchar_t*>(a.displayName.c_str());
        item.lParam = static_cast<LPARAM>(a.game.pid);
        ListView_InsertItem(clientList_, &item);
        SetRowText(row, 1, std::to_wstring(a.game.pid));
        UpdateAccountRow(row, a);
    }

    void SetRowText(int row, int sub, const std::wstring& text) {
        ListView_SetItemText(clientList_, row, sub, const_cast<wchar_t*>(text.c_str()));
    }

    void UpdateAccountRow(int row, const Account& a) {
        SetRowText(row, 0, a.displayName);
        SetRowText(row, 1, std::to_wstring(a.game.pid));
        SetRowText(row, 2, (a.runtime.running ? L"RUN • " : L"STOP • ") + a.runtime.status);
        if (a.snapshotValid && (a.snapshot.validMask & (ValidMap | ValidPosition)) == (ValidMap | ValidPosition)) {
            std::wstring mapText = L"M" + std::to_wstring(a.snapshot.mapID) + L" • " +
                                   std::to_wstring(a.snapshot.x) + L"," + std::to_wstring(a.snapshot.y);
            if (a.snapshot.validMask & ValidBagSpace) mapText += L" • Trống " + std::to_wstring(a.snapshot.freeBagSpace);
            SetRowText(row, 3, mapText);
        } else {
            SetRowText(row, 3, L"?");
        }
        if (a.profile.target.valid) {
            SetRowText(row, 4, a.profile.target.name + L" • M" + std::to_wstring(a.profile.target.mapID) +
                             L" • " + std::to_wstring(a.profile.target.x) + L"," + std::to_wstring(a.profile.target.y) +
                             L" • vòng " + std::to_wstring(a.profile.rotationSpots.size()) + L" bãi");
        } else {
            SetRowText(row, 4, L"CHƯA CHỌN BÃI");
        }
    }


    void ResolveProfileTarget(AccountProfile& p) {
        const int index = FindSpotIndex(spots_, p.selectedSpot);
        if (index >= 0) {
            p.target = spots_[static_cast<std::size_t>(index)];
            p.target.valid = true;
        } else {
            p.target = {};
        }
    }

    bool RotationContains(const AccountProfile& p, const std::wstring& name) const {
        return std::any_of(p.rotationSpots.begin(), p.rotationSpots.end(), [&](const std::wstring& x){
            return _wcsicmp(x.c_str(), name.c_str()) == 0;
        });
    }

    void NormalizeRotationProfile(AccountProfile& p) {
        std::vector<std::wstring> clean;
        for (const auto& name : p.rotationSpots) {
            if (FindSpotIndex(spots_, name) < 0) continue;
            if (std::none_of(clean.begin(), clean.end(), [&](const std::wstring& x){ return _wcsicmp(x.c_str(), name.c_str()) == 0; })) {
                clean.push_back(name);
            }
        }
        p.rotationSpots = std::move(clean);
        if (p.rotationSpots.empty() && !p.selectedSpot.empty() && FindSpotIndex(spots_, p.selectedSpot) >= 0) {
            p.rotationSpots.push_back(p.selectedSpot);
        }
        if (!p.rotationSpots.empty() && (p.selectedSpot.empty() || !RotationContains(p, p.selectedSpot))) {
            p.selectedSpot = p.rotationSpots.front();
        }
        ResolveProfileTarget(p);
    }

    void RefreshRotationList() {
        if (!rotationList_) return;
        rotationUiLoading_ = true;
        ListView_DeleteAllItems(rotationList_);
        Account* a = SelectedAccount();
        for (std::size_t i = 0; i < spots_.size(); ++i) {
            const TargetProfile& spot = spots_[i];
            LVITEMW item{};
            item.mask = LVIF_TEXT;
            item.iItem = static_cast<int>(i);
            item.pszText = const_cast<wchar_t*>(spot.name.c_str());
            ListView_InsertItem(rotationList_, &item);
            const std::wstring map = L"M" + std::to_wstring(spot.mapID);
            const std::wstring xy = std::to_wstring(spot.x) + L"," + std::to_wstring(spot.y);
            ListView_SetItemText(rotationList_, static_cast<int>(i), 1, const_cast<wchar_t*>(map.c_str()));
            ListView_SetItemText(rotationList_, static_cast<int>(i), 2, const_cast<wchar_t*>(xy.c_str()));
            const std::wstring note = (a && _wcsicmp(a->profile.selectedSpot.c_str(), spot.name.c_str()) == 0) ? L"BÃI HIỆN TẠI" : L"";
            ListView_SetItemText(rotationList_, static_cast<int>(i), 3, const_cast<wchar_t*>(note.c_str()));
            if (a && RotationContains(a->profile, spot.name)) ListView_SetCheckState(rotationList_, static_cast<int>(i), TRUE);
        }
        rotationUiLoading_ = false;
    }

    void PersistRotationListFromUi(Account& a) {
        if (!rotationList_) return;
        std::vector<std::wstring> selected;
        const int count = std::min(ListView_GetItemCount(rotationList_), static_cast<int>(spots_.size()));
        for (int i = 0; i < count; ++i) {
            if (ListView_GetCheckState(rotationList_, i)) selected.push_back(spots_[static_cast<std::size_t>(i)].name);
        }
        if (selected.empty() && !a.profile.selectedSpot.empty() && FindSpotIndex(spots_, a.profile.selectedSpot) >= 0) {
            selected.push_back(a.profile.selectedSpot);
        }
        a.profile.rotationSpots = std::move(selected);
        NormalizeRotationProfile(a.profile);
    }

    void MigrateLegacySpot(AccountProfile& p) {
        if (p.selectedSpot.empty() && p.target.valid) p.selectedSpot = p.target.name;
        if (p.target.valid && !p.selectedSpot.empty()) {
            int index = FindSpotIndex(spots_, p.selectedSpot);
            if (index >= 0) {
                const TargetProfile& existing = spots_[static_cast<std::size_t>(index)];
                if (existing.mapID != p.target.mapID || existing.x != p.target.x || existing.y != p.target.y) {
                    p.selectedSpot += L" [M" + std::to_wstring(p.target.mapID) + L" " +
                                      std::to_wstring(p.target.x) + L"," + std::to_wstring(p.target.y) + L"]";
                    index = FindSpotIndex(spots_, p.selectedSpot);
                }
            }
            if (index < 0) {
                TargetProfile migrated = p.target;
                migrated.name = p.selectedSpot;
                migrated.valid = true;
                spots_.push_back(std::move(migrated));
                SaveSharedSpots(spots_);
            }
        }
        NormalizeRotationProfile(p);
    }

    void RefreshSpotCombo() {
        if (!spotCombo_) return;
        SendMessageW(spotCombo_, CB_RESETCONTENT, 0, 0);
        for (const auto& spot : spots_) {
            SendMessageW(spotCombo_, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(spot.name.c_str()));
        }
        Account* a = SelectedAccount();
        int select = -1;
        if (a) select = FindSpotIndex(spots_, a->profile.selectedSpot);
        SendMessageW(spotCombo_, CB_SETCURSEL, select, 0);
    }

    void SelectSharedSpotForAccount() {
        Account* a = SelectedAccount();
        if (!a) return;
        const LRESULT sel = SendMessageW(spotCombo_, CB_GETCURSEL, 0, 0);
        if (sel == CB_ERR || sel < 0 || static_cast<std::size_t>(sel) >= spots_.size()) return;
        const TargetProfile& spot = spots_[static_cast<std::size_t>(sel)];
        const std::wstring oldSpot = a->profile.selectedSpot;
        a->profile.selectedSpot = spot.name;
        a->profile.target = spot;
        if (!RotationContains(a->profile, spot.name)) a->profile.rotationSpots.push_back(spot.name);
        NormalizeRotationProfile(a->profile);
        SetText(targetName_, spot.name);
        SaveProfile(a->profile);
        if (_wcsicmp(oldSpot.c_str(), spot.name.c_str()) != 0) {
            ResetRotationWindow(*a, GetTickCount());
            if (a->runtime.running) BeginTrainRecovery(*a, GetTickCount());
        }
        LoadSelectedProfileToUi();
        const int row = SelectedIndex();
        if (row >= 0) UpdateAccountRow(row, *a);
        LogAccount(*a, L"Đã chọn bãi chung: " + spot.name + L" • M" + std::to_wstring(spot.mapID) + L" • " +
                       std::to_wstring(spot.x) + L"," + std::to_wstring(spot.y));
    }

    void DeleteSelectedSharedSpot() {
        Account* a = SelectedAccount();
        if (!a) { Log(L"Chưa chọn acc"); return; }
        const LRESULT sel = SendMessageW(spotCombo_, CB_GETCURSEL, 0, 0);
        if (sel == CB_ERR || sel < 0 || static_cast<std::size_t>(sel) >= spots_.size()) {
            Log(L"Chưa chọn bãi chung để xóa");
            return;
        }
        const std::wstring name = spots_[static_cast<std::size_t>(sel)].name;
        spots_.erase(spots_.begin() + sel);
        SaveSharedSpots(spots_);
        for (auto& item : accounts_) {
            item->profile.rotationSpots.erase(std::remove_if(item->profile.rotationSpots.begin(), item->profile.rotationSpots.end(), [&](const std::wstring& x){
                return _wcsicmp(x.c_str(), name.c_str()) == 0;
            }), item->profile.rotationSpots.end());
            if (_wcsicmp(item->profile.selectedSpot.c_str(), name.c_str()) == 0) {
                item->profile.selectedSpot = item->profile.rotationSpots.empty() ? L"" : item->profile.rotationSpots.front();
            }
            NormalizeRotationProfile(item->profile);
            SaveProfile(item->profile);
        }
        RefreshSpotCombo();
        LoadSelectedProfileToUi();
        for (std::size_t i = 0; i < accounts_.size(); ++i) UpdateAccountRow(static_cast<int>(i), *accounts_[i]);
        Log(L"Đã xóa bãi chung: " + name);
    }

    int SelectedMacroIndex() const {
        if (!sellMacroList_) return -1;
        return ListView_GetNextItem(sellMacroList_, -1, LVNI_SELECTED);
    }

    void RefreshSellMacroList() {
        if (!sellMacroList_) return;
        ListView_DeleteAllItems(sellMacroList_);
        Account* a = SelectedAccount();
        if (!a) return;
        for (std::size_t i = 0; i < a->profile.sellMacro.size(); ++i) {
            const SellMacroStep& step = a->profile.sellMacro[i];
            std::wstring no = std::to_wstring(i + 1);
            LVITEMW item{};
            item.mask = LVIF_TEXT;
            item.iItem = static_cast<int>(i);
            item.pszText = const_cast<wchar_t*>(no.c_str());
            ListView_InsertItem(sellMacroList_, &item);
            ListView_SetItemText(sellMacroList_, static_cast<int>(i), 1,
                const_cast<wchar_t*>(step.description.empty() ? L"(chưa mô tả)" : step.description.c_str()));
            std::wstring point = PointDescription(step.point);
            ListView_SetItemText(sellMacroList_, static_cast<int>(i), 2, const_cast<wchar_t*>(point.c_str()));
            std::wstring delay = std::to_wstring(step.delayMs);
            std::wstring repeat = std::to_wstring(step.repeat);
            ListView_SetItemText(sellMacroList_, static_cast<int>(i), 3, const_cast<wchar_t*>(delay.c_str()));
            ListView_SetItemText(sellMacroList_, static_cast<int>(i), 4, const_cast<wchar_t*>(repeat.c_str()));
        }
    }

    void ClearSellMacroEditor() {
        SetText(sellDesc_, L"");
        SetText(sellDelay_, L"600");
        SetText(sellRepeat_, L"1");
    }

    void LoadSelectedMacroEditor() {
        Account* a = SelectedAccount();
        const int index = SelectedMacroIndex();
        if (!a || index < 0 || index >= static_cast<int>(a->profile.sellMacro.size())) {
            ClearSellMacroEditor();
            return;
        }
        const SellMacroStep& step = a->profile.sellMacro[static_cast<std::size_t>(index)];
        SetText(sellDesc_, step.description);
        SetText(sellDelay_, std::to_wstring(step.delayMs));
        SetText(sellRepeat_, std::to_wstring(step.repeat));
    }

    void AddSellMacroRow() {
        Account* a = SelectedAccount();
        if (!a) { Log(L"Chưa chọn acc để thêm bước bán"); return; }
        if (a->profile.sellMacro.size() >= 64) { LogAccount(*a, L"Macro bán tối đa 64 dòng."); return; }
        SellMacroStep step{};
        step.description = L"Bước " + std::to_wstring(a->profile.sellMacro.size() + 1);
        a->profile.sellMacro.push_back(step);
        SaveProfile(a->profile);
        RefreshSellMacroList();
        const int row = static_cast<int>(a->profile.sellMacro.size() - 1);
        ListView_SetItemState(sellMacroList_, row, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        ListView_EnsureVisible(sellMacroList_, row, FALSE);
        LoadSelectedMacroEditor();
    }

    void DeleteSellMacroRow() {
        Account* a = SelectedAccount();
        const int index = SelectedMacroIndex();
        if (!a || index < 0 || index >= static_cast<int>(a->profile.sellMacro.size())) {
            Log(L"Chưa chọn dòng macro để xóa"); return;
        }
        a->profile.sellMacro.erase(a->profile.sellMacro.begin() + index);
        SaveProfile(a->profile);
        RefreshSellMacroList();
        ClearSellMacroEditor();
    }

    void SaveSellMacroRow() {
        Account* a = SelectedAccount();
        const int index = SelectedMacroIndex();
        if (!a || index < 0 || index >= static_cast<int>(a->profile.sellMacro.size())) {
            Log(L"Chưa chọn dòng macro để lưu"); return;
        }
        SellMacroStep& step = a->profile.sellMacro[static_cast<std::size_t>(index)];
        step.description = GetText(sellDesc_);
        int delay = _wtoi(GetText(sellDelay_).c_str());
        int repeat = _wtoi(GetText(sellRepeat_).c_str());
        if (delay < 50) delay = 50;
        if (delay > 60000) delay = 60000;
        if (repeat < 1) repeat = 1;
        if (repeat > 999) repeat = 999;
        step.delayMs = delay;
        step.repeat = repeat;
        SaveProfile(a->profile);
        RefreshSellMacroList();
        ListView_SetItemState(sellMacroList_, index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
        LoadSelectedMacroEditor();
    }

    void BeginMacroCapture() {
        Account* a = SelectedAccount();
        const int index = SelectedMacroIndex();
        if (!a || index < 0 || index >= static_cast<int>(a->profile.sellMacro.size())) {
            Log(L"Chưa chọn dòng macro để lấy tọa độ"); return;
        }
        captureSlot_ = ClickSlot::None;
        captureMacroIndex_ = index;
        capturePid_ = a->game.pid;
        LogAccount(*a, L"Đang chờ F8 cho macro bán dòng " + std::to_wstring(index + 1));
        SetText(selected_, L"LẤY TỌA ĐỘ MACRO DÒNG " + std::to_wstring(index + 1) + L" • đưa chuột vào game rồi F8");
    }

    void TestSellMacroRow() {
        Account* a = SelectedAccount();
        const int index = SelectedMacroIndex();
        if (!a || index < 0 || index >= static_cast<int>(a->profile.sellMacro.size())) {
            Log(L"Chưa chọn dòng macro để TEST"); return;
        }
        const SellMacroStep& step = a->profile.sellMacro[static_cast<std::size_t>(index)];
        POINT point{}; std::wstring error;
        if (!ScaleClickPoint(a->game, step.point, point, error)) {
            LogAccount(*a, L"TEST macro FAIL: " + error); return;
        }
        if (!RealInputClick(*a, point, error)) {
            LogAccount(*a, L"TEST macro REAL INPUT FAIL: " + error); return;
        }
        LogAccount(*a, L"TEST macro dòng " + std::to_wstring(index + 1) + L" PASS • chuột giữ tại game");
    }

    void LoadSellNpcPositionToUi(const Account& a) {
        int index = a.profile.sellNpcPreset;
        if (index < 0 || index >= static_cast<int>(kSellNpcs.size())) index = 0;
        const SellNpcPosition& pos = sellNpcPositions_[static_cast<std::size_t>(index)];
        if (pos.valid) {
            SetText(sellNpcX_, std::to_wstring(pos.x));
            SetText(sellNpcY_, std::to_wstring(pos.y));
            SetText(sellNpcPosText_, L"M" + std::to_wstring(kSellNpcs[static_cast<std::size_t>(index)].mapID) + L" • " +
                                     std::to_wstring(pos.x) + L"," + std::to_wstring(pos.y));
        } else {
            SetText(sellNpcX_, L"");
            SetText(sellNpcY_, L"");
            SetText(sellNpcPosText_, L"CHƯA LẤY");
        }
    }

    void PersistSellNpcPositionEditor(Account& a) {
        int index = a.profile.sellNpcPreset;
        if (index < 0 || index >= static_cast<int>(kSellNpcs.size())) index = 0;
        const std::wstring xText = GetText(sellNpcX_);
        const std::wstring yText = GetText(sellNpcY_);
        SellNpcPosition& pos = sellNpcPositions_[static_cast<std::size_t>(index)];
        if (xText.empty() || yText.empty()) {
            pos = SellNpcPosition{};
            SaveSharedSellNpcPositions(sellNpcPositions_);
            return;
        }
        const int x = _wtoi(xText.c_str());
        const int y = _wtoi(yText.c_str());
        if (x < 0 || y < 0) {
            pos = SellNpcPosition{};
            SaveSharedSellNpcPositions(sellNpcPositions_);
            return;
        }
        pos.x = x;
        pos.y = y;
        pos.valid = true;
        SaveSharedSellNpcPositions(sellNpcPositions_);
    }

    void OnSellNpcSelectionChanged() {
        Account* a = SelectedAccount();
        if (!a) return;
        PersistSellNpcPositionEditor(*a);
        const LRESULT sellSel = SendMessageW(sellNpcCombo_, CB_GETCURSEL, 0, 0);
        if (sellSel != CB_ERR && sellSel >= 0 && sellSel < static_cast<LRESULT>(kSellNpcs.size())) {
            a->profile.sellNpcPreset = static_cast<int>(sellSel);
        }
        SaveProfile(a->profile);
        LoadSellNpcPositionToUi(*a);
    }

    void CaptureSellNpcPosition() {
        Account* a = SelectedAccount();
        if (!a) { Log(L"Chưa chọn acc để lấy tọa NPC"); return; }
        std::wstring error;
        if (!ReadSnapshot(*a, error, 1200)) {
            LogAccount(*a, L"Không đọc được state để lấy tọa NPC: " + error);
            return;
        }
        int index = a->profile.sellNpcPreset;
        if (index < 0 || index >= static_cast<int>(kSellNpcs.size())) index = 0;
        const SellNpcPreset& npc = kSellNpcs[static_cast<std::size_t>(index)];
        const Snapshot& snap = a->snapshot;
        if ((snap.validMask & (ValidMap | ValidPosition)) != (ValidMap | ValidPosition)) {
            LogAccount(*a, L"State chưa có Map/X/Y để lấy tọa NPC");
            return;
        }
        if (snap.mapID != npc.mapID) {
            LogAccount(*a, L"Không lưu: đang ở MapID " + std::to_wstring(snap.mapID) +
                           L" nhưng NPC đã chọn thuộc MapID " + std::to_wstring(npc.mapID));
            return;
        }
        SellNpcPosition& pos = sellNpcPositions_[static_cast<std::size_t>(index)];
        pos.x = snap.x;
        pos.y = snap.y;
        pos.valid = true;
        SaveSharedSellNpcPositions(sellNpcPositions_);
        LoadSellNpcPositionToUi(*a);
        LogAccount(*a, L"ĐÃ LẤY TỌA NPC • " + std::wstring(npc.name) + L" • " +
                       std::to_wstring(pos.x) + L"," + std::to_wstring(pos.y));
    }

    void ClearEditor() {
        SetText(selected_, L"ACC ĐANG CHỈNH: chưa chọn");
        SetText(live_, L"STATE: chưa có");
        SetText(targetName_, L"");
        if (spotCombo_) SendMessageW(spotCombo_, CB_SETCURSEL, -1, 0);
        SetText(targetText_, L"CHƯA CHỌN");
        SetText(tolerance_, L"120");
        SendMessageW(enableRevive_, BM_SETCHECK, BST_UNCHECKED, 0);
        SetText(rotateDeathLimit_, std::to_wstring(kRotateDeathLimitDefault));
        SetText(rotateDeathWindow_, std::to_wstring(kRotateDeathWindowMinDefault));
        SetText(rotateNoFullBag_, std::to_wstring(kRotateNoFullBagMinDefault));
        if (rotationList_) ListView_DeleteAllItems(rotationList_);
        SendMessageW(enableFight_, BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessageW(enableSell_, BM_SETCHECK, BST_UNCHECKED, 0);
        if (sellNpcCombo_) SendMessageW(sellNpcCombo_, CB_SETCURSEL, 0, 0);
        SetText(sellNpcX_, L"");
        SetText(sellNpcY_, L"");
        SetText(sellNpcPosText_, L"CHƯA LẤY");
        for (HWND h : pointLabels_) SetText(h, L"CHƯA LẤY");
        RefreshSellMacroList();
        ClearSellMacroEditor();
    }

    void LoadSelectedProfileToUi() {
        Account* a = SelectedAccount();
        if (!a) { ClearEditor(); return; }
        ResolveProfileTarget(a->profile);
        SetText(selected_, L"ACC ĐANG CHỈNH: " + AccountTag(*a));
        RefreshSpotCombo();
        RefreshRotationList();
        SetText(targetName_, a->profile.selectedSpot);
        SetText(tolerance_, std::to_wstring(a->profile.tolerance));
        SetText(rotateDeathLimit_, std::to_wstring(a->profile.rotateDeathLimit));
        SetText(rotateDeathWindow_, std::to_wstring(a->profile.rotateDeathWindowMin));
        SetText(rotateNoFullBag_, std::to_wstring(a->profile.rotateNoFullBagMin));
        SendMessageW(enableRevive_, BM_SETCHECK, a->profile.enableRevive ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(enableFight_, BM_SETCHECK, a->profile.enableFight ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(enableSell_, BM_SETCHECK, a->profile.enableSell ? BST_CHECKED : BST_UNCHECKED, 0);
        SendMessageW(sellNpcCombo_, CB_SETCURSEL, a->profile.sellNpcPreset, 0);
        LoadSellNpcPositionToUi(*a);
        if (a->profile.target.valid) {
            SetText(targetText_, L"M" + std::to_wstring(a->profile.target.mapID) + L" • " +
                                std::to_wstring(a->profile.target.x) + L"," + std::to_wstring(a->profile.target.y));
        } else {
            SetText(targetText_, L"CHƯA CHỌN");
        }
        for (int i = 0; i < 4; ++i) {
            SetText(pointLabels_[static_cast<std::size_t>(i)], PointDescription(a->profile.points[static_cast<std::size_t>(i)]));
        }
        RefreshSellMacroList();
        ClearSellMacroEditor();
        UpdateSelectedLive();
    }

    void PersistSelectedEditor() {
        Account* a = SelectedAccount();
        if (!a) return;
        int tol = _wtoi(GetText(tolerance_).c_str());
        if (tol < 20) tol = 20;
        if (tol > 2000) tol = 2000;
        a->profile.tolerance = tol;
        a->profile.enableRevive = SendMessageW(enableRevive_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        int deathLimit = _wtoi(GetText(rotateDeathLimit_).c_str());
        if (deathLimit < kRotateDeathLimitMin) deathLimit = kRotateDeathLimitMin;
        if (deathLimit > kRotateDeathLimitMax) deathLimit = kRotateDeathLimitMax;
        int deathWindow = _wtoi(GetText(rotateDeathWindow_).c_str());
        if (deathWindow < kRotateWindowMin) deathWindow = kRotateWindowMin;
        if (deathWindow > kRotateWindowMax) deathWindow = kRotateWindowMax;
        int noBagWindow = _wtoi(GetText(rotateNoFullBag_).c_str());
        if (noBagWindow < kRotateWindowMin) noBagWindow = kRotateWindowMin;
        if (noBagWindow > kRotateWindowMax) noBagWindow = kRotateWindowMax;
        a->profile.rotateDeathLimit = deathLimit;
        a->profile.rotateDeathWindowMin = deathWindow;
        a->profile.rotateNoFullBagMin = noBagWindow;
        SetText(rotateDeathLimit_, std::to_wstring(deathLimit));
        SetText(rotateDeathWindow_, std::to_wstring(deathWindow));
        SetText(rotateNoFullBag_, std::to_wstring(noBagWindow));
        PersistRotationListFromUi(*a);
        a->profile.enableFight = SendMessageW(enableFight_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        a->profile.enableSell = SendMessageW(enableSell_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        PersistSellNpcPositionEditor(*a);
        const LRESULT sellSel = SendMessageW(sellNpcCombo_, CB_GETCURSEL, 0, 0);
        if (sellSel != CB_ERR && sellSel >= 0 && sellSel < static_cast<LRESULT>(kSellNpcs.size())) a->profile.sellNpcPreset = static_cast<int>(sellSel);
        SaveProfile(a->profile);
        const int row = SelectedIndex();
        if (row >= 0) UpdateAccountRow(row, *a);
    }

    void SaveTargetForSelected() {
        Account* a = SelectedAccount();
        if (!a) { Log(L"Chưa chọn acc"); return; }
        PersistSelectedEditor();
        std::wstring error;
        if (!ReadSnapshot(*a, error, 1200)) { LogAccount(*a, L"Không đọc được state để lưu bãi: " + error); return; }
        const Snapshot& s = a->snapshot;
        if (!s.mapReady || s.waitingChangeMap ||
            (s.validMask & (ValidMap | ValidPosition)) != (ValidMap | ValidPosition)) {
            LogAccount(*a, L"State chưa ổn định, không lưu bãi.");
            return;
        }
        std::wstring name = GetText(targetName_);
        if (name.empty()) name = L"Bãi M" + std::to_wstring(s.mapID) + L" " + std::to_wstring(s.x) + L"," + std::to_wstring(s.y);
        TargetProfile spot{name, s.mapID, s.x, s.y, true};
        const int existing = FindSpotIndex(spots_, name);
        if (existing >= 0) spots_[static_cast<std::size_t>(existing)] = spot;
        else spots_.push_back(spot);
        SaveSharedSpots(spots_);
        a->profile.selectedSpot = name;
        a->profile.target = spot;
        if (!RotationContains(a->profile, name)) a->profile.rotationSpots.push_back(name);
        NormalizeRotationProfile(a->profile);
        SaveProfile(a->profile);
        RefreshSpotCombo();
        LoadSelectedProfileToUi();
        for (std::size_t i = 0; i < accounts_.size(); ++i) {
            if (_wcsicmp(accounts_[i]->profile.selectedSpot.c_str(), name.c_str()) == 0) {
                accounts_[i]->profile.target = spot;
                SaveProfile(accounts_[i]->profile);
                UpdateAccountRow(static_cast<int>(i), *accounts_[i]);
            }
        }
        LogAccount(*a, L"Đã lưu/cập nhật bãi CHUNG: " + name + L" • M" + std::to_wstring(s.mapID) + L" • " +
                       std::to_wstring(s.x) + L"," + std::to_wstring(s.y));
    }

    void BeginCapture(ClickSlot slot) {
        Account* a = SelectedAccount();
        if (!a) { Log(L"Chưa chọn acc để lấy tọa độ"); return; }
        captureSlot_ = slot;
        captureMacroIndex_ = -1;
        capturePid_ = a->game.pid;
        const int index = static_cast<int>(slot);
        LogAccount(*a, L"Đang chờ F8 để lấy điểm " + std::wstring(kClickLabels[static_cast<std::size_t>(index)]) + L".");
        SetText(selected_, L"LẤY TỌA ĐỘ CHO " + AccountTag(*a) + L" • đưa chuột vào nút rồi F8");
    }

    void CaptureHotkeyPoint() {
        if ((captureSlot_ == ClickSlot::None && captureMacroIndex_ < 0) || capturePid_ == 0) return;
        Account* a = AccountByPid(capturePid_);
        if (!a || !IsWindow(a->game.window)) {
            Log(L"Lấy tọa độ thất bại: acc/cửa sổ đã mất.");
            captureSlot_ = ClickSlot::None; captureMacroIndex_ = -1; capturePid_ = 0;
            return;
        }
        POINT screen{};
        if (!GetCursorPos(&screen)) return;
        POINT client = screen;
        if (!ScreenToClient(a->game.window, &client)) return;
        RECT rc{};
        if (!GetClientRect(a->game.window, &rc)) return;
        const int width = rc.right - rc.left;
        const int height = rc.bottom - rc.top;
        if (client.x < 0 || client.y < 0 || client.x >= width || client.y >= height) {
            LogAccount(*a, L"F8 bỏ qua: con trỏ không nằm trong client game của acc này.");
            return;
        }
        const ClickPoint captured{client.x, client.y, width, height, true};
        if (captureMacroIndex_ >= 0) {
            if (captureMacroIndex_ >= static_cast<int>(a->profile.sellMacro.size())) {
                LogAccount(*a, L"F8 macro thất bại: dòng đã bị xóa.");
            } else {
                a->profile.sellMacro[static_cast<std::size_t>(captureMacroIndex_)].point = captured;
                SaveProfile(a->profile);
                LogAccount(*a, L"Đã lưu macro dòng " + std::to_wstring(captureMacroIndex_ + 1) + L" = " + PointDescription(captured));
            }
        } else {
            const int index = static_cast<int>(captureSlot_);
            if (index >= 0 && index < 4) {
                a->profile.points[static_cast<std::size_t>(index)] = captured;
                SaveProfile(a->profile);
                LogAccount(*a, L"Đã lưu " + std::wstring(kClickLabels[static_cast<std::size_t>(index)]) + L" = " + PointDescription(captured));
            }
        }
        if (SelectedAccount() == a) LoadSelectedProfileToUi();
        captureSlot_ = ClickSlot::None; captureMacroIndex_ = -1; capturePid_ = 0;
    }

    bool RealInputClick(Account& a, const POINT& clientPoint, std::wstring& error) {
        if (!IsWindow(a.game.window) || !IsWindowVisible(a.game.window)) {
            error = L"Cửa sổ game không tồn tại/không visible"; return false;
        }
        if (IsIconic(a.game.window)) {
            error = L"Cửa sổ game đang minimized; không click để tránh sai mục tiêu"; return false;
        }
        POINT screen = clientPoint;
        if (!ClientToScreen(a.game.window, &screen)) { error = L"ClientToScreen thất bại"; return false; }
        const DWORD selfTid = GetCurrentThreadId();
        bool attachedGame = false;
        if (a.game.threadId != 0 && a.game.threadId != selfTid) {
            attachedGame = AttachThreadInput(selfTid, a.game.threadId, TRUE) != FALSE;
        }
        (void)SetForegroundWindow(a.game.window);
        (void)BringWindowToTop(a.game.window);
        Sleep(kMouseSettleMs);
        if (GetForegroundWindow() != a.game.window) {
            if (attachedGame) AttachThreadInput(selfTid, a.game.threadId, FALSE);
            error = L"Không đưa được đúng acc lên foreground; chặn click để tránh bấm nhầm"; return false;
        }
        if (!SetCursorPos(screen.x, screen.y)) {
            if (attachedGame) AttachThreadInput(selfTid, a.game.threadId, FALSE);
            error = L"SetCursorPos thất bại"; return false;
        }
        Sleep(kMouseSettleMs);
        INPUT down{}; down.type = INPUT_MOUSE; down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        if (SendInput(1, &down, sizeof(INPUT)) != 1) {
            if (attachedGame) AttachThreadInput(selfTid, a.game.threadId, FALSE);
            error = L"SendInput LEFTDOWN thất bại"; return false;
        }
        Sleep(kMouseHoldMs);
        INPUT up{}; up.type = INPUT_MOUSE; up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        if (SendInput(1, &up, sizeof(INPUT)) != 1) {
            if (attachedGame) AttachThreadInput(selfTid, a.game.threadId, FALSE);
            error = L"SendInput LEFTUP thất bại"; return false;
        }
        Sleep(kMouseAfterMs);
        if (attachedGame) AttachThreadInput(selfTid, a.game.threadId, FALSE);
        const DWORD clickedAt = GetTickCount();
        a.runtime.lastRealInputTick = clickedAt;
        globalLastRealInputTick_ = clickedAt;
        // Deliberately keep game foreground and leave cursor at the click point: temporary mouse-lock mode requested for reliability.
        return true;
    }

    bool ClickSlotNow(Account& a, ClickSlot slot, const wchar_t* reason, bool verbose = true) {
        if (a.runtime.running && a.runtime.clientFreezeActive) {
            if (verbose) LogAccount(a, L"CLICK bị chặn: client/map đang FREEZE");
            return false;
        }
        const int index = static_cast<int>(slot);
        if (index < 0 || index >= 4) return false;
        POINT point{};
        std::wstring error;
        if (!ScaleClickPoint(a.game, a.profile.points[static_cast<std::size_t>(index)], point, error)) {
            if (verbose) LogAccount(a, L"CLICK " + std::wstring(kClickLabels[static_cast<std::size_t>(index)]) + L" FAIL: " + error);
            return false;
        }
        if (!RealInputClick(a, point, error)) {
            if (verbose) LogAccount(a, L"REAL CLICK " + std::wstring(kClickLabels[static_cast<std::size_t>(index)]) + L" FAIL: " + error);
            return false;
        }
        if (verbose) {
            std::wstring line = L"REAL CLICK " + std::wstring(kClickLabels[static_cast<std::size_t>(index)]) +
                                L" -> " + std::to_wstring(point.x) + L"," + std::to_wstring(point.y);
            if (reason && *reason) line += L" • giữ chuột 140ms • KHÔNG restore";
            LogAccount(a, line);
        }
        return true;
    }


    bool InternalUiAction(Account& a, Command command, const wchar_t* label, bool verbose = true) {
        if (a.runtime.running && a.runtime.clientFreezeActive) {
            if (verbose) LogAccount(a, std::wstring(label ? label : L"UI nội bộ") + L" bị chặn: client/map đang FREEZE");
            return false;
        }
        Response r{};
        std::wstring error;
        const bool ok = a.bridge.Call(command, 0, 0, 0, r, error, 1600);
        if (!ok && BridgeLooksUnresponsive(error)) {
            EnterClientFreeze(a, L"Bridge internal UIButton timeout/busy", GetTickCount());
        }
        if (verbose) {
            if (ok) LogAccount(a, std::wstring(label ? label : L"UI nội bộ") + L" PASS • " + std::wstring(r.detail));
            else LogAccount(a, std::wstring(label ? label : L"UI nội bộ") + L" FAIL-CLOSED • " + error);
        }
        return ok;
    }

    void TestClick(ClickSlot slot) {
        Account* a = SelectedAccount();
        if (!a) { Log(L"TEST: chưa chọn acc"); return; }
        const int index = static_cast<int>(slot);
        if (!a->profile.points[static_cast<std::size_t>(index)].valid) {
            LogAccount(*a, L"TEST bỏ qua: chưa lấy điểm " + std::wstring(kClickLabels[static_cast<std::size_t>(index)]));
            return;
        }
        (void)ClickSlotNow(*a, slot, L"TEST MANUAL");
    }

    void StartChecked() {
        PersistSelectedEditor();
        int started = 0;
        const int count = ListView_GetItemCount(clientList_);
        for (int i = 0; i < count && i < static_cast<int>(accounts_.size()); ++i) {
            if (!ListView_GetCheckState(clientList_, i)) continue;
            Account& a = *accounts_[static_cast<std::size_t>(i)];
            if (!a.profile.target.valid) {
                LogAccount(a, L"Không start: acc chưa chọn bãi chung.");
                continue;
            }
            std::wstring error;
            if (!EnsureAttach(a, error)) {
                LogAccount(a, L"Không start: " + error);
                continue;
            }
            a.deathSessionLatched = false;
            a.rotationDeathTicks.clear();
            a.rotationMetricTick = GetTickCount();
            a.rotationActiveTrainMs = 0;
            a.rotationBagWasFull = false;
            a.runtime.running = true;
            ResetRuntime(a.runtime);
            a.runtime.running = true;
            a.runtime.routeOwnershipResetPending = true;
            a.runtime.status = L"Đang giám sát • chuẩn hóa ownership AutoPath";
            ++started;
            LogAccount(a, L"BẮT ĐẦU • bãi " + a.profile.target.name + L" • M" +
                           std::to_wstring(a.profile.target.mapID) + L" • " +
                           std::to_wstring(a.profile.target.x) + L"," + std::to_wstring(a.profile.target.y) +
                           L" • vòng " + std::to_wstring(a.profile.rotationSpots.size()) + L" bãi • chết quá " +
                           std::to_wstring(a.profile.rotateDeathLimit) + L"/" + std::to_wstring(a.profile.rotateDeathWindowMin) +
                           L" phút • chưa FULL túi " + std::to_wstring(a.profile.rotateNoFullBagMin) + L" phút");
            UpdateAccountRow(i, a);
        }
        if (started == 0) Log(L"Không có acc hợp lệ được start. Hãy tick checkbox và chọn bãi chung cho acc.");
    }

    void StopAccount(Account& a) {
        const bool wasFrozen = a.runtime.clientFreezeActive;
        a.deathSessionLatched = false;
        a.rotationDeathTicks.clear();
        a.rotationMetricTick = 0;
        a.rotationActiveTrainMs = 0;
        a.rotationBagWasFull = false;
        a.runtime.running = false;
        ResetRuntime(a.runtime);
        a.runtime.running = false;
        a.runtime.status = L"Đã dừng";
        if (a.bridge.Attached() && !wasFrozen) {
            Response r{};
            std::wstring ignored;
            (void)a.bridge.Call(Command::StopPath, 0, 0, 0, r, ignored, 700);
        }
        LogAccount(a, L"Đã dừng. Không tự đổi trạng thái ngựa.");
    }

    void StopChecked() {
        int stopped = 0;
        const int count = ListView_GetItemCount(clientList_);
        for (int i = 0; i < count && i < static_cast<int>(accounts_.size()); ++i) {
            if (!ListView_GetCheckState(clientList_, i)) continue;
            StopAccount(*accounts_[static_cast<std::size_t>(i)]);
            UpdateAccountRow(i, *accounts_[static_cast<std::size_t>(i)]);
            ++stopped;
        }
        if (stopped == 0) Log(L"Không có acc nào được tick để dừng.");
    }

    static bool BridgeLooksUnresponsive(const std::wstring& error) {
        return error.find(L"timeout") != std::wstring::npos ||
               error.find(L"Bridge còn bận") != std::wstring::npos ||
               error.find(L"Bridge busy") != std::wstring::npos;
    }

    bool WindowResponsive(const GameClient& game) const {
        if (!game.window || !IsWindow(game.window)) return false;
        DWORD_PTR ignored = 0;
        const LRESULT ok = SendMessageTimeoutW(game.window, WM_NULL, 0, 0,
                                               SMTO_ABORTIFHUNG | SMTO_BLOCK,
                                               kWindowResponsiveProbeMs, &ignored);
        return ok != 0;
    }

    void EnterClientFreeze(Account& a, const wchar_t* reason, DWORD now) {
        RuntimeState& rt = a.runtime;
        const bool first = !rt.clientFreezeActive;
        rt.clientFreezeActive = true;
        if (rt.clientFreezeSinceTick == 0) rt.clientFreezeSinceTick = now;
        rt.clientStableSinceTick = 0;
        rt.candidateCount = 0;
        rt.qualifiedMap = 0;
        rt.stallSinceTick = 0;
        rt.fightPhase = 0;
        if (first) {
            LogAccount(a, L"FREEZE ACTION: " + std::wstring(reason ? reason : L"client/map chưa ổn định"));
        }
    }

    void MarkReadStateFailure(Account& a, const std::wstring& error, DWORD now) {
        RuntimeState& rt = a.runtime;
        EnterClientFreeze(a, L"ReadState/Bridge không phản hồi", now);
        ++rt.readStateFailStreak;
        rt.clientStableSinceTick = 0;
        rt.status = L"CLIENT KHÔNG PHẢN HỒI • FREEZE ACTION";
        if (rt.lastReadFailureLogTick == 0 || now - rt.lastReadFailureLogTick >= kReadFailLogIntervalMs) {
            LogAccount(a, L"ReadState fail x" + std::to_wstring(rt.readStateFailStreak) + L": " + error +
                          L" • FREEZE, không gửi action mới");
            rt.lastReadFailureLogTick = now;
        }
    }

    bool HoldUntilClientStable(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;

        if (!s.mapReady || s.waitingChangeMap) {
            EnterClientFreeze(a, L"game đang chuyển map", now);
            rt.clientStableSinceTick = 0;
            rt.status = L"ĐANG CHUYỂN MAP • FREEZE ACTION";
            return true;
        }

        if (!rt.clientFreezeActive) {
            rt.readStateFailStreak = 0;
            rt.lastReadFailureLogTick = 0;
            return false;
        }

        if (!WindowResponsive(a.game)) {
            rt.clientStableSinceTick = 0;
            rt.status = L"CỬA SỔ GAME CHƯA PHẢN HỒI • FREEZE ACTION";
            return true;
        }

        if (rt.clientStableSinceTick == 0) {
            rt.clientStableSinceTick = now;
            rt.status = L"MAP/CLIENT ĐÃ PHẢN HỒI • chờ ổn định 2.0s";
            return true;
        }
        if (!Elapsed(now, rt.clientStableSinceTick, kClientStableResumeMs)) {
            const DWORD elapsed = now - rt.clientStableSinceTick;
            const DWORD remainMs = elapsed >= kClientStableResumeMs ? 0 : kClientStableResumeMs - elapsed;
            rt.status = L"CLIENT ĐANG ỔN ĐỊNH • chờ " + std::to_wstring((remainMs + 99) / 100) + L"00ms";
            return true;
        }

        rt.clientFreezeActive = false;
        rt.clientFreezeSinceTick = 0;
        rt.clientStableSinceTick = 0;
        rt.readStateFailStreak = 0;
        rt.lastReadFailureLogTick = 0;
        rt.lastActionTick = 0;
        rt.lastAction = Action::Wait;
        LogAccount(a, L"CLIENT ỔN ĐỊNH LIÊN TỤC 2s → mở khóa action, tiếp tục auto.");
        rt.status = L"Client ổn định 2s • tiếp tục auto";
        return false;
    }

    bool CooldownReady(RuntimeState& rt, Action a, DWORD now) {
        DWORD delay = 1500;
        if (a == Action::Mount || a == Action::Dismount) delay = 4000;
        if (a == Action::StartPath) delay = 5000;
        if (a != rt.lastAction) {
            rt.lastAction = a;
            rt.lastActionTick = 0;
        }
        return rt.lastActionTick == 0 || now - rt.lastActionTick >= delay;
    }

    bool SendDecision(Account& a, Action action, const TargetProfile& t, const wchar_t* context) {
        RuntimeState& rt = a.runtime;
        if (rt.clientFreezeActive) {
            rt.status = L"FREEZE ACTION • bỏ qua route/mount command";
            return false;
        }
        const DWORD now = GetTickCount();
        if (!CooldownReady(rt, action, now)) return false;
        Response r{};
        std::wstring error;
        bool ok = false;
        const std::wstring where = context ? context : L"đích";
        switch (action) {
            case Action::Mount:
                ok = a.bridge.Call(Command::ToggleRide, 1, 0, 0, r, error, 1000);
                rt.status = L"Đang lên ngựa • " + where;
                break;
            case Action::Dismount:
                ok = a.bridge.Call(Command::ToggleRide, 0, 0, 0, r, error, 1000);
                rt.status = L"Tới " + where + L" • xuống ngựa";
                break;
            case Action::StartPath:
                ok = a.bridge.Call(Command::StartPath, t.mapID, t.x, t.y, r, error, 1300);
                rt.status = L"Đang AutoPath tới " + where;
                break;
            case Action::StopPath:
                ok = a.bridge.Call(Command::StopPath, 0, 0, 0, r, error, 900);
                rt.status = L"Tới " + where + L" • StopPath";
                break;
            default:
                return false;
        }
        rt.lastActionTick = now;
        if (!ok && BridgeLooksUnresponsive(error)) {
            EnterClientFreeze(a, L"Bridge action timeout/busy", now);
        }
        if (ok && action == Action::StartPath && t.mapID != a.snapshot.mapID) {
            // Arm cross-map confirmation from the command itself. Movement/autoPath
            // evidence is still required before any Confirm click is allowed.
            if (!rt.crossMapRouteArmed) rt.crossMapRouteMoved = false;
            rt.crossMapRouteArmed = true;
        }
        if (!ok) LogAccount(a, L"Route action fail-closed: " + error);
        return ok;
    }

    void ObserveMovement(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        if (rt.lastObservedMap != s.mapID) {
            rt.lastObservedMap = s.mapID;
            rt.lastObservedX = s.x;
            rt.lastObservedY = s.y;
            rt.lastMovementTick = now;
            rt.crossMapSeenAutoPath = false;
            rt.crossMapRouteArmed = false;
            rt.crossMapRouteMoved = false;
            rt.stallSinceTick = 0;
            rt.confirmAttempts = 0;
            rt.fightPhase = 0;
            rt.fightAttempts = 0;
            rt.wasAtTarget = false;
            return;
        }
        const long long dx = static_cast<long long>(s.x) - rt.lastObservedX;
        const long long dy = static_cast<long long>(s.y) - rt.lastObservedY;
        if (dx * dx + dy * dy >= 25) {
            if (rt.crossMapRouteArmed) rt.crossMapRouteMoved = true;
            rt.lastMovementTick = now;
            rt.lastObservedX = s.x;
            rt.lastObservedY = s.y;
            rt.stallSinceTick = 0;
        }
    }

    void ResetRotationWindow(Account& a, DWORD now) {
        a.rotationDeathTicks.clear();
        a.rotationMetricTick = now;
        a.rotationActiveTrainMs = 0;
        a.rotationBagWasFull = false;
    }

    bool SwitchToNextRotationSpot(Account& a, DWORD now, const std::wstring& reason) {
        NormalizeRotationProfile(a.profile);
        const std::size_t count = a.profile.rotationSpots.size();
        if (count <= 1) {
            ResetRotationWindow(a, now);
            LogAccount(a, L"XOAY BÃI bỏ qua: chỉ tick 1 bãi • " + reason);
            SaveProfile(a.profile);
            return false;
        }
        std::size_t current = 0;
        for (std::size_t i = 0; i < count; ++i) {
            if (_wcsicmp(a.profile.rotationSpots[i].c_str(), a.profile.selectedSpot.c_str()) == 0) {
                current = i;
                break;
            }
        }
        const std::size_t next = NextRotationIndex(current, count);
        const std::wstring oldName = a.profile.selectedSpot;
        const std::wstring nextName = a.profile.rotationSpots[next];
        const int spotIndex = FindSpotIndex(spots_, nextName);
        if (spotIndex < 0) {
            a.profile.rotationSpots.erase(a.profile.rotationSpots.begin() + static_cast<std::ptrdiff_t>(next));
            NormalizeRotationProfile(a.profile);
            ResetRotationWindow(a, now);
            SaveProfile(a.profile);
            LogAccount(a, L"XOAY BÃI: bãi kế tiếp không còn trong data, đã loại khỏi vòng: " + nextName);
            return false;
        }
        a.profile.selectedSpot = nextName;
        a.profile.target = spots_[static_cast<std::size_t>(spotIndex)];
        a.profile.target.valid = true;
        ResetRotationWindow(a, now);
        SaveProfile(a.profile);
        if (SelectedAccount() == &a) LoadSelectedProfileToUi();
        LogAccount(a, L"XOAY BÃI: " + oldName + L" → " + nextName + L" • " + reason +
                      L" • M" + std::to_wstring(a.profile.target.mapID) + L" " +
                      std::to_wstring(a.profile.target.x) + L"," + std::to_wstring(a.profile.target.y));
        return true;
    }

    void RecordDeathForRotation(Account& a, DWORD now) {
        const DWORD windowMs = static_cast<DWORD>(a.profile.rotateDeathWindowMin) * 60u * 1000u;
        a.rotationDeathTicks.push_back(now);
        a.rotationDeathTicks.erase(std::remove_if(a.rotationDeathTicks.begin(), a.rotationDeathTicks.end(), [&](DWORD t){
            return static_cast<DWORD>(now - t) > windowMs;
        }), a.rotationDeathTicks.end());
        const std::size_t count = a.rotationDeathTicks.size();
        LogAccount(a, L"XOAY BÃI death-window: " + std::to_wstring(count) + L" chết / " +
                      std::to_wstring(a.profile.rotateDeathWindowMin) + L" phút");
        if (DeathLimitExceeded(count, a.profile.rotateDeathLimit)) {
            const std::wstring reason = L"chết quá " + std::to_wstring(a.profile.rotateDeathLimit) + L" lần / " +
                                        std::to_wstring(a.profile.rotateDeathWindowMin) + L" phút";
            (void)SwitchToNextRotationSpot(a, now, reason);
        }
    }

    bool UpdateRotationEfficiency(Account& a, DWORD now) {
        const Snapshot& s = a.snapshot;
        if (a.rotationMetricTick == 0) a.rotationMetricTick = now;
        DWORD delta = now - a.rotationMetricTick;
        a.rotationMetricTick = now;
        if (delta > 2000) delta = 2000;

        if (s.validMask & ValidBagSpace) {
            const bool full = s.freeBagSpace <= 0;
            if (full && !a.rotationBagWasFull) {
                a.rotationBagWasFull = true;
                a.rotationActiveTrainMs = 0;
                LogAccount(a, L"XOAY BÃI: ghi nhận 1 lần FULL túi → reset đồng hồ hiệu quả bãi.");
            } else if (!full) {
                a.rotationBagWasFull = false;
            }
        }

        bool activelyTraining = false;
        if (a.profile.target.valid &&
            (s.validMask & (ValidMap | ValidPosition | ValidAutoFight | ValidLifeState | ValidBagSpace)) ==
                (ValidMap | ValidPosition | ValidAutoFight | ValidLifeState | ValidBagSpace) &&
            !s.dead && s.autoFight) {
            State state{};
            state.valid = true; state.mapReady = true; state.waitingMap = false;
            state.mapID = s.mapID; state.x = s.x; state.y = s.y;
            Target target{a.profile.target.mapID, a.profile.target.x, a.profile.target.y, a.profile.tolerance};
            activelyTraining = AtTarget(state, target);
        }
        if (activelyTraining) a.rotationActiveTrainMs += delta;

        if (!NoFullBagWindowReached(a.rotationActiveTrainMs, a.profile.rotateNoFullBagMin)) return false;
        const std::wstring reason = L"train thực " + std::to_wstring(a.profile.rotateNoFullBagMin) + L" phút chưa FULL túi";
        if (!SwitchToNextRotationSpot(a, now, reason)) return false;
        BeginTrainRecovery(a, now);
        return true;
    }

    bool HandleDeath(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;

        // Life state is authoritative for the death-session boundary. If it becomes
        // temporarily unavailable while a death session is latched, fail closed and
        // preserve the latch/timers instead of silently returning to normal automation.
        if ((s.validMask & ValidLifeState) == 0) {
            if (a.deathSessionLatched) {
                rt.status = L"DEATH SESSION • chờ life-state authoritative";
                return true;
            }
            return false;
        }

        if (!s.dead) {
            if (!a.deathSessionLatched) return false;
            a.rotationMetricTick = now;

            // SECOND boundary reset: the character is alive again on a stable client
            // snapshot. Wipe every revive/travel/fight/sell/confirm/watchdog phase and
            // resume exactly like a fresh BẮT ĐẦU, while AccountProfile/settings and
            // the existing Bridge attachment remain intact.
            ResetRuntime(rt);
            a.deathSessionLatched = false;
            rt.routeOwnershipResetPending = true;
            rt.status = L"ALIVE • cold restart + chuẩn hóa ownership AutoPath";
            LogAccount(a, L"POST-REVIVE COLD START: ResetRuntime toàn bộ • giữ nguyên setting/bãi/click • phiên auto mới.");
            return true;
        }

        if (!a.deathSessionLatched) {
            a.rotationMetricTick = now;
            RecordDeathForRotation(a, now);
            // FIRST boundary reset: a new authoritative death is a hard session
            // boundary. Never carry ANY runtime state from the previous life. The
            // lifecycle latch is outside RuntimeState so this full reset cannot cause
            // a repeated-reset loop while the same dead snapshot remains true.
            ResetRuntime(rt);
            a.deathSessionLatched = true;
            rt.deadSinceTick = now;
            rt.status = L"DEAD • hard reset runtime đời trước";
            LogAccount(a, L"NEW DEATH SESSION: HARD ResetRuntime toàn bộ • coi như AUTO vừa được bật lại từ đầu.");

            // Preserve the donor's repeated-PK guard: stop only an authoritative active
            // AutoPath on the fresh death edge, then leave every other UI/path action
            // locked until the death overlay has settled. The current Bridge still owns
            // serialization; no donor remote-worker path is reintroduced.
            if ((s.validMask & ValidAutoPath) && s.autoPathing &&
                (s.validMask & ValidMapTransition) && s.mapReady && !s.waitingChangeMap) {
                (void)SendDecision(a, Action::StopPath, a.profile.target, L"death edge trước Đầu thai");
            }
        }

        rt.status = L"Nhân vật đang chết";
        if (!a.profile.enableRevive) {
            rt.status = L"CHẾT • Tự Đầu thai đang OFF";
            return true;
        }
        if (rt.revivePhase == 0 && Elapsed(now, rt.deadSinceTick, 500) &&
            (rt.lastReviveClickTick == 0 || Elapsed(now, rt.lastReviveClickTick, 2000))) {
            // Match the donor's conservative death-overlay cadence: failed scans retry
            // no faster than every 2s. Once a callback succeeds, lock the UI/path flow
            // instead of cycling the revive state every few seconds.
            rt.lastReviveClickTick = now;
            if (InternalUiAction(a, Command::ClickInternalRevive, L"ĐẦU THAI NỘI BỘ v0.8.7")) {
                rt.revivePhase = 1; // one successful callback sent
                rt.revivePhaseTick = now;
                rt.status = L"Đã gọi Đầu thai • khóa UI/path, chờ IsDeath tắt";
            } else {
                rt.status = L"CHẾT • chưa thấy nút Đầu thai hợp lệ • dò lại sau 2s";
            }
            return true;
        }
        if (rt.revivePhase == 1) {
            if (!Elapsed(now, rt.revivePhaseTick, 15000)) {
                rt.status = L"Đầu thai đã gửi • chờ IsDeath tắt • không click lặp";
                return true;
            }

            // v0.8.7 allows exactly one late retry after 15s, and only while the
            // authoritative death flag is still true (Bridge re-reads it immediately).
            // Regardless of retry result, phase 2 forbids further death-UI callbacks
            // until the ALIVE edge performs the full cold restart.
            rt.revivePhase = 2;
            rt.revivePhaseTick = now;
            const bool retryOk = InternalUiAction(a, Command::ClickInternalRevive,
                                                  L"ĐẦU THAI NỘI BỘ v0.8.7 • RETRY 1/1");
            rt.status = retryOk
                ? L"Đầu thai retry 1/1 đã gửi • tiếp tục chờ IsDeath tắt"
                : L"Đầu thai retry 1/1 fail-closed • không click thêm";
            return true;
        }
        if (rt.revivePhase == 2) {
            rt.status = L"Đã dùng tối đa 1 retry Đầu thai • chờ IsDeath tắt";
            return true;
        }
        return true;
    }

    bool HandleRouteOwnershipReset(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        if (!rt.routeOwnershipResetPending) return false;

        // This is the missing game-side half of a true cold start. ResetRuntime()
        // clears controller ownership flags, but the client may preserve AutoPath=ON
        // across death/revive. If we accepted that stale path as our route, then
        // crossMapRouteArmed would stay false forever and Confirm would fail closed.
        if ((s.validMask & ValidAutoPath) == 0) {
            rt.status = L"SESSION ROUTE RESET • chờ AutoPath authoritative";
            return true;
        }

        if (!s.autoPathing) {
            rt.routeOwnershipResetPending = false;
            rt.routeOwnershipStopTick = 0;
            rt.routeOwnershipStopAttempts = 0;
            rt.crossMapRouteArmed = false;
            rt.crossMapRouteMoved = false;
            rt.crossMapSeenAutoPath = false;
            rt.confirmAttempts = 0;
            rt.confirmUiFirstSeenTick = 0;
            rt.confirmStopPathTick = 0;
            rt.lastConfirmClickTick = 0;
            rt.suppressRouteSinceTick = 0;
            rt.status = L"SESSION ROUTE RESET • AutoPath OFF • ownership sạch";
            if (!rt.routeOwnershipResetLogged) {
                LogAccount(a, L"SESSION ROUTE RESET PASS: AutoPath OFF → route kế tiếp phải do tool StartPath mới để arm Confirm.");
                rt.routeOwnershipResetLogged = true;
            }
            return true; // one-cycle barrier before normal route logic
        }

        rt.routeOwnershipResetLogged = false;
        if (rt.routeOwnershipStopAttempts >= kRouteOwnershipStopMaxAttempts &&
            rt.routeOwnershipStopTick != 0 &&
            Elapsed(now, rt.routeOwnershipStopTick, kRouteOwnershipStopRetryMs)) {
            rt.status = L"SESSION ROUTE RESET • AutoPath cũ vẫn ON sau 3 StopPath • fail-closed";
            return true;
        }

        if (rt.routeOwnershipStopTick == 0 || Elapsed(now, rt.routeOwnershipStopTick, kRouteOwnershipStopRetryMs)) {
            if (SendDecision(a, Action::StopPath, a.profile.target, L"session route ownership reset")) {
                ++rt.routeOwnershipStopAttempts;
                rt.routeOwnershipStopTick = now;
                rt.status = L"SESSION ROUTE RESET • phát hiện AutoPath cũ ON → StopPath, chờ verify OFF";
                LogAccount(a, L"SESSION ROUTE RESET: AutoPath=ON nhưng controller vừa cold-reset → StopPath để xóa path đời trước trước khi route mới.");
            } else {
                rt.status = L"SESSION ROUTE RESET • chờ gửi StopPath fail-closed";
            }
        } else {
            rt.status = L"SESSION ROUTE RESET • đã StopPath → chờ snapshot AutoPath OFF";
        }
        return true;
    }

    bool HandleUnderworldAutoFightGuard(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;

        if (s.mapID != kUnderworldMapId) {
            rt.underworldStopPhase = 0;
            rt.underworldStopTick = 0;
            rt.underworldStopAttempts = 0;
            rt.underworldGuardLogged = false;
            return false;
        }

        // Địa Phủ is a recovery-only map for this flow. No train/sell/route action
        // may compete until the authoritative AutoFight getter proves OFF.
        if ((s.validMask & ValidAutoFight) == 0) {
            rt.status = L"ĐỊA PHỦ M87 • chờ đọc AutoFight authoritative";
            return true;
        }

        if (!s.autoFight) {
            if (!rt.underworldGuardLogged) {
                LogAccount(a, L"ĐỊA PHỦ M87: AutoFight OFF đã xác nhận → cho phép tiếp tục route về bãi.");
                rt.underworldGuardLogged = true;
            }
            rt.underworldStopPhase = 0;
            rt.underworldStopTick = 0;
            rt.underworldStopAttempts = 0;
            return false;
        }

        rt.underworldGuardLogged = false;
        if (!a.profile.points[static_cast<std::size_t>(ClickSlot::StopAuto1)].valid ||
            !a.profile.points[static_cast<std::size_t>(ClickSlot::StopAuto2)].valid) {
            rt.status = L"ĐỊA PHỦ M87 • AutoFight ON nhưng chưa lấy đủ DỪNG AUTO 1/2";
            return true;
        }

        if (rt.underworldStopAttempts >= kUnderworldMaxStopAttempts && rt.underworldStopPhase == 0) {
            rt.status = L"ĐỊA PHỦ M87 • không tắt được AutoFight sau 3 lần • chờ thủ công";
            return true;
        }

        if (rt.underworldStopPhase == 0) {
            if (ClickSlotNow(a, ClickSlot::StopAuto1, L"Địa Phủ M87: AutoFight ON → mở Dừng Auto", false)) {
                rt.underworldStopPhase = 1;
                rt.underworldStopTick = now;
                rt.status = L"ĐỊA PHỦ M87 • đã click DỪNG AUTO 1";
                LogAccount(a, L"ĐỊA PHỦ M87: AutoFight=ON → DỪNG AUTO 1.");
            }
            return true;
        }

        if (rt.underworldStopPhase == 1) {
            if (!Elapsed(now, rt.underworldStopTick, kUnderworldStopStepMs)) {
                rt.status = L"ĐỊA PHỦ M87 • chờ click DỪNG AUTO 2";
                return true;
            }
            if (ClickSlotNow(a, ClickSlot::StopAuto2, L"Địa Phủ M87: chọn dừng đánh", false)) {
                rt.underworldStopPhase = 2;
                rt.underworldStopTick = now;
                ++rt.underworldStopAttempts;
                rt.status = L"ĐỊA PHỦ M87 • đã click DỪNG AUTO 2 • verify OFF";
                LogAccount(a, L"ĐỊA PHỦ M87: DỪNG AUTO 2 → verify AutoFight OFF.");
            }
            return true;
        }

        if (!s.autoFight) {
            rt.underworldStopPhase = 0;
            rt.underworldStopTick = 0;
            rt.underworldStopAttempts = 0;
            rt.underworldGuardLogged = true;
            rt.status = L"ĐỊA PHỦ M87 • AutoFight OFF • tiếp tục route";
            LogAccount(a, L"ĐỊA PHỦ M87: verify AutoFight OFF PASS → tiếp tục auto train.");
            return false;
        }

        if (!Elapsed(now, rt.underworldStopTick, kUnderworldVerifyMs)) {
            rt.status = L"ĐỊA PHỦ M87 • AutoFight vẫn ON • đang verify";
            return true;
        }

        rt.underworldStopPhase = 0;
        rt.underworldStopTick = 0;
        rt.status = L"ĐỊA PHỦ M87 • AutoFight vẫn ON • retry dừng auto";
        LogAccount(a, L"ĐỊA PHỦ M87: AutoFight vẫn ON sau verify → retry chuỗi DỪNG AUTO.");
        return true;
    }

    bool HandleCrossMapConfirm(Account& a, DWORD now, const TargetProfile& spot) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;

        // Do NOT key ownership off the configured train-map ID. The same serialized
        // route engine is also used by recovery/sell travel, so the only authoritative
        // ownership boundary is a successful cross-map StartPath issued by this tool.
        // ObserveMovement() clears this ownership immediately when MapID really changes.

        // Only a route started by this tool may consume a MessageBox as a map-confirm
        // candidate.  This route ownership is the primary unrelated-dialog guard.
        if (!rt.crossMapRouteArmed) return false;
        if ((s.validMask & ValidMapTransition) && (!s.mapReady || s.waitingChangeMap)) {
            rt.status = L"Cross-map • game đang transition thật";
            return true;
        }

        // v1.5.3 correction from the v0.8.7 runtime-known-good donor:
        // AutoPath may remain logically ON while the character is already stopped at a
        // portal and MessageBox is visible.  Therefore Path ON is route evidence, NOT a
        // reason to return before inspecting the authoritative popup state.
        if (s.autoPathing) rt.crossMapSeenAutoPath = true;
        const bool routeEvidence = rt.crossMapSeenAutoPath || rt.crossMapRouteMoved;
        if (!routeEvidence) {
            rt.status = L"Cross-map đã arm • chưa có bằng chứng route → cho phép retry";
            return false;
        }

        if ((s.validMask & ValidConfirmUi) == 0) {
            rt.status = s.autoPathing
                ? L"Cross-map • Path ON • MessageBox ? → không click mù"
                : L"Cross-map • Confirm UI detector chưa authoritative → không click mù";
            return true;
        }

        if (!s.confirmUiVisible) {
            rt.confirmUiFirstSeenTick = 0;
            rt.confirmStopPathTick = 0;

            // After a successful click the MessageBox should disappear before the map changes.
            // Wait for real transition/map-id proof.  Timeout only re-arms route; it never
            // becomes evidence for another Confirm click.
            if (rt.lastConfirmClickTick != 0) {
                if (!Elapsed(now, rt.lastConfirmClickTick, 5000)) {
                    rt.status = L"MessageBox đã đóng • chờ chuyển map";
                    return true;
                }
                rt.crossMapSeenAutoPath = false;
                rt.crossMapRouteMoved = false;
                rt.confirmAttempts = 0;
                rt.lastConfirmClickTick = 0;
                rt.suppressRouteSinceTick = 0;
                rt.status = L"Confirm UI đã đóng nhưng map chưa đổi • cho phép AutoPath re-arm";
                return false;
            }

            rt.status = s.autoPathing ? L"Cross-map • Path ON • chờ MessageBox thật"
                                      : L"Cross-map • chờ MessageBox thật";
            return true;
        }

        // From this point the authoritative MessageBox is ON.  Require a trustworthy
        // movement observer and the character to be physically stopped, but do NOT wait
        // for AutoPath's logical flag to become OFF; the donor proves that flag can stay
        // ON at the portal while the popup is already waiting for the player.
        if ((s.validMask & ValidMoving) == 0) {
            rt.status = L"MessageBox ON • moving observer chưa authoritative → chờ";
            return true;
        }
        if (s.moving) {
            rt.confirmUiFirstSeenTick = 0;
            rt.confirmStopPathTick = 0;
            rt.status = L"MessageBox ON • nhân vật còn di chuyển → chờ dừng";
            return true;
        }
        if (rt.confirmUiFirstSeenTick == 0) {
            rt.confirmUiFirstSeenTick = now;
            rt.status = L"MessageBox ON • xác nhận ổn định popup";
            return true;
        }
        if (!Elapsed(now, rt.confirmUiFirstSeenTick, 200)) {
            rt.status = L"MessageBox ON • debounce 200ms";
            return true;
        }

        // v0.8.7 stopped AutoPath immediately before Confirm even when AutoPath still
        // reported running.  Preserve CleanRoute's serialized-action rule: StopPath in
        // one controller cycle, then use a fresh snapshot before the internal UIButton callback.
        if (s.autoPathing && rt.confirmStopPathTick == 0) {
            if (SendDecision(a, Action::StopPath, spot, L"cổng trước Xác nhận")) {
                rt.confirmStopPathTick = now;
                rt.status = L"MessageBox ON + Path ON → đã StopPath • chờ snapshot mới";
                LogAccount(a, L"INTERNAL CONFIRM v0.8.7: MessageBox=1 trong khi AutoPath=1 • StopPath trước callback Confirm.");
            } else {
                rt.status = L"MessageBox ON + Path ON • StopPath đang cooldown/fail-closed";
            }
            return true;
        }
        if (rt.confirmStopPathTick != 0 && !Elapsed(now, rt.confirmStopPathTick, 250)) {
            rt.status = L"Đã StopPath • chờ 250ms rồi Confirm";
            return true;
        }

        // If AutoPath's logical flag is still ON after our explicit StopPath request,
        // do not deadlock again.  MessageBox ON + tool-owned cross-map route + moving=OFF
        // is sufficient authoritative context to perform the internal UIButton callback.
        const bool debounceReady = rt.lastConfirmClickTick == 0 || Elapsed(now, rt.lastConfirmClickTick, 2500);
        if (debounceReady && rt.confirmAttempts < 2) {
            if (InternalUiAction(a, Command::ClickInternalConfirm, L"XÁC NHẬN RA MAP NỘI BỘ v0.8.7")) {
                rt.lastConfirmClickTick = now;
                ++rt.confirmAttempts;
                rt.suppressRouteSinceTick = now;
                rt.status = L"MessageBox ON → UIButton Confirm nội bộ đã gửi • chờ UI/map đổi";
                LogAccount(a, L"INTERNAL CONFIRM v0.8.7: MessageBox=1 • moving=0 • callback " +
                              std::to_wstring(rt.confirmAttempts) + L"/2 • Path=" +
                              std::wstring(s.autoPathing ? L"ON(after StopPath)" : L"OFF") + L".");
                // A successful Confirm click means a map transition is now expected even if
                // the client has not published WaitingChangeMap yet. Freeze immediately so
                // no second click/route/mount/AutoFight action races the loading transition.
                EnterClientFreeze(a, L"đã click Confirm • chờ map/client ổn định", now);
            }
        } else {
            rt.status = rt.confirmAttempts >= 2
                ? L"MessageBox vẫn còn sau 2 click • chờ thủ công"
                : L"MessageBox vẫn hiện • chờ debounce trước retry";
        }
        return true;
    }

    bool HandleFightClicks(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        if (!a.profile.enableFight) {
            rt.fightPhase = 0;
            rt.fightAttempts = 0;
            return false;
        }
        if (!a.profile.points[static_cast<std::size_t>(ClickSlot::AutoMenu)].valid ||
            !a.profile.points[static_cast<std::size_t>(ClickSlot::Attack)].valid) {
            rt.status = L"Đúng bãi • chưa lấy đủ AUTO/Đánh quái";
            return true;
        }
        // We have a real read-only AutoFight getter. Never click AUTO blindly when
        // the getter is temporarily unavailable: wait for a valid state instead.
        if ((s.validMask & ValidAutoFight) == 0) {
            rt.status = L"Đúng bãi • chờ đọc trạng thái AutoFight";
            return true;
        }
        if (s.autoFight) {
            rt.fightPhase = 3;
            rt.fightAttempts = 0;
            if (!rt.trainPositionMonitorArmed) {
                rt.trainPositionMonitorArmed = true;
                rt.lastTrainPositionCheckTick = now;
                LogAccount(a, L"AutoFight ON • bắt đầu check tọa độ train 3 phút/lần.");
            }
            rt.lastAutoFightCheckTick = now;
            rt.status = L"Đúng bãi • AutoFight ON • check Auto mỗi 1 phút";
            return true;
        }
        if (rt.fightAttempts >= 2) {
            rt.status = L"AUTO→Đánh quái thử 2 lần • chờ chu kỳ check Auto tiếp theo";
            rt.fightPhase = 3;
            rt.lastAutoFightCheckTick = now;
            return true;
        }
        if (rt.fightPhase == 3) rt.fightPhase = 0;
        if (rt.fightPhase == 0) {
            if (ClickSlotNow(a, ClickSlot::AutoMenu, L"AutoFight OFF, mở AUTO", false)) {
                rt.fightPhase = 1;
                rt.fightPhaseTick = now;
                rt.status = L"AutoFight OFF • đã click AUTO • chờ menu";
            }
            return true;
        }
        if (rt.fightPhase == 1 && Elapsed(now, rt.fightPhaseTick, 700)) {
            if (ClickSlotNow(a, ClickSlot::Attack, L"AutoFight OFF, chọn Đánh quái", false)) {
                rt.fightPhase = 2;
                rt.fightPhaseTick = now;
                ++rt.fightAttempts;
                rt.status = L"Đã click Đánh quái • verify AutoFight";
            }
            return true;
        }
        if (rt.fightPhase == 2 && Elapsed(now, rt.fightPhaseTick, 1500)) {
            if (s.autoFight) {
                rt.fightPhase = 3;
                rt.fightAttempts = 0;
                rt.lastAutoFightCheckTick = now;
                if (!rt.trainPositionMonitorArmed) {
                    rt.trainPositionMonitorArmed = true;
                    rt.lastTrainPositionCheckTick = now;
                }
                rt.status = L"AutoFight ON • bật thành công";
                LogAccount(a, L"AutoFight verify ON sau chuỗi AUTO→Đánh quái.");
                return true;
            }
            if (rt.fightAttempts < 2) {
                rt.fightPhase = 0;
                rt.fightPhaseTick = now;
                return true;
            }
        }
        return true;
    }

    bool AutoFightCheckBusy(const Account& a, DWORD now) const {
        const RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        // Hard exclusion gate: an AutoFight check/click sequence may run only when the
        // account is completely idle at the train spot. Do not interleave with any
        // route, mount, death, sell, recovery or another click operation.
        if (rt.sellPhase != 0 || rt.trainRecoveryPhase != 0 || rt.revivePhase != 0) return true;
        if (rt.confirmAttempts != 0 || rt.stallSinceTick != 0) return true;
        if (rt.crossMapRouteArmed || rt.crossMapRouteMoved) return true;
        if (rt.suppressRouteSinceTick != 0 && !Elapsed(now, rt.suppressRouteSinceTick, 2500)) return true;
        if (s.riding || s.autoPathing || s.waitingChangeMap || !s.mapReady) return true;
        if ((s.validMask & ValidLifeState) && s.dead) return true;
        return false;
    }

    void ResetRobustTravel(RuntimeState& rt) {
        rt.travelMountAttempts = 0;
        rt.travelMountTick = 0;
        rt.travelFootFallback = false;
        rt.travelFootTick = 0;
    }

    bool HandleRobustTravel(Account& a, DWORD now, const TargetProfile& targetProfile,
                            const wchar_t* context, bool& arrived) {
        arrived = false;
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        State logic{};
        logic.valid = true; logic.mapReady = true; logic.waitingMap = false;
        logic.mapID = s.mapID; logic.x = s.x; logic.y = s.y;
        logic.riding = s.riding != 0; logic.autoPathing = s.autoPathing != 0;
        Target target{targetProfile.mapID, targetProfile.x, targetProfile.y, a.profile.tolerance};
        const std::wstring where = context ? context : L"đích";

        if (AtTarget(logic, target)) {
            if (s.autoPathing) {
                (void)SendDecision(a, Action::StopPath, targetProfile, context);
                return true;
            }
            if (s.riding) {
                (void)SendDecision(a, Action::Dismount, targetProfile, context);
                return true;
            }
            ResetRobustTravel(rt);
            arrived = true;
            return true;
        }

        const DWORD phaseElapsed = rt.travelFootFallback
            ? (rt.travelFootTick == 0 ? 0 : now - rt.travelFootTick)
            : (rt.travelMountTick == 0 ? 0 : now - rt.travelMountTick);
        const MountAssistAction assist = DecideMountAssist(s.riding != 0, s.autoPathing != 0,
                                                           rt.travelMountAttempts, rt.travelFootFallback,
                                                           phaseElapsed, kMountRetryWaitMs, kFootWalkMaxMs);
        if (s.riding) {
            ResetRobustTravel(rt);
            if (assist == MountAssistAction::StartPath) (void)SendDecision(a, Action::StartPath, targetProfile, context);
            else rt.status = L"Đang cưỡi ngựa AutoPath tới " + where;
            return true;
        }
        if (rt.travelFootFallback) {
            if (assist == MountAssistAction::FinishFootCycle) {
                if (s.autoPathing) (void)SendDecision(a, Action::StopPath, targetProfile, context);
                ResetRobustTravel(rt);
                rt.status = L"Đã chạy bộ tối đa 15s • lặp lại lên ngựa";
                return true;
            }
            if (assist == MountAssistAction::StartPath) (void)SendDecision(a, Action::StartPath, targetProfile, context);
            const DWORD walked = rt.travelFootTick == 0 ? 0 : (now - rt.travelFootTick) / 1000;
            rt.status = L"Chạy bộ dự phòng tới " + where + L" • " + std::to_wstring(walked) + L"/15s";
            return true;
        }
        if (assist == MountAssistAction::Wait) {
            rt.status = rt.travelMountAttempts <= 1 ? L"Chờ lên ngựa lần 1 • tối đa 5s" : L"Chờ lên ngựa lần 2 • tối đa 5s";
            return true;
        }
        if (assist == MountAssistAction::Mount) {
            if (SendDecision(a, Action::Mount, targetProfile, context)) {
                ++rt.travelMountAttempts;
                if (rt.travelMountAttempts > 2) rt.travelMountAttempts = 2;
                rt.travelMountTick = now;
                rt.status = rt.travelMountAttempts == 1 ? L"Lên ngựa lần 1 • chờ 5s" : L"Lên ngựa lần 2 • chờ 5s";
            } else {
                rt.status = L"Chờ gửi lệnh lên ngựa • chưa tính lần thử";
            }
            return true;
        }
        // Two successfully-sent mount attempts timed out: StartAutoPath while still on foot.
        if (SendDecision(a, Action::StartPath, targetProfile, context)) {
            rt.travelFootFallback = true;
            rt.travelFootTick = now;
            rt.status = L"2 lần chưa lên ngựa • chạy bộ dự phòng tối đa 15s";
        } else {
            rt.status = L"2 lần chưa lên ngựa • chờ gửi lệnh chạy bộ";
        }
        return true;
    }

    void BeginTrainRecovery(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        rt.trainPositionMonitorArmed = false;
        rt.lastTrainPositionCheckTick = 0;
        rt.trainRecoveryPhase = 1;
        rt.trainRecoveryTick = now;
        rt.trainRecoveryStopAttempts = 0;
        rt.fightPhase = 0;
        rt.fightAttempts = 0;
        ResetRobustTravel(rt);
        LogAccount(a, L"CHECK 3 PHÚT: lệch bãi → tắt AUTO rồi quay lại tọa train.");
    }

    bool HandleTrainRecovery(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        if (rt.trainRecoveryPhase == 0) return false;

        if (rt.trainRecoveryPhase == 1) {
            if ((s.validMask & ValidAutoFight) && !s.autoFight) {
                rt.trainRecoveryPhase = 4; rt.trainRecoveryTick = now; ResetRobustTravel(rt);
                return true;
            }
            if (!a.profile.points[static_cast<std::size_t>(ClickSlot::StopAuto1)].valid ||
                !a.profile.points[static_cast<std::size_t>(ClickSlot::StopAuto2)].valid) {
                rt.status = L"Lệch bãi • thiếu 2 tọa DỪNG AUTO";
                return true;
            }
            if (ClickSlotNow(a, ClickSlot::StopAuto1, L"lệch bãi: mở dừng AUTO", false)) {
                rt.trainRecoveryPhase = 2; rt.trainRecoveryTick = now;
                rt.status = L"Lệch bãi • đã click DỪNG AUTO 1";
            }
            return true;
        }
        if (rt.trainRecoveryPhase == 2) {
            if (!Elapsed(now, rt.trainRecoveryTick, 750)) return true;
            if (ClickSlotNow(a, ClickSlot::StopAuto2, L"lệch bãi: dừng đánh", false)) {
                rt.trainRecoveryPhase = 3; rt.trainRecoveryTick = now; ++rt.trainRecoveryStopAttempts;
                rt.status = L"Lệch bãi • đã click DỪNG AUTO 2 • verify";
            }
            return true;
        }
        if (rt.trainRecoveryPhase == 3) {
            if (!Elapsed(now, rt.trainRecoveryTick, 1300)) return true;
            if ((s.validMask & ValidAutoFight) == 0 || !s.autoFight) {
                rt.trainRecoveryPhase = 4; rt.trainRecoveryTick = now; ResetRobustTravel(rt);
                rt.status = L"Đã tắt AUTO • quay về bãi";
                return true;
            }
            if (rt.trainRecoveryStopAttempts < 2) {
                rt.trainRecoveryPhase = 1; rt.trainRecoveryTick = now;
                return true;
            }
            rt.status = L"Lệch bãi nhưng chưa tắt được AUTO sau 2 lần • chờ thủ công";
            return true;
        }
        if (rt.trainRecoveryPhase == 4) {
            bool arrived = false;
            (void)HandleRobustTravel(a, now, a.profile.target, L"bãi train", arrived);
            if (arrived) {
                rt.trainRecoveryPhase = 0;
                rt.trainRecoveryTick = 0;
                rt.wasAtTarget = false;
                rt.fightPhase = 0;
                rt.fightAttempts = 0;
                rt.status = L"Đã về bãi • chuẩn bị bật lại Đánh quái";
                LogAccount(a, L"Đã quay lại bãi sau check lệch • chuẩn bị AUTO→Đánh quái.");
            }
            return true;
        }
        return true;
    }

    TargetProfile SellNpcTarget(const Account& a) const {
        const int presetIndex = (a.profile.sellNpcPreset >= 0 && a.profile.sellNpcPreset < static_cast<int>(kSellNpcs.size()))
            ? a.profile.sellNpcPreset : 0;
        const SellNpcPreset& npc = kSellNpcs[static_cast<std::size_t>(presetIndex)];
        TargetProfile t{};
        t.name = npc.name;
        t.mapID = npc.mapID;
        const SellNpcPosition& pos = sellNpcPositions_[static_cast<std::size_t>(presetIndex)];
        t.x = pos.x;
        t.y = pos.y;
        t.valid = pos.valid;
        return t;
    }

    bool SellMacroConfigured(const Account& a, std::wstring& reason) const {
        if (a.profile.sellMacro.empty()) { reason = L"chưa có dòng macro bán"; return false; }
        for (std::size_t i = 0; i < a.profile.sellMacro.size(); ++i) {
            if (!a.profile.sellMacro[i].point.valid) {
                reason = L"macro dòng " + std::to_wstring(i + 1) + L" chưa có tọa độ"; return false;
            }
        }
        return true;
    }

    void BeginAutoSell(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        rt.sellPhase = 1;
        rt.sellPhaseTick = now;
        rt.sellStopAttempts = 0;
        rt.sellOpenAttempts = 0;
        rt.sellMacroIndex = 0;
        rt.sellMacroRepeatDone = 0;
        rt.sellMacroNextTick = 0;
        rt.sellMacroPass = 0;
        rt.sellLastFreeBag = a.snapshot.freeBagSpace;
        rt.sellBagStableSince = 0;
        rt.sellTriggeredByFullBag = true;
        rt.trainPositionMonitorArmed = false;
        rt.lastTrainPositionCheckTick = 0;
        rt.trainRecoveryPhase = 0;
        rt.fightPhase = 0;
        rt.fightAttempts = 0;
        rt.wasAtTarget = false;
        rt.crossMapSeenAutoPath = false;
        rt.stallSinceTick = 0;
        rt.confirmAttempts = 0;
        rt.crossMapRouteArmed = false;
        rt.crossMapRouteMoved = false;
        ResetRobustTravel(rt);
        if (a.bridge.Attached()) {
            Response r{}; std::wstring error;
            if (!a.bridge.Call(Command::StopPath, 0, 0, 0, r, error, 700) && BridgeLooksUnresponsive(error)) {
                EnterClientFreeze(a, L"Bridge timeout lúc bắt đầu Auto Sell", now);
            }
        }
        const SellNpcPreset& npc = kSellNpcs[static_cast<std::size_t>(a.profile.sellNpcPreset)];
        LogAccount(a, L"TÚI FULL → bắt đầu bán • " + std::wstring(npc.name));
    }

    bool RunSellMacroClick(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        if (rt.sellMacroIndex >= static_cast<int>(a.profile.sellMacro.size())) {
            rt.sellPhase = 7;
            rt.sellPhaseTick = now;
            rt.sellBagStableSince = 0;
            rt.status = L"Macro bán xong • chờ FreeBagSpace xác nhận";
            return true;
        }
        SellMacroStep& step = a.profile.sellMacro[static_cast<std::size_t>(rt.sellMacroIndex)];
        if (rt.sellMacroNextTick != 0 && !Elapsed(now, rt.sellMacroNextTick, static_cast<DWORD>(step.delayMs))) return true;
        POINT point{}; std::wstring error;
        if (!ScaleClickPoint(a.game, step.point, point, error) || !RealInputClick(a, point, error)) {
            rt.status = L"Macro bán click FAIL • chờ thủ công";
            LogAccount(a, L"Macro bán dòng " + std::to_wstring(rt.sellMacroIndex + 1) + L" FAIL: " + error);
            rt.sellPhase = 10;
            return true;
        }
        ++rt.sellMacroRepeatDone;
        rt.sellMacroNextTick = now;
        rt.status = L"Bán • bước " + std::to_wstring(rt.sellMacroIndex + 1) + L"/" + std::to_wstring(a.profile.sellMacro.size()) +
                    L" • lặp " + std::to_wstring(rt.sellMacroRepeatDone) + L"/" + std::to_wstring(step.repeat);
        if (rt.sellMacroRepeatDone >= step.repeat) {
            ++rt.sellMacroIndex;
            rt.sellMacroRepeatDone = 0;
        }
        return true;
    }

    bool HandleAutoSell(Account& a, DWORD now) {
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        if (rt.sellPhase == 0) return false;

        if (rt.sellPhase == 1) {
            if ((s.validMask & ValidAutoFight) && !s.autoFight) {
                rt.sellPhase = 4; rt.sellPhaseTick = now;
                return true;
            }
            if (!a.profile.points[static_cast<std::size_t>(ClickSlot::StopAuto1)].valid ||
                !a.profile.points[static_cast<std::size_t>(ClickSlot::StopAuto2)].valid) {
                rt.status = L"Túi full • thiếu 2 tọa độ DỪNG AUTO";
                return true;
            }
            if (ClickSlotNow(a, ClickSlot::StopAuto1, L"auto sell: mở phần dừng auto", false)) {
                rt.sellPhase = 2; rt.sellPhaseTick = now;
                rt.status = L"Bán • đã click DỪNG AUTO 1";
            }
            return true;
        }
        if (rt.sellPhase == 2) {
            if (!Elapsed(now, rt.sellPhaseTick, 750)) return true;
            if (ClickSlotNow(a, ClickSlot::StopAuto2, L"auto sell: dừng đánh", false)) {
                rt.sellPhase = 3; rt.sellPhaseTick = now; ++rt.sellStopAttempts;
                rt.status = L"Bán • đã click DỪNG AUTO 2 • verify";
            }
            return true;
        }
        if (rt.sellPhase == 3) {
            if (!Elapsed(now, rt.sellPhaseTick, 1300)) return true;
            if ((s.validMask & ValidAutoFight) == 0 || !s.autoFight) {
                rt.sellPhase = 4; rt.sellPhaseTick = now; ResetRobustTravel(rt);
                rt.status = L"Đã dừng đánh • lên ngựa/đi NPC bán";
                return true;
            }
            if (rt.sellStopAttempts < 2) {
                rt.sellPhase = 1; rt.sellPhaseTick = now;
                return true;
            }
            rt.status = L"Không verify được DỪNG AUTO sau 2 lần • chờ thủ công";
            return true;
        }

        if (rt.sellPhase == 4) {
            const TargetProfile npcTarget = SellNpcTarget(a);
            if (!npcTarget.valid) {
                rt.status = L"NPC bán chưa có tọa độ • nhập X/Y hoặc LẤY VỊ TRÍ";
                return true;
            }
            bool arrived = false;
            (void)HandleRobustTravel(a, now, npcTarget, L"NPC bán", arrived);
            if (arrived) {
                rt.lastAction = Action::Hold;
                rt.sellPhase = 5; rt.sellPhaseTick = now;
                rt.status = L"Đã tới NPC • chuẩn bị ClickNPC";
            }
            return true;
        }

        if (rt.sellPhase == 5) {
            if (!Elapsed(now, rt.sellPhaseTick, 500)) return true;
            const SellNpcPreset& npc = kSellNpcs[static_cast<std::size_t>(a.profile.sellNpcPreset)];
            Response r{}; std::wstring error;
            if (!a.bridge.Call(Command::ClickNpc, npc.npcID, 0, 0, r, error, 1200)) {
                if (BridgeLooksUnresponsive(error)) EnterClientFreeze(a, L"Bridge timeout/busy khi ClickNPC", now);
                ++rt.sellOpenAttempts;
                LogAccount(a, L"ClickNPC " + std::to_wstring(npc.npcID) + L" FAIL: " + error);
                if (rt.sellOpenAttempts >= 2) { rt.sellPhase = 10; rt.status = L"Không mở được NPC • chờ thủ công"; }
                else rt.sellPhaseTick = now;
                return true;
            }
            ++rt.sellOpenAttempts;
            ++rt.sellMacroPass;
            rt.sellPhase = 6; rt.sellPhaseTick = now;
            rt.sellMacroIndex = 0; rt.sellMacroRepeatDone = 0; rt.sellMacroNextTick = 0;
            rt.status = L"Đã ClickNPC ID " + std::to_wstring(npc.npcID) + L" • chờ UI";
            return true;
        }

        if (rt.sellPhase == 6) {
            if (!Elapsed(now, rt.sellPhaseTick, 1200)) return true;
            return RunSellMacroClick(a, now);
        }

        if (rt.sellPhase == 7) {
            if ((s.validMask & ValidBagSpace) == 0) {
                rt.status = L"Không đọc được FreeBagSpace • không tự kết luận bán xong";
                return true;
            }
            if (s.freeBagSpace > 0) {
                if (rt.sellLastFreeBag != s.freeBagSpace) {
                    rt.sellLastFreeBag = s.freeBagSpace;
                    rt.sellBagStableSince = now;
                } else if (rt.sellBagStableSince == 0) {
                    rt.sellBagStableSince = now;
                } else if (Elapsed(now, rt.sellBagStableSince, 1500)) {
                    rt.sellPhase = 8; rt.sellPhaseTick = now;
                    rt.crossMapSeenAutoPath = false; rt.crossMapRouteArmed = false; rt.crossMapRouteMoved = false; rt.stallSinceTick = 0; rt.confirmAttempts = 0;
                    ResetRobustTravel(rt);
                    rt.status = L"Đã nhận diện bán xong • quay về bãi train";
                    LogAccount(a, L"BÁN XONG • FreeBagSpace=" + std::to_wstring(s.freeBagSpace) + L" • quay bãi train");
                }
                return true;
            }
            if (Elapsed(now, rt.sellPhaseTick, 3500)) {
                if (rt.sellMacroPass < 2) {
                    rt.sellPhase = 5; rt.sellPhaseTick = now; rt.sellOpenAttempts = 0;
                    rt.status = L"Túi vẫn full • mở NPC + chạy macro lại lần 2";
                } else {
                    rt.sellPhase = 10;
                    rt.status = L"Macro bán 2 lần nhưng túi vẫn full • chờ thủ công";
                }
            }
            return true;
        }

        if (rt.sellPhase == 8) {
            const TargetProfile& trainTarget = a.profile.target;
            bool arrived = false;
            (void)HandleRobustTravel(a, now, trainTarget, L"bãi train", arrived);
            if (arrived) {
                rt.sellPhase = 0; rt.sellTriggeredByFullBag = false;
                rt.fightPhase = 0; rt.fightAttempts = 0; rt.wasAtTarget = false;
                rt.trainPositionMonitorArmed = false; rt.lastTrainPositionCheckTick = 0;
                rt.lastAction = Action::Hold;
                rt.status = L"Đã về bãi • tiếp tục AUTO train";
                LogAccount(a, L"Đã về bãi train sau bán đồ • tiếp tục chu trình.");
                return false;
            }
            return true;
        }

        if (rt.sellPhase == 10) {
            if ((s.validMask & ValidBagSpace) && s.freeBagSpace > 0) {
                rt.sellPhase = 8; rt.sellPhaseTick = now; ResetRobustTravel(rt);
                rt.status = L"Túi đã có ô trống • quay về bãi train";
            }
            return true;
        }
        return true;
    }

    void TickAccount(Account& a) {
        if (!a.runtime.running) return;
        RuntimeState& rt = a.runtime;
        const Snapshot& s = a.snapshot;
        const DWORD now = GetTickCount();

        if (!s.mapReady || s.waitingChangeMap) {
            rt.candidateCount = 0;
            rt.qualifiedMap = 0;
            rt.stallSinceTick = 0;
            rt.fightPhase = 0;
            rt.status = L"Đang chuyển map • chặn action/click";
            return;
        }
        const std::uint32_t need = ValidMap | ValidPosition | ValidRiding | ValidAutoPath;
        if ((s.validMask & need) != need) {
            rt.status = L"State chưa đủ";
            return;
        }

        ObserveMovement(a, now);
        if (HandleDeath(a, now)) return;
        if (UpdateRotationEfficiency(a, now)) return;
        if (HandleRouteOwnershipReset(a, now)) return;
        if (HandleUnderworldAutoFightGuard(a, now)) return;

        // v1.5.10: restore the runtime-proven v0.8.7 portal flow. A MessageBox may
        // be consumed only while a cross-map StartPath owned by this tool is armed.
        // This runs before sell/recovery/train route FSMs so no second action can race
        // the internal UIButton callback in the same controller cycle.
        if (HandleCrossMapConfirm(a, now, a.profile.target)) return;

        if (rt.qualifiedMap != s.mapID) {
            if (rt.candidateMap == s.mapID) ++rt.candidateCount;
            else { rt.candidateMap = s.mapID; rt.candidateCount = 1; }
            if (rt.candidateCount < 2) {
                rt.status = L"Ổn định Map 1/2";
                return;
            }
            rt.qualifiedMap = s.mapID;
            rt.candidateCount = 0;
        }

        if (rt.sellPhase != 0) {
            if (HandleAutoSell(a, now)) return;
        } else if (a.profile.enableSell && (s.validMask & ValidBagSpace) && s.freeBagSpace <= 0) {
            const TargetProfile npcTarget = SellNpcTarget(a);
            if (!npcTarget.valid) {
                rt.status = L"TÚI FULL nhưng NPC bán chưa có tọa độ • nhập X/Y hoặc LẤY VỊ TRÍ";
                return;
            }
            std::wstring sellReason;
            if (!SellMacroConfigured(a, sellReason)) {
                rt.status = L"TÚI FULL nhưng " + sellReason;
                return;
            }
            BeginAutoSell(a, now);
            if (HandleAutoSell(a, now)) return;
        }

        if (rt.trainRecoveryPhase != 0) {
            if (HandleTrainRecovery(a, now)) return;
        }

        // Steady training mode: AutoFight is checked once per minute, but ONLY when
        // no death/sell/recovery/confirm/path action is active. A busy state does not
        // advance the timer; the check is deferred until the account becomes idle.
        if (rt.trainPositionMonitorArmed) {
            // The exclusion gate also applies while an AUTO→Đánh quái sequence is
            // already in progress. If another operation starts between the two clicks,
            // freeze the sequence and resume only after the account is idle again.
            if (AutoFightCheckBusy(a, now)) {
                rt.status = L"Train • đang có thao tác khác → hoãn check/bật AutoFight";
                return;
            }
            if (rt.fightPhase != 3) {
                if (HandleFightClicks(a, now)) return;
            }

            const bool autoCheckDue = rt.lastAutoFightCheckTick == 0 ||
                                      Elapsed(now, rt.lastAutoFightCheckTick, kAutoFightRecheckMs);
            if (autoCheckDue && !AutoFightCheckBusy(a, now)) {
                if ((s.validMask & ValidAutoFight) == 0) {
                    rt.status = L"CHECK AUTO 1 PHÚT: getter chưa sẵn sàng • không click";
                    return;
                }
                rt.lastAutoFightCheckTick = now;
                if (!s.autoFight) {
                    rt.fightPhase = 0;
                    rt.fightAttempts = 0;
                    LogAccount(a, L"CHECK AUTO 1 PHÚT: AutoFight OFF → chạy AUTO→Đánh quái.");
                    if (HandleFightClicks(a, now)) return;
                } else {
                    LogAccount(a, L"CHECK AUTO 1 PHÚT: AutoFight vẫn ON • không click.");
                }
            }

            if (!Elapsed(now, rt.lastTrainPositionCheckTick, kTrainPositionCheckMs)) {
                const DWORD elapsedMs = rt.lastTrainPositionCheckTick == 0 ? 0 : now - rt.lastTrainPositionCheckTick;
                const DWORD remainSec = elapsedMs >= kTrainPositionCheckMs ? 0 : (kTrainPositionCheckMs - elapsedMs + 999) / 1000;
                rt.status = L"Train ổn định • Auto check 1 phút • tọa check sau " + std::to_wstring(remainSec) + L"s";
                return;
            }

            rt.lastTrainPositionCheckTick = now;
            State monitor{};
            monitor.valid = true; monitor.mapReady = true; monitor.waitingMap = false;
            monitor.mapID = s.mapID; monitor.x = s.x; monitor.y = s.y;
            monitor.riding = s.riding != 0; monitor.autoPathing = s.autoPathing != 0;
            Target monitorTarget{a.profile.target.mapID, a.profile.target.x, a.profile.target.y, a.profile.tolerance};
            if (AtTarget(monitor, monitorTarget)) {
                rt.status = L"CHECK 3 PHÚT: đúng tọa • tiếp tục đánh";
                LogAccount(a, L"CHECK 3 PHÚT: tọa train vẫn đúng.");
                return;
            }
            BeginTrainRecovery(a, now);
            if (HandleTrainRecovery(a, now)) return;
        }

        State logic{};
        logic.valid = true;
        logic.mapReady = true;
        logic.waitingMap = false;
        logic.mapID = s.mapID;
        logic.x = s.x;
        logic.y = s.y;
        logic.riding = s.riding != 0;
        logic.autoPathing = s.autoPathing != 0;
        Target target{a.profile.target.mapID, a.profile.target.x, a.profile.target.y, a.profile.tolerance};
        const bool atTarget = AtTarget(logic, target);
        if (!atTarget) {
            rt.trainPositionMonitorArmed = false;
            rt.lastTrainPositionCheckTick = 0;
            rt.lastAutoFightCheckTick = 0;
            if (rt.wasAtTarget) {
                rt.fightPhase = 0;
                rt.fightAttempts = 0;
            }
            rt.wasAtTarget = false;
        }

        const Action action = Decide(logic, target);
        if (action == Action::Hold) {
            rt.lastAction = Action::Hold;
            if (!rt.wasAtTarget) {
                rt.fightPhase = 0;
                rt.fightAttempts = 0;
                LogAccount(a, L"Đã tới bãi và ổn định.");
            }
            rt.wasAtTarget = true;
            if (HandleFightClicks(a, now)) return;
            rt.status = L"Đúng bãi • giám sát tọa độ";
            return;
        }
        if (action == Action::Wait) {
            if (s.autoPathing) rt.status = L"Đang AutoPath tới bãi";
            return;
        }
        SendDecision(a, action, a.profile.target, L"bãi train");
    }

    void RefreshAccountIdentityIfNeeded(Account& a) {
        if (!a.snapshotValid) return;
        const std::wstring newSection = ProfileSection(a.snapshot, a.game.pid);
        if (a.profile.section == newSection) return;
        // PID fallback is only temporary. Once RoleID is proven, switch to the persistent role profile.
        AccountProfile persistent = LoadProfile(newSection);
        const bool persistentHasData = !persistent.selectedSpot.empty() || !persistent.rotationSpots.empty() || persistent.target.valid || persistent.enableSell ||
            !persistent.sellMacro.empty() ||
            std::any_of(persistent.points.begin(), persistent.points.end(), [](const ClickPoint& p){ return p.valid; });
        if (!persistentHasData) {
            persistent = a.profile;
            persistent.section = newSection;
        } else {
            // Merge data captured while identity was temporarily PID-based. The old
            // all-or-nothing switch could make newly captured clicks/macro appear lost.
            if (persistent.selectedSpot.empty() && !a.profile.selectedSpot.empty()) persistent.selectedSpot = a.profile.selectedSpot;
            if (persistent.rotationSpots.empty() && !a.profile.rotationSpots.empty()) persistent.rotationSpots = a.profile.rotationSpots;
            if (!persistent.target.valid && a.profile.target.valid) persistent.target = a.profile.target;
            for (std::size_t i = 0; i < persistent.points.size(); ++i) {
                if (!persistent.points[i].valid && a.profile.points[i].valid) persistent.points[i] = a.profile.points[i];
            }
            if (persistent.sellMacro.empty() && !a.profile.sellMacro.empty()) persistent.sellMacro = a.profile.sellMacro;
        }
        persistent.section = newSection;
        SaveProfile(persistent);
        a.profile = persistent;
        MigrateLegacySpot(a.profile);
        a.displayName = DisplayName(a.snapshot, a.game.pid);
    }

    void UpdateSelectedLive() {
        Account* a = SelectedAccount();
        if (!a) return;
        if (!a->snapshotValid) {
            SetText(live_, L"STATE: chưa đọc được snapshot");
            return;
        }
        const Snapshot& s = a->snapshot;
        std::wstring text = L"STATE " + AccountTag(*a) + L" • M" + std::to_wstring(s.mapID) + L" • " +
                            std::to_wstring(s.x) + L"," + std::to_wstring(s.y) +
                            L" • Ngựa " + (s.riding ? L"ON" : L"OFF") +
                            L" • Path " + (s.autoPathing ? L"ON" : L"OFF");
        if (s.validMask & ValidLifeState) text += L" • " + std::wstring(s.dead ? L"CHẾT" : L"SỐNG");
        if (s.validMask & ValidAutoFight) text += L" • Đánh quái " + std::wstring(s.autoFight ? L"ON" : L"OFF");
        if (s.validMask & ValidBagSpace) text += L" • Túi trống " + std::to_wstring(s.freeBagSpace);
        text += L" • XN map nội bộ";
        if (globalPaused_) text += L" • F4 PAUSE";
        if (a->runtime.clientFreezeActive) text += L" • FREEZE ACTION";
        if (!s.mapReady || s.waitingChangeMap) text = L"STATE " + AccountTag(*a) + L" • ĐANG CHUYỂN MAP • FREEZE ACTION";
        SetText(live_, text);
    }

    static ULONGLONG LocalCalendarTicks(const SYSTEMTIME& st) {
        FILETIME ft{};
        if (!SystemTimeToFileTime(&st, &ft)) return 0;
        ULARGE_INTEGER u{};
        u.LowPart = ft.dwLowDateTime;
        u.HighPart = ft.dwHighDateTime;
        return u.QuadPart;
    }

    void UpdateShutdownStatusText() {
        if (!shutdownStatus_) return;
        if (!shutdownEnabled_) {
            SetWindowTextW(shutdownStatus_, L"OFF");
            return;
        }
        wchar_t text[64]{};
        wsprintfW(text, L"%02u/%02u %02u:%02u", shutdownTargetLocal_.wDay, shutdownTargetLocal_.wMonth,
                  shutdownTargetLocal_.wHour, shutdownTargetLocal_.wMinute);
        SetWindowTextW(shutdownStatus_, text);
    }

    void ResolveNextShutdownTarget() {
        if (!shutdownEnabled_) { shutdownTargetLocal_ = SYSTEMTIME{}; UpdateShutdownStatusText(); return; }
        SYSTEMTIME now{};
        GetLocalTime(&now);
        SYSTEMTIME target = now;
        target.wHour = static_cast<WORD>(shutdownHour_);
        target.wMinute = static_cast<WORD>(shutdownMinute_);
        target.wSecond = 0;
        target.wMilliseconds = 0;
        ULONGLONG nowTicks = LocalCalendarTicks(now);
        ULONGLONG targetTicks = LocalCalendarTicks(target);
        if (targetTicks <= nowTicks) {
            targetTicks += 24ULL * 60ULL * 60ULL * 10000000ULL;
            FILETIME ft{};
            ULARGE_INTEGER u{}; u.QuadPart = targetTicks;
            ft.dwLowDateTime = u.LowPart; ft.dwHighDateTime = u.HighPart;
            (void)FileTimeToSystemTime(&ft, &target);
        }
        shutdownTargetLocal_ = target;
        UpdateShutdownStatusText();
    }

    void LoadShutdownSettings() {
        shutdownEnabled_ = ReadIniInt(L"Global", L"ShutdownEnabled", 0) != 0;
        shutdownHour_ = ReadIniInt(L"Global", L"ShutdownHour", 0);
        shutdownMinute_ = ReadIniInt(L"Global", L"ShutdownMinute", 0);
        if (shutdownHour_ < 0 || shutdownHour_ > 23) shutdownHour_ = 0;
        if (shutdownMinute_ < 0 || shutdownMinute_ > 59) shutdownMinute_ = 0;
        ResolveNextShutdownTarget();
    }

    void ApplyShutdownScheduleFromUi() {
        if (!shutdownEnable_ || !shutdownHourEdit_ || !shutdownMinuteEdit_) return;
        wchar_t h[8]{}, m[8]{};
        GetWindowTextW(shutdownHourEdit_, h, _countof(h));
        GetWindowTextW(shutdownMinuteEdit_, m, _countof(m));
        int hour = _wtoi(h);
        int minute = _wtoi(m);
        if (hour < 0) hour = 0;
        if (hour > 23) hour = 23;
        if (minute < 0) minute = 0;
        if (minute > 59) minute = 59;
        shutdownHour_ = hour;
        shutdownMinute_ = minute;
        shutdownEnabled_ = SendMessageW(shutdownEnable_, BM_GETCHECK, 0, 0) == BST_CHECKED;
        WriteIniInt(L"Global", L"ShutdownEnabled", shutdownEnabled_ ? 1 : 0);
        WriteIniInt(L"Global", L"ShutdownHour", shutdownHour_);
        WriteIniInt(L"Global", L"ShutdownMinute", shutdownMinute_);
        wchar_t hh[8]{}, mm[8]{}; wsprintfW(hh, L"%02d", hour); wsprintfW(mm, L"%02d", minute);
        SetWindowTextW(shutdownHourEdit_, hh); SetWindowTextW(shutdownMinuteEdit_, mm);
        ResolveNextShutdownTarget();
        if (shutdownEnabled_) Log(L"Đã hẹn TẮT TOOL theo giờ hệ thống: " + std::to_wstring(hour) + L":" + (minute < 10 ? L"0" : L"") + std::to_wstring(minute));
        else Log(L"Đã tắt hẹn giờ đóng tool.");
    }

    bool CheckScheduledShutdown() {
        if (!shutdownEnabled_) return false;
        SYSTEMTIME now{};
        GetLocalTime(&now);
        if (LocalCalendarTicks(now) < LocalCalendarTicks(shutdownTargetLocal_)) return false;
        Log(L"ĐẾN GIỜ HẸN → dừng route và đóng TOOL (không tắt Windows).");
        for (auto& item : accounts_) {
            Account& a = *item;
            if (a.runtime.running && a.bridge.Attached() && !a.runtime.clientFreezeActive) {
                Response r{}; std::wstring ignored;
                (void)a.bridge.Call(Command::StopPath, 0, 0, 0, r, ignored, 700);
            }
        }
        shutdownEnabled_ = false;
        WriteIniInt(L"Global", L"ShutdownEnabled", 0);
        DestroyWindow(hwnd_);
        return true;
    }

    void Tick() {
        if (CheckScheduledShutdown()) return;
        for (std::size_t i = 0; i < accounts_.size(); ++i) {
            Account& a = *accounts_[i];
            const bool selected = static_cast<int>(i) == SelectedIndex();
            if (!a.runtime.running && !selected) {
                UpdateAccountRow(static_cast<int>(i), a);
                continue;
            }
            std::wstring error;
            const DWORD now = GetTickCount();
            if (!ReadSnapshot(a, error, a.runtime.running ? 700 : 900)) {
                if (a.runtime.running) MarkReadStateFailure(a, error, now);
                else a.runtime.status = L"Mất state/bridge";
                UpdateAccountRow(static_cast<int>(i), a);
                continue;
            }
            RefreshAccountIdentityIfNeeded(a);
            if (a.runtime.running) {
                if (HoldUntilClientStable(a, now)) {
                    UpdateAccountRow(static_cast<int>(i), a);
                    continue;
                }
                if (!globalPaused_) TickAccount(a);
                else a.runtime.status = L"TẠM DỪNG F4";
            }
            UpdateAccountRow(static_cast<int>(i), a);
        }
        UpdateSelectedLive();
    }

    void OnListNotification(const NMHDR* hdr) {
        if (!hdr) return;
        if (hdr->hwndFrom == clientList_ && hdr->code == LVN_ITEMCHANGED) {
            const auto* n = reinterpret_cast<const NMLISTVIEW*>(hdr);
            if ((n->uChanged & LVIF_STATE) != 0 && (n->uNewState & LVIS_SELECTED) != 0) {
                PersistSelectedEditorSafeBeforeSwitch(n->iItem);
                LoadSelectedProfileToUi();
            }
            return;
        }
        if (hdr->hwndFrom == sellMacroList_ && hdr->code == LVN_ITEMCHANGED) {
            const auto* n = reinterpret_cast<const NMLISTVIEW*>(hdr);
            if ((n->uChanged & LVIF_STATE) != 0 && (n->uNewState & LVIS_SELECTED) != 0) LoadSelectedMacroEditor();
            return;
        }
        if (hdr->hwndFrom == rotationList_ && hdr->code == LVN_ITEMCHANGED) {
            if (rotationUiLoading_) return;
            const auto* n = reinterpret_cast<const NMLISTVIEW*>(hdr);
            if ((n->uChanged & LVIF_STATE) != 0 && ((n->uOldState ^ n->uNewState) & LVIS_STATEIMAGEMASK) != 0) {
                Account* a = SelectedAccount();
                if (a) {
                    const std::wstring oldSpot = a->profile.selectedSpot;
                    PersistRotationListFromUi(*a);
                    SaveProfile(a->profile);
                    RefreshRotationList();
                    if (_wcsicmp(oldSpot.c_str(), a->profile.selectedSpot.c_str()) != 0) {
                        const DWORD now = GetTickCount();
                        ResetRotationWindow(*a, now);
                        if (a->runtime.running) BeginTrainRecovery(*a, now);
                        LogAccount(*a, L"Đổi pool → bãi hiện tại chuyển sang " + a->profile.selectedSpot);
                    }
                    const int row = SelectedIndex();
                    if (row >= 0) UpdateAccountRow(row, *a);
                }
            }
            return;
        }
    }

    void ToggleGlobalPause() {
        globalPaused_ = !globalPaused_;
        if (globalPaused_) {
            for (auto& item : accounts_) {
                Account& a = *item;
                if (!a.runtime.running) continue;
                if (a.bridge.Attached() && !a.runtime.clientFreezeActive) {
                    Response r{}; std::wstring ignored;
                    (void)a.bridge.Call(Command::StopPath, 0, 0, 0, r, ignored, 700);
                }
                a.runtime.status = L"TẠM DỪNG F4";
            }
            Log(L"F4 → TẠM DỪNG toàn bộ acc đang RUN; StopPath đã gửi, không tự đổi combat.");
        } else {
            for (auto& item : accounts_) if (item->runtime.running) item->runtime.status = L"Tiếp tục sau F4";
            Log(L"F4 → TIẾP TỤC toàn bộ acc đang RUN.");
        }
    }

    void PersistSelectedEditorSafeBeforeSwitch(int newIndex) {
        // LVN_ITEMCHANGED arrives after selection state changes, so we cannot reliably know the old row here.
        // All meaningful editor mutations are persisted immediately on their own events/capture/save.
        (void)newIndex;
    }

    LRESULT Handle(UINT msg, WPARAM wp, LPARAM lp) {
        switch (msg) {
            case WM_CREATE:
                BuildUi();
                return 0;
            case WM_NOTIFY:
                OnListNotification(reinterpret_cast<const NMHDR*>(lp));
                return 0;
            case WM_COMMAND:
                switch (LOWORD(wp)) {
                    case IDC_SCAN:
                        ScanClients();
                        break;
                    case IDC_START_CHECKED:
                        StartChecked();
                        break;
                    case IDC_STOP_CHECKED:
                        StopChecked();
                        break;
                    case IDC_SHUTDOWN_APPLY:
                        ApplyShutdownScheduleFromUi();
                        break;
                    case IDC_SHUTDOWN_ENABLE:
                        if (HIWORD(wp) == BN_CLICKED) ApplyShutdownScheduleFromUi();
                        break;
                    case IDC_ROTATE_DEATH_LIMIT:
                    case IDC_ROTATE_DEATH_WINDOW:
                    case IDC_ROTATE_NO_BAG:
                        if (HIWORD(wp) == EN_KILLFOCUS) PersistSelectedEditor();
                        break;
                    case IDC_SAVE_TARGET:
                        SaveTargetForSelected();
                        break;
                    case IDC_DELETE_SPOT:
                        DeleteSelectedSharedSpot();
                        break;
                    case IDC_SPOT_COMBO:
                        if (HIWORD(wp) == CBN_SELCHANGE) SelectSharedSpotForAccount();
                        break;
                    case IDC_CAPTURE_AUTO:
                        BeginCapture(ClickSlot::AutoMenu);
                        break;
                    case IDC_CAPTURE_ATTACK:
                        BeginCapture(ClickSlot::Attack);
                        break;
                    case IDC_CAPTURE_STOP_AUTO_1:
                        BeginCapture(ClickSlot::StopAuto1);
                        break;
                    case IDC_CAPTURE_STOP_AUTO_2:
                        BeginCapture(ClickSlot::StopAuto2);
                        break;
                    case IDC_TEST_AUTO:
                        TestClick(ClickSlot::AutoMenu);
                        break;
                    case IDC_TEST_ATTACK:
                        TestClick(ClickSlot::Attack);
                        break;
                    case IDC_TEST_STOP_AUTO_1:
                        TestClick(ClickSlot::StopAuto1);
                        break;
                    case IDC_TEST_STOP_AUTO_2:
                        TestClick(ClickSlot::StopAuto2);
                        break;
                    case IDC_SELL_ADD:
                        AddSellMacroRow();
                        break;
                    case IDC_SELL_DELETE:
                        DeleteSellMacroRow();
                        break;
                    case IDC_SELL_SAVE:
                        SaveSellMacroRow();
                        break;
                    case IDC_SELL_CAPTURE:
                        BeginMacroCapture();
                        break;
                    case IDC_SELL_TEST:
                        TestSellMacroRow();
                        break;
                    case IDC_ENABLE_REVIVE:
                    case IDC_ENABLE_FIGHT:
                    case IDC_ENABLE_SELL:
                        if (HIWORD(wp) == BN_CLICKED) PersistSelectedEditor();
                        break;
                    case IDC_SELL_NPC:
                        if (HIWORD(wp) == CBN_SELCHANGE) OnSellNpcSelectionChanged();
                        break;
                    case IDC_SELL_NPC_CAPTURE:
                        if (HIWORD(wp) == BN_CLICKED) CaptureSellNpcPosition();
                        break;
                    case IDC_SELL_NPC_X:
                    case IDC_SELL_NPC_Y:
                    case IDC_TOLERANCE:
                        if (HIWORD(wp) == EN_KILLFOCUS) PersistSelectedEditor();
                        break;
                }
                return 0;
            case WM_HOTKEY:
                if (static_cast<int>(wp) == kCaptureHotkeyId) {
                    CaptureHotkeyPoint();
                    return 0;
                }
                if (static_cast<int>(wp) == kPauseHotkeyId) {
                    ToggleGlobalPause();
                    return 0;
                }
                break;
            case WM_TIMER:
                if (wp == kTimer) Tick();
                return 0;
            case WM_DESTROY:
                // Auto-save every persistent input before exit. Captures already save
                // immediately; this final pass also commits the currently edited macro row.
                SaveSellMacroRow();
                PersistSelectedEditor();
                SaveSharedSellNpcPositions(sellNpcPositions_);
                for (auto& a : accounts_) SaveProfile(a->profile);
                FlushIni();
                UnregisterHotKey(hwnd_, kCaptureHotkeyId);
                UnregisterHotKey(hwnd_, kPauseHotkeyId);
                for (auto& a : accounts_) a->bridge.Close();
                PostQuitMessage(0);
                return 0;
        }
        return DefWindowProcW(hwnd_, msg, wp, lp);
    }

    HWND hwnd_ = nullptr;
    HWND clientList_ = nullptr;
    HWND selected_ = nullptr;
    HWND live_ = nullptr;
    HWND shutdownEnable_ = nullptr;
    HWND shutdownHourEdit_ = nullptr;
    HWND shutdownMinuteEdit_ = nullptr;
    HWND shutdownStatus_ = nullptr;
    HWND targetName_ = nullptr;
    HWND spotCombo_ = nullptr;
    HWND targetText_ = nullptr;
    HWND tolerance_ = nullptr;
    HWND enableRevive_ = nullptr;
    HWND rotationList_ = nullptr;
    HWND rotateDeathLimit_ = nullptr;
    HWND rotateDeathWindow_ = nullptr;
    HWND rotateNoFullBag_ = nullptr;
    HWND enableFight_ = nullptr;
    HWND enableSell_ = nullptr;
    HWND sellNpcCombo_ = nullptr;
    HWND sellNpcX_ = nullptr;
    HWND sellNpcY_ = nullptr;
    HWND sellNpcPosText_ = nullptr;
    HWND sellMacroList_ = nullptr;
    HWND sellDesc_ = nullptr;
    HWND sellDelay_ = nullptr;
    HWND sellRepeat_ = nullptr;
    std::array<HWND, 4> pointLabels_{};
    HWND log_ = nullptr;

    std::vector<std::unique_ptr<Account>> accounts_;
    std::vector<TargetProfile> spots_;
    std::array<SellNpcPosition, kSellNpcs.size()> sellNpcPositions_ = LoadSharedSellNpcPositions();
    ClickSlot captureSlot_ = ClickSlot::None;
    int captureMacroIndex_ = -1;
    DWORD capturePid_ = 0;
    bool globalPaused_ = false;
    bool rotationUiLoading_ = false;
    DWORD globalLastRealInputTick_ = 0;
    bool shutdownEnabled_ = false;
    int shutdownHour_ = 0;
    int shutdownMinute_ = 0;
    SYSTEMTIME shutdownTargetLocal_{};
};

} // namespace

void EnableDpiAwareness() {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    using SetContextFn = BOOL (WINAPI*)(HANDLE);
    SetContextFn setContext = nullptr;
    if (user32) ResolveProc(user32, "SetProcessDpiAwarenessContext", setContext);
    if (setContext) {
        // DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 == (HANDLE)-4. Dynamic lookup keeps old SDKs buildable.
        (void)setContext(reinterpret_cast<HANDLE>(static_cast<INT_PTR>(-4)));
    } else {
        (void)SetProcessDPIAware();
    }
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    // Prevent DPI virtualization from corrupting cursor->client coordinate capture on scaled displays.
    EnableDpiAwareness();
    App app;
    if (!app.Create(instance)) return 2;
    app.Show(show);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
