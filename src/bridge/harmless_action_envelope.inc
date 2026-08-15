// Harmless infrastructure action-envelope proof.
// Runs only inside the existing WH_GETMESSAGE callback that foundation already proves to be
// the target window thread + managed Unity main thread. No gameplay class/method is resolved
// or invoked here. The only mutation is this bridge-local monotonic proof counter.

volatile LONG g_harmlessProofSequence = 0;

bool ProveHarmlessActionEnvelope(const BridgeRequest& request,
                                 FoundationSnapshot& snap,
                                 InfrastructureProofSnapshot& proof,
                                 wchar_t* detail, std::size_t cap) {
    proof = {};
    if (request.u64Arg0 == 0) {
        CopyText(detail, cap, L"HARMLESS ENVELOPE FAIL: token=0; fail-closed");
        return false;
    }

    wchar_t mainThreadDetail[512]{};
    if (!ProveUnityMainThread(snap, mainThreadDetail, _countof(mainThreadDetail))) {
        ClearText(detail, cap);
        AppendText(detail, cap, L"HARMLESS ENVELOPE FAIL: Unity main thread not proven • ");
        AppendText(detail, cap, mainThreadDetail);
        return false;
    }

    const DWORD callbackThreadId = GetCurrentThreadId();
    if (callbackThreadId != snap.hookThreadId || callbackThreadId != snap.windowThreadId) {
        CopyText(detail, cap,
                 L"HARMLESS ENVELOPE FAIL: callback/hook/window thread identity mismatch; fail-closed");
        return false;
    }
    if (snap.currentManagedThreadId <= 0 ||
        snap.currentManagedThreadId != snap.unityMainManagedThreadId) {
        CopyText(detail, cap,
                 L"HARMLESS ENVELOPE FAIL: managed current/main mismatch; fail-closed");
        return false;
    }

    const LONG next = InterlockedIncrement(&g_harmlessProofSequence);
    if (next <= 0) {
        CopyText(detail, cap, L"HARMLESS ENVELOPE FAIL: proof sequence invalid; fail-closed");
        return false;
    }

    proof.token = request.u64Arg0;
    proof.sequence = static_cast<std::uint32_t>(next);
    proof.callbackThreadId = callbackThreadId;
    proof.currentManagedThreadId = snap.currentManagedThreadId;
    proof.unityMainManagedThreadId = snap.unityMainManagedThreadId;
    proof.completed = 1;

    ClearText(detail, cap);
    AppendText(detail, cap, L"HARMLESS ACTION ENVELOPE PASS • token echoed • seq=");
    AppendUInt(detail, cap, proof.sequence);
    AppendText(detail, cap, L" • callbackTID=");
    AppendUInt(detail, cap, proof.callbackThreadId);
    AppendText(detail, cap, L" • managed current/main=");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(proof.currentManagedThreadId));
    AppendText(detail, cap, L"/");
    AppendUInt(detail, cap, static_cast<std::uint32_t>(proof.unityMainManagedThreadId));
    AppendText(detail, cap,
               L" • bridge-local counter only • NO GAMEPLAY METHOD CALLED • mutation LOCKED");
    return true;
}
