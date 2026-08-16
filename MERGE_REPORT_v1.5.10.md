# MERGE REPORT — v1.5.10 Internal Confirm + Revive v0.8.7

Base: `ThanLongCleanRoute v1.5.9 Auto Rotate Train Spots`  
Donor: `ThanLongAutoTrain v0.8.7 buildfix`

## Scope transplanted from v0.8.7

Only the proven semantic UI slice for these two actions is transplanted:

1. Cross-map Confirm
   - cached managed `MessageBox` string via IL2CPP GCHandle;
   - donor-exact `LuaMainFindUI("MessageBox")` detection;
   - MessageBox tree traversal;
   - active/interactable `UIButton` filtering;
   - accent-aware donor positive/negative button matcher;
   - unique-candidate guard;
   - `UIButton.HandleClickEvent()` invocation.

2. Revive / Đầu thai
   - fresh authoritative `get_IsDeath` re-check immediately before action;
   - scan `UIObject.instances`;
   - reject disposed UI objects;
   - two-pass direct/descendant label matching for `Đầu thai`;
   - unique-candidate guard;
   - `UIButton.HandleClickEvent()` invocation;
   - donor-style anti-spam cadence: failed detection retries after 2s; after first successful callback, only one late retry after 15s while still dead.

## Execution boundary intentionally NOT transplanted

The v0.8.7 `RemoteExecutor` / `CreateRemoteThread` pipeline is **not** restored. v1.5.10 keeps the existing v1.5.9 serialized Bridge on the registered game-window thread, with one request in flight per PID and timeout/freeze behavior.

Raw v0.8.7 UI RVAs are callable only after:
- exact GameAssembly TimeDateStamp `0x6A410C14`;
- exact SizeOfImage `0x03DCB000`;
- required donor UI byte signatures match.

Transient startup unreadiness is not cached as a permanent mismatch. Proven PE/signature mismatch is cached FAIL and blocks the internal action.

## Removed from v1.5.9

- Confirm coordinate slot;
- Revive coordinate slot;
- Confirm capture/test controls;
- Revive capture/test controls;
- Confirm checkbox;
- `ConfirmIntervalSec` editor and periodic scheduler;
- `ClickSlot::Confirm` / `ClickSlot::Revive`;
- generic Bridge `ClickAt` command/implementation.

On the next profile save, legacy INI keys for Confirm/Revive coordinates and the old Confirm timer are deleted.

## Retained coordinate clicks

Exactly four per-account calibration points remain:
- AUTO;
- ĐÁNH QUÁI;
- DỪNG AUTO 1;
- DỪNG AUTO 2.

Auto Sell macro coordinates remain because they are a separate existing feature.

## Portal ownership / anti-race rules

`tool-owned cross-map StartPath -> authoritative MessageBox -> authoritative moving=OFF -> 200ms settle -> StopPath if still ON -> >=250ms fresh cycle -> internal Confirm -> immediate Transition Freeze`

A generic/unrelated MessageBox is not actionable without tool-owned cross-map route ownership. No second mutable action is allowed in the same controller cycle after Confirm.

## Death ownership / anti-race rules

Fresh death hard-resets prior runtime state. If AutoPath is authoritatively ON and the map is stable, StopPath is sent once on the death edge before touching death UI. Revive then waits for the overlay, re-checks `get_IsDeath` inside the Bridge, invokes a uniquely identified `Đầu thai`, and locks UI/path actions until the ALIVE cold restart.

## Validation performed in this workspace

- Route/mount logic test: 15/15 PASS.
- Rotation logic test: 8/8 PASS.
- Integration/static invariant audit: 33/33 PASS.
- `bridge.cpp` Clang C++17 syntax check with WinAPI stub: PASS with `-Wall -Wextra -Werror`.
- Lexical delimiter/string/comment balance: `controller.cpp`, `bridge.cpp`, `protocol.h` PASS.

Windows PE build was **not** performed here because Zig/MinGW Windows cross toolchain is unavailable in this workspace. Runtime with the real game client therefore remains **NEEDS USER TEST**.
