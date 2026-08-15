    AppendText(detail, cap, L" dead="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.dead));
    AppendText(detail, cap, L" riding="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.riding));
    AppendText(detail, cap, L" moving=");
    if (game.validMask & ValidMovingState) AppendUInt(detail, cap, static_cast<std::uint32_t>(game.moving));
    else AppendText(detail, cap, L"?");
    AppendText(detail, cap, L" auto="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.autoFight));
    AppendText(detail, cap, L" autoPath=");
    if (game.validMask & ValidAutoPathState) AppendUInt(detail, cap, static_cast<std::uint32_t>(game.autoPathing));
    else AppendText(detail, cap, L"?");
    AppendText(detail, cap, L" autoPathProbe=");
    AppendUInt(detail, cap, game.autoPathProbeMask);
    AppendText(detail, cap, L" mapReady="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.mapReady));
    AppendText(detail, cap, L" waitingMap="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.waitingChangeMap));
    AppendText(detail, cap, L". READ-ONLY; action LOCKED.");
    return true;
}

#include "action_capability_probe.inc"
#include "harmless_action_envelope.inc"

void SetDetail(BridgeResponse& response, const wchar_t* text) {
    CopyText(response.detail, _countof(response.detail), text ? text : L"");
}

bool EnsureShared() {
    if (g_shared) return true;
    wchar_t name[96]{};
    MappingName(GetCurrentProcessId(), name, _countof(name));
    g_mapping = OpenFileMappingW(FILE_MAP_ALL_ACCESS, FALSE, name);
    if (!g_mapping) return false;
    g_shared = reinterpret_cast<SharedBlock*>(MapViewOfFile(g_mapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedBlock)));
    if (!g_shared || g_shared->magic != kMagic || g_shared->protocolVersion != kProtocolVersion) {
        if (g_shared) UnmapViewOfFile(g_shared);
        g_shared = nullptr;
        CloseHandle(g_mapping); g_mapping = nullptr;
        return false;
    }
    InterlockedExchange(&g_shared->bridgeLoaded, 1);
    return true;
}

void ProcessRequest() {
    if (!EnsureShared()) return;
    const LONG seq = g_shared->requestSeq;
    if (seq == g_shared->completedSeq || seq <= 0) return;
    if (InterlockedCompareExchange(&g_shared->bridgeBusy, 1, 0) != 0) return;

    BridgeResponse response{};
    FoundationSnapshot snap{};
    GameSnapshot game{};
    wchar_t detail[1024]{};
    bool ok = false;

    const BridgeCommand command = static_cast<BridgeCommand>(g_shared->request.command);
    switch (command) {
        case BridgeCommand::ValidateNative:
            ok = NativeValidate(snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2101; break;
        case BridgeCommand::InspectFgMainThread:
            ok = DescribeType("FGStudio.Engine.Utilities", "MainThread", ValidFgMainThreadType,
                              snap.fgMainThreadMethodCount, snap.fgMainThreadFieldCount,
                              snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2201; break;
        case BridgeCommand::InspectUnityDispatcher:
            ok = DescribeType("", "UnityMainThreadDispatcher", ValidUnityDispatcher,
                              snap.unityDispatcherMethodCount, snap.unityDispatcherFieldCount,
                              snap, detail, _countof(detail)); response.errorCode = ok ? 0 : 2301; break;
        case BridgeCommand::ProveUnityMainThread: {
            ok = ProveUnityMainThread(snap, detail, _countof(detail));
            response.errorCode = ok ? 0 : 2401;
            if (ok) {
                wchar_t capabilityDetail[768]{};
                ActionCapabilitySnapshot capabilities{};
                const bool capabilityProbeOk =
                    ProbeActionCapabilities(capabilities, capabilityDetail, _countof(capabilityDetail));
                response.actionCapabilities = capabilities;
                AppendText(detail, _countof(detail), L" | ");
                AppendText(detail, _countof(detail), capabilityDetail);
                if (!capabilityProbeOk) {
                    AppendText(detail, _countof(detail), L" • capability probe incomplete; action remains LOCKED");
                }
            }
            break;
        }
        case BridgeCommand::ReadGameSnapshot:
            ok = ReadGameSnapshot(snap, game, detail, _countof(detail)); response.errorCode = ok ? 0 : 2501; break;
        case BridgeCommand::ProveHookActionEnvelope:
            ok = ProveHarmlessActionEnvelope(g_shared->request, snap, response.infrastructureProof,
                                              detail, _countof(detail));
            response.errorCode = ok ? 0 : 2601;
            break;
        default:
            CopyText(detail, _countof(detail), L"Command không hợp lệ"); response.errorCode = 2003; break;
    }

    response.ok = ok ? 1 : 0;
    response.snapshot = snap;
    response.gameSnapshot = game;
    SetDetail(response, detail);
    g_shared->response = response;
    MemoryBarrier();
    InterlockedExchange(&g_shared->completedSeq, seq);
    InterlockedExchange(&g_shared->bridgeBusy, 0);
}

} // namespace

extern "C" __declspec(dllexport) LRESULT CALLBACK TlGetMessageHook(int code, WPARAM wParam, LPARAM lParam) {
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
        g_shared = nullptr;
        g_mapping = nullptr;
    }
    return TRUE;
}
