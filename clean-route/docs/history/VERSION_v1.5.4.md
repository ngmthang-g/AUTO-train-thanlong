# VERSION v1.5.4 — 2026-08-16

## A. Identity / Lineage
Version: v1.5.4
Based On: v1.5.3
Reason Created: runtime showed Confirm can work, but slow map transitions produce repeated Bridge/ReadState timeouts and need a global no-action stability gate.
Related: BUG-001, BUG-002, EVID-004

## B. User Request
When game is changing map or temporarily not responding, send no actions. Resume auto only when map/client has become stable for 2 seconds.

## C. State Before Modification
Fast-map v1.5.3 flow can succeed. Slow-map flow may emit repeated `Bridge timeout; fail-closed`, leaving controller without authoritative state.

## D. Root Cause
CONFIRMED:
1. no global freeze existed outside the map-ready check inside TickAccount; ReadState failure prevented TickAccount but did not establish a persistent recovery gate;
2. BridgeClient could write a new request sequence after a timeout even while the prior request might still be pending/executing.
UNKNOWN: exact internal game-side reason for every slow loading delay.

## E. Changes
- Add per-PID `clientFreezeActive`.
- Enter freeze on explicit transition (`!MapReady || WaitingChangeMap`) and Bridge/ReadState timeout/busy.
- During freeze, read-only polling may continue but no mutable state-machine action executes.
- Protect Bridge requests with `pendingSeq`; never overwrite timed-out in-flight request.
- Re-post only the same wake request when bridge is not busy.
- Require successful fresh state + ready map + responsive window continuously for 2000 ms before resuming.
- Reset the stability timer on any read failure, transition state, or unresponsive-window probe.
- Preserve v1.5.3 Confirm behavior.

## F. Important Constants
- stable resume: 2000 ms
- window responsiveness probe timeout: 120 ms
- same-request wake nudge: 750 ms
- repeated failure log throttle: 2000 ms

## G. Build / Runtime
Build/CI: **PASS** — Windows run `31931942494`, job `95128013208`, artifact `9259572967`, digest `sha256:f3c07bd0ca7270fcec806c9e01a28cd9b67913334a183cd1fef453e8d7a5be2a`. Controller SHA256 `582c723501234caef039ea00d9ace0be0ef6a86b37874927b1ac5a6da5c98d3b`; EXE SHA256 `76cf90583c552549052a1c3b615c436b75f8840b313756d19c33ac6d30608e49`; Bridge SHA256 `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`.
Runtime: NEEDS USER TEST.

## H. Desired Runtime Trace
Slow transition:
`Confirm click / map transition → FREEZE ACTION → ReadState may fail without new overlapping requests → state returns → MapReady=1 + WaitingChangeMap=0 + responsive window → stable 2s → mở khóa action → auto continues`.

## I. Post-Confirm freeze refinement
After runtime showed the first Confirm click can be followed by a redundant second click while the transition flag is still late, v1.5.4 enters `clientFreezeActive` immediately after the first successful saved-coordinate Confirm click. This is an expected-transition safety boundary, not proof that the click succeeded. If the client remains on the same popup/map, a retry is possible only after the 2-second stable-resume gate clears and fresh semantic state still justifies it.
