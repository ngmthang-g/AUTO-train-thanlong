# Clean Route v1.5.10 — Internal Confirm + Revive donor audit

## Required removals — PASS in source
- no runtime `enableConfirm` / `confirmIntervalSec`; legacy `EnableConfirm` / `ConfirmIntervalSec` INI keys are deletion-only migration data;
- no Confirm interval edit/checkbox;
- no Confirm coordinate capture/display/test;
- no Revive coordinate capture/display/test;
- no `ClickSlot::Confirm` / `ClickSlot::Revive`;
- no periodic Confirm scheduler;
- no generic Bridge `ClickAt` command/implementation.

## Required retained coordinate slots — PASS in source
Exactly four per-account AutoFight points remain:
- AUTO;
- ĐÁNH QUÁI;
- DỪNG AUTO 1;
- DỪNG AUTO 2.

Auto Sell macro coordinates are an independent existing feature and are intentionally retained.

## Donor v0.8.7 internal UI contract — PASS in source
- exact donor PE timestamp/SizeOfImage gate;
- byte signatures for UI donor RVAs;
- donor-exact `MainFindUI("MessageBox")` only; no widened `FindUI` fallback;
- cached managed MessageBox name via IL2CPP GCHandle;
- `UIObject.instances` scan for Revive;
- active/interactable UIButton filtering;
- disposed-object guard;
- donor-exact accent-aware Confirm matcher + unique-candidate policy;
- `UIButton.HandleClickEvent()` for Confirm and Đầu thai;
- fresh authoritative `get_IsDeath` check immediately before Revive callback; after success, no repeated spam—only one late retry after 15s.

## Orchestration contract — PASS in source
- Confirm only for a tool-owned cross-map StartPath;
- MessageBox and moving observer must be authoritative;
- Path ON is evidence, not an early-return blocker;
- StopPath is serialized before Confirm when needed;
- internal Confirm is handled before sell/recovery/train route FSM can issue another action;
- successful Confirm enters Transition Freeze;
- death/revive remains the highest-priority lifecycle flow;
- Map 87 guard remains ahead of normal route flow.

## Local validation
- route/mount test: 15/15 PASS;
- rotation test: 8/8 PASS;
- static audit: PASS;
- lexical delimiter/balance audit: PASS;
- Windows DLL/EXE compilation: NOT RUN in this workspace (Zig/Windows toolchain unavailable).
