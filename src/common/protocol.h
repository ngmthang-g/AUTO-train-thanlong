#pragma once
#include <windows.h>
#include <cstdint>
#include <cstddef>

namespace tlcore {

constexpr std::uint32_t kMagic = 0x544C4E43u; // TLNC
constexpr std::uint32_t kProtocolVersion = 0x00010003u;
constexpr UINT kWakeMessage = WM_APP + 0x4A1;
constexpr wchar_t kMappingPrefix[] = L"Local\\ThanLongNewCore_";

enum class BridgeCommand : std::uint32_t {
    None = 0,
    ValidateNative = 1,
    InspectFgMainThread = 2,
    InspectUnityDispatcher = 3,
};

enum FoundationValid : std::uint32_t {
    ValidHookThread       = 1u << 0,
    ValidIl2CppExports    = 1u << 1,
    ValidFgMainThreadType = 1u << 2,
    ValidUnityDispatcher  = 1u << 3,
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
