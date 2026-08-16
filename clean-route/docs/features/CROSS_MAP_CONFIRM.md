# FEATURE: CROSS-MAP CONFIRM

## Purpose
Recognize the real cross-map confirmation UI and click the user-captured Confirm coordinate only when semantic popup state and tool-owned route context agree.

## Semantic observer
Bridge snapshot exposes `ValidConfirmUi + confirmUiVisible` from read-only `LuaSystemAPI_GUI.MainFindUI("MessageBox")`; `FindUI` is fallback.

## Runtime correction from v1.5.3
The game may show the portal MessageBox while logical AutoPath remains ON. Therefore Path ON is route evidence, not a blocker.

Current order:
`tool-owned cross-map route → MessageBox ON + moving OFF → 200ms settle → if Path ON serialized StopPath → fresh snapshot ≥250ms → saved Confirm click → transition proof`.

Fast v1.5.3 runtime crossed the tested gate with this order.

## v1.5.4 transition handoff
Immediately after the first successful saved Confirm click, ownership passes to the per-PID Transition Freeze. The tool does not wait for `WaitingChangeMap` to become visible before blocking further actions.

During freeze:
- no second Confirm, route, mount, AutoFight, AutoSell or ClickNPC action;
- ReadState/Bridge timeout remains unknown/failure;
- timed-out Bridge request may not be overwritten;
- resume only after fresh state + ready map + no waiting transition + responsive window stay stable for 2000 ms.

If the first Confirm did not take, a controlled retry is possible only after this stable-resume gate clears and fresh semantic MessageBox state still justifies it.

## Donor boundary
v0.8.7/v0.9.0 were studied only for detector/action ordering. Do not transplant remote worker, fixed RVA or old UIButton-tree executor.

## Do-Not-Break
1. semantic UI proof and click engine stay separate;
2. detector invalid/off → no blind click;
3. tool-owned cross-map route required;
4. Path ON must not hide MessageBox handling;
5. post-Confirm Transition Freeze must not be bypassed;
6. timing alone never proves popup existence.

## Validation
- manual Confirm click coordinate: RUNTIME PASS.
- v1.5.3 fast automatic Confirm: RUNTIME PARTIAL PASS.
- v1.5.4 Transition Freeze build: Windows CI PASS run `31931942494`, job `95128013208`.
- v1.5.4 slow-map runtime: NEEDS USER TEST.

## v1.5.5 repeated-death protection
A new death cycle explicitly clears the prior life's `crossMapRouteArmed`, route movement evidence, stall state, Confirm attempts, popup debounce/StopPath timing and route suppression before revive. This does not change the v1.5.3 Confirm detector/action order; it only guarantees the next life starts a fresh cross-map generation.

On verified Địa Phủ MapID 87, the separate M87 AutoFight guard must release first (authoritative AutoFight OFF) before normal route/Confirm logic can run.
