#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace cleanroute {

constexpr std::uint32_t kMagic = 0x4352544Cu; // CRTL
// v1.5.10 restores v0.8.7-style internal UIButton Confirm/Revive actions.
constexpr std::uint32_t kProtocolVersion = 0x00010510u;
constexpr UINT kWakeMessage = WM_APP + 0x531;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongCleanRoute_";

enum class Command : std::uint32_t {
    None = 0,
    ReadState = 1,
    ToggleRide = 2,
    StartPath = 3,
    StopPath = 4,
    ClickNpc = 6,
    ClickInternalConfirm = 7,
    ClickInternalRevive = 8,
};

enum SnapshotValid : std::uint32_t {
    ValidMapTransition = 1u << 0,
    ValidIdentity      = 1u << 1,
    ValidMap           = 1u << 2,
    ValidPosition      = 1u << 3,
    ValidRiding        = 1u << 4,
    ValidMoving        = 1u << 5,
    ValidAutoPath      = 1u << 6,
    ValidVitals        = 1u << 7,
    ValidLifeState     = 1u << 8,
    ValidAutoFight     = 1u << 9,
    ValidBagSpace      = 1u << 10,
    ValidConfirmUi     = 1u << 11,
};

struct Snapshot {
    std::uint32_t validMask = 0;
    std::uint32_t sequence = 0;
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t riding = 0;
    std::int32_t moving = 0;
    std::int32_t autoPathing = 0;
    std::int32_t mapReady = 0;
    std::int32_t waitingChangeMap = 0;
    std::int32_t hp = 0;
    std::int32_t maxHP = 0;
    std::int32_t dead = 0;
    std::int32_t autoFight = 0;
    std::int32_t freeBagSpace = -1;
    std::int32_t confirmUiVisible = 0;
    wchar_t characterName[64]{};
};

struct Request {
    std::uint32_t command = 0;
    std::int32_t arg0 = 0;
    std::int32_t arg1 = 0;
    std::int32_t arg2 = 0;
};

struct Response {
    std::int32_t ok = 0;
    std::int32_t errorCode = 0;
    std::uint32_t callbackThreadId = 0;
    Snapshot snapshot{};
    wchar_t detail[512]{};
};

struct SharedBlock {
    std::uint32_t magic = kMagic;
    std::uint32_t protocolVersion = kProtocolVersion;
    std::uint32_t targetPid = 0;
    std::uint32_t targetWindowThreadId = 0;
    std::uint64_t targetHwnd = 0;
    volatile LONG requestSeq = 0;
    volatile LONG completedSeq = 0;
    volatile LONG bridgeLoaded = 0;
    volatile LONG bridgeBusy = 0;
    Request request{};
    Response response{};
};

inline void MappingName(DWORD pid, wchar_t* output, std::size_t count) {
    if (!output || count == 0) return;
    wsprintfW(output, L"%s%lu", kMappingPrefix, static_cast<unsigned long>(pid));
}

} // namespace cleanroute
