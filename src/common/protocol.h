#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace tlcore {

constexpr std::uint32_t kMagic = 0x544C4E43u; // TLNC
constexpr std::uint32_t kProtocolVersion = 0x0001000Bu;
constexpr UINT kWakeMessage = WM_APP + 0x4A1;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongNewCore_";

enum class BridgeCommand : std::uint32_t {
    None = 0,
    ValidateNative = 1,
    InspectFgMainThread = 2,
    InspectUnityDispatcher = 3,
    ProveUnityMainThread = 4,
    ReadGameSnapshot = 5,
};

enum FoundationValid : std::uint32_t {
    ValidHookThread          = 1u << 0,
    ValidIl2CppExports       = 1u << 1,
    ValidFgMainThreadType    = 1u << 2,
    ValidUnityDispatcher     = 1u << 3,
    ValidUnityMainThread     = 1u << 4,
    ValidUnitySyncContext    = 1u << 5,
};

struct FoundationSnapshot {
    std::uint32_t validMask = 0;
    std::uint32_t hookThreadId = 0;
    std::uint32_t windowThreadId = 0;
    std::uint32_t resolvedExports = 0;
    std::uint32_t requiredExports = 0;
    std::uint32_t fgMainThreadMethodCount = 0;
    std::uint32_t fgMainThreadFieldCount = 0;
    std::uint32_t unityDispatcherMethodCount = 0;
    std::uint32_t unityDispatcherFieldCount = 0;
    std::int32_t currentManagedThreadId = 0;
    std::int32_t unityMainManagedThreadId = 0;
};

enum GameSnapshotValid : std::uint32_t {
    ValidRoleIdentity       = 1u << 0,
    ValidMapId              = 1u << 1,
    ValidPosition           = 1u << 2,
    ValidVitals             = 1u << 3,
    ValidLifeState          = 1u << 4,
    ValidRideState          = 1u << 5,
    ValidAutoFightState     = 1u << 6,
    ValidBagSpace           = 1u << 7,
    ValidMapReadyState      = 1u << 8,
    ValidMapTransitionState = 1u << 9,
    ValidMovingState        = 1u << 10,
    ValidCharacterName      = 1u << 11,
    ValidAutoPathState       = 1u << 12,
};


enum AutoPathProbeMask : std::uint32_t {
    AutoPathClassResolved          = 1u << 0,
    AutoPathInstanceGetterResolved = 1u << 1,
    AutoPathInstanceResolved       = 1u << 2,
    AutoPathStateGetterResolved    = 1u << 3,
    AutoPathValueRead              = 1u << 4,
};

constexpr std::uint32_t kAutoPathProbeCompleteMask =
    AutoPathClassResolved | AutoPathInstanceGetterResolved |
    AutoPathInstanceResolved | AutoPathStateGetterResolved | AutoPathValueRead;

constexpr std::uint32_t kRequiredGameCoreMask =
    ValidRoleIdentity | ValidMapId | ValidVitals | ValidLifeState |
    ValidRideState | ValidAutoFightState | ValidBagSpace | ValidMapReadyState |
    ValidMapTransitionState;

// v1.0.11 observer parity gate: these are the read-only states already proven useful in
// the v0.9.0 donor state machine. NewCore still resolves them by metadata rather than
// copying donor RVAs/offsets. No gameplay action is enabled by this mask.
constexpr std::uint32_t kRequiredObserverStableMask =
    kRequiredGameCoreMask | ValidPosition | ValidMovingState | ValidAutoPathState;

struct GameSnapshot {
    std::uint32_t validMask = 0;
    std::uint32_t sequence = 0;
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t hp = 0;
    std::int32_t maxHP = 0;
    std::int32_t freeBagSpace = -1;
    std::int32_t dead = 0;
    std::int32_t riding = 0;
    std::int32_t autoFight = 0;
    std::int32_t moving = 0;
    std::int32_t mapReady = 0;
    std::int32_t waitingChangeMap = 0;
    std::int32_t autoPathing = 0;
    std::uint32_t autoPathProbeMask = 0;
    wchar_t characterName[64]{};
};

struct BridgeRequest {
    std::uint32_t command = 0;
    std::int32_t intArg0 = 0;
    std::int32_t intArg1 = 0;
    std::uint64_t u64Arg0 = 0;
};

struct BridgeResponse {
    std::int32_t ok = 0;
    std::int32_t errorCode = 0;
    FoundationSnapshot snapshot{};
    GameSnapshot gameSnapshot{};
    wchar_t detail[1024]{};
};

struct SharedBlock {
    std::uint32_t magic = kMagic;
    std::uint32_t protocolVersion = kProtocolVersion;
    std::uint32_t targetPid = 0;
    std::uint32_t targetWindowThreadId = 0;
    volatile LONG requestSeq = 0;
    volatile LONG completedSeq = 0;
    volatile LONG bridgeLoaded = 0;
    volatile LONG bridgeBusy = 0;
    BridgeRequest request{};
    BridgeResponse response{};
};

inline void MappingName(DWORD pid, wchar_t* output, std::size_t count) {
    if (!output || count == 0) return;
    wsprintfW(output, L"%s%lu", kMappingPrefix, static_cast<unsigned long>(pid));
}

} // namespace tlcore
