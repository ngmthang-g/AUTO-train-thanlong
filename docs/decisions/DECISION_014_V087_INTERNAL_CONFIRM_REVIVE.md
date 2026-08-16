# DECISION-014 — Transplant v0.8.7 internal UIButton Confirm + Revive

Date / Version: 2026-08-16 / v1.5.10
Status: ACTIVE

## Decision
Use the v0.8.7 runtime-proven internal UI mechanism for exactly two actions:
- cross-map `Xác nhận`;
- death-overlay `Đầu thai`.

Remove their coordinate/timer UI and state completely. Keep the v1.5.9 controller architecture, route FSM, freeze, rotation, M87 guard and AutoSell.

## Safety boundary
The transplant is not a wholesale return to the donor architecture. Raw donor UI RVAs may execute only when:
- current GameAssembly PE identity exactly matches the donor;
- all required donor UI byte signatures match;
- Bridge is executing on the registered game-window thread;
- controller state authorizes the action;
- the target UIButton is active/interactable and uniquely identified.

No donor `remote_worker`, `CreateRemoteThread`, or cross-process mutation pipeline is restored.

## Confirm ownership
A MessageBox is actionable only if a cross-map StartPath issued by this tool is armed. This prevents unrelated dialogs from being consumed merely because they look like a generic confirmation box.

## Revive ownership
Bridge re-reads authoritative `get_IsDeath` immediately before scanning/calling Đầu thai. If IsDeath is false/unknown, no callback is sent.
