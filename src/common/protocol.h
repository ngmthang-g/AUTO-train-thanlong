#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace tlcore {

constexpr std::uint32_t kMagic = 0x544C4E43u; // TLNC
constexpr std::uint32_t kProtocolVersion = 0x00010000u;
constexpr UINT kWakeMessage = WM_APP + 0x4A1;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongNewCore_";

enum class BridgeCommand : std::uint32_t {
    None = 0,
    Probe = 1,
    StartAutoFight = 2,
    StopAutoFight = 3,
    ClickNpc = 4,
};

enum SnapshotValid : std::uint32_t {
    ValidMainThread = 1u << 0,
    ValidMapReady   = 1u << 1,
    ValidAutoFight  = 1u << 2,
    ValidDead       = 1u << 3,
    ValidRiding     = 1u << 4,
    ValidMoving     = 1u << 5,
    ValidBag        = 1u << 6,
    ValidPosition   = 1u << 7,
    ValidRole       = 1u << 8,
    ValidMapId      = 1u << 9,
};

struct GameSnapshot {
    std::uint32_t validMask = 0;
    std::uint32_t mainThreadProof = 0; // 1=UnitySynchronizationContext (strict proof)
    std::uint32_t mainThreadId = 0;
    std::uint32_t windowThreadId = 0;
    std::int32_t mapId = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t roleId = 0;
    std::int32_t freeBagSpace = -1;
    std::uint8_t mapReady = 0;
    std::uint8_t autoFight = 0;
    std::uint8_t dead = 0;
    std::uint8_t riding = 0;
    std::uint8_t moving = 0;
    std::uint8_t reserved8[3]{};
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
    GameSnapshot snapshot{};
    wchar_t detail[320]{};
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
