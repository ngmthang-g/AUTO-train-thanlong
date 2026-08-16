# EVIDENCE — v0.8.7 cross-map Confirm while AutoPath remains ON

Date: 2026-08-16
Status: **CONFIRMED FROM SOURCE + USER RUNTIME SCREENSHOT**

## User runtime evidence against v1.5.2

During a real route from Map 5 toward Map 68, the game visibly showed the PK-warning `MessageBox` with `Xác nhận` / `Hủy bỏ`, while CleanRoute status still displayed:

`RUN • Cross-map • AutoPath đang chạy`

Automatic Confirm did not fire.

## Current v1.5.2 source blocker

`HandleCrossMapConfirm` checks `s.autoPathing` before the semantic `MessageBox` branch:

1. if AutoPath is ON, set `crossMapSeenAutoPath`;
2. set status `Cross-map • AutoPath đang chạy`;
3. `return true`;
4. therefore `ValidConfirmUi/confirmUiVisible` is never consumed by the controller in that cycle.

This creates a deterministic deadlock when the client keeps AutoPath logically ON while the portal popup is already visible.

## v0.8.7 direct source evidence

Supplied file: `ThanLongAutoTrain_Source_v0.8.7_buildfix(1).zip`, real `src/main.cpp`.

Relevant behavior:

- `RefreshLive()` reads `MapReady` and `WaitingChangeMap` first.
- If map state is stable, `ReadMessageBoxVisible()` calls `FindMessageBox()`.
- `FindMessageBox()` uses `LuaMainFindUI` with a cached managed string `MessageBox`.
- The worker computes `portalConfirmationReady` from route/gate context, `messageBoxVisible`, movement/progress stall and cooldown.
- **It does not require `autoPathing == false`.**
- When ready it calls `StopPathOnly()` and then `ClickInternalConfirm()`.
- After Confirm it enters `awaitingMapTransition` and waits for real transition evidence.

The v0.8.7 `ClickInternalConfirm()` itself finds active buttons under the `MessageBox`, scores positive labels (`xác nhận`, `confirm`, `ok`, `yes`, etc.), rejects negative/cancel labels, requires a unique candidate, then invokes the button callback.

## Comparison with v0.9.0

Source comparison shows the core functions `FindMessageBox`, `ReadMessageBoxVisible`, `ClickInternalConfirm`, and the `portalConfirmationReady` condition are effectively the same between the supplied v0.8.7 and v0.9.0 sources. The user's observation that v0.8.7 feels faster is therefore more likely caused by surrounding navigation/mount state-machine differences, not a different MessageBox API.

## Design conclusion for CleanRoute

Do **not** copy v0.8.7 remote worker/RVA executor or old UI-tree architecture.

Carry over only the proven ordering principle:

`tool-owned cross-map route + MessageBox ON + physically stopped`
→ `StopPath`
→ fresh snapshot
→ if MessageBox still ON, perform current CleanRoute Confirm action
→ wait for MessageBox OFF / WaitingChangeMap / MapID transition.

`AutoPath ON` is route evidence and may be a state that must be actively stopped; it must never be an early-return reason that hides a visible portal MessageBox.
