#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace tlcore {

constexpr std::uint32_t kMagic = 0x544C4E43u; // TLNC
constexpr std::uint32_t kProtocolVersion = 0x00010200u;
constexpr UINT kWakeMessage = WM_APP + 0x4A1;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongNewCore_";

enum class BridgeCommand : std::uint32_t {
    None = 0,
    ValidateNative = 1,
    InspectFgMainThread = 2,
    InspectUnityDispatcher = 3,
    ProveUnityMainThread = 4,
    ReadGameSnapshot = 5,
    ProveHookActionEnvelope = 6,
    InvokeReviveButton = 7,
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

enum ActionMetadataCapability : std::uint32_t {
    CapUiObjectType                   = 1u << 0,
    CapUiObjectInstances              = 1u << 1,
    CapUiObjectActive                 = 1u << 2,
    CapUiObjectChildren               = 1u << 3,
    CapUiButtonType                   = 1u << 4,
    CapUiButtonInteractable           = 1u << 5,
    CapUiButtonText                   = 1u << 6,
    CapUiButtonHandleClick            = 1u << 7,
    CapUiToggleType                   = 1u << 8,
    CapUiToggleSelected               = 1u << 9,
    CapUiToggleSetSelected            = 1u << 10,
    CapUiToggleHandleSelect           = 1u << 11,
    CapGameApiType                    = 1u << 12,
    CapClickNpc                       = 1u << 13,
    CapGetNearestNpc                  = 1u << 14,
    CapBagFreeSpace                   = 1u << 15,
    CapGetItemsAtSite                 = 1u << 16,
    CapGetItemData                    = 1u << 17,
    CapGetItemType                    = 1u << 18,
    CapGetEquipType                   = 1u << 19,
    CapIsItemSellable                 = 1u << 20,
    CapMonoExecutorType               = 1u << 21,
    CapMonoExecutorInstanceGetter     = 1u << 22,
    CapMonoExecutorExecuteScript      = 1u << 23,
    CapMainThreadType                 = 1u << 24,
    CapMainThreadInstanceGetter       = 1u << 25,
    CapMainThreadExecuteAction        = 1u << 26,
    CapUnityDispatcherType            = 1u << 27,
    CapUnityDispatcherInstanceGetter  = 1u << 28,
    CapUnityDispatcherEnqueueAction   = 1u << 29,
    CapUnityDispatcherDispatchAction  = 1u << 30,
};

constexpr std::uint32_t kReviveMetadataSupportMask =
    CapUiObjectType | CapUiObjectInstances | CapUiObjectActive | CapUiObjectChildren |
    CapUiButtonType | CapUiButtonInteractable | CapUiButtonText | CapUiButtonHandleClick;

constexpr std::uint32_t kToggleMetadataSupportMask =
    CapUiToggleType | CapUiToggleSelected | CapUiToggleSetSelected | CapUiToggleHandleSelect;

constexpr std::uint32_t kNpcMetadataSupportMask =
    CapGameApiType | CapClickNpc | CapGetNearestNpc;

constexpr std::uint32_t kInventoryMetadataSupportMask =
    CapGameApiType | CapBagFreeSpace | CapGetItemsAtSite | CapGetItemData;

constexpr std::uint32_t kSellClassificationMetadataSupportMask =
    kInventoryMetadataSupportMask | CapGetItemType | CapGetEquipType | CapIsItemSellable;

constexpr std::uint32_t kLuaExecutorMetadataSupportMask =
    CapMonoExecutorType | CapMonoExecutorInstanceGetter | CapMonoExecutorExecuteScript;

constexpr std::uint32_t kMainThreadDispatcherMetadataSupportMask =
    CapMainThreadType | CapMainThreadInstanceGetter | CapMainThreadExecuteAction;

constexpr std::uint32_t kUnityDispatcherMetadataSupportMask =
    CapUnityDispatcherType | CapUnityDispatcherInstanceGetter |
    CapUnityDispatcherEnqueueAction | CapUnityDispatcherDispatchAction;

struct ActionCapabilitySnapshot {
    std::uint32_t probeVersion = 0;
    std::uint32_t metadataMask = 0;
};

struct InfrastructureProofSnapshot {
    std::uint64_t token = 0;
    std::uint32_t sequence = 0;
    std::uint32_t callbackThreadId = 0;
    std::int32_t currentManagedThreadId = 0;
    std::int32_t unityMainManagedThreadId = 0;
    std::int32_t completed = 0;
};

struct ReviveActionSnapshot {
    std::uint64_t token = 0;
    std::uint32_t callbackThreadId = 0;
    std::int32_t currentManagedThreadId = 0;
    std::int32_t unityMainManagedThreadId = 0;
    std::int32_t preRoleID = 0;
    std::int32_t preMapID = 0;
    std::int32_t preDead = 0;
    std::int32_t buttonScore = 0;
    std::int32_t invoked = 0;
    wchar_t buttonLabel[96]{};
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
    ValidAutoPathState      = 1u << 12,
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
    ActionCapabilitySnapshot actionCapabilities{};
    InfrastructureProofSnapshot infrastructureProof{};
    ReviveActionSnapshot reviveAction{};
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
