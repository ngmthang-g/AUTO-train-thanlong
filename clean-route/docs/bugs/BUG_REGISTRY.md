# BUG REGISTRY

## BUG-001 — Automatic cross-map Confirm blocked when portal MessageBox appeared with AutoPath still ON
Status: **v1.5.3 RUNTIME PARTIAL PASS / protected in v1.5.4**

Evidence:
- manual Confirm coordinate TEST PASS;
- v1.5.2 had semantic `ValidConfirmUi + confirmUiVisible` but returned early on `s.autoPathing` before consuming it;
- supplied v0.8.7 source does not require AutoPath OFF before portal-ready; it StopPaths immediately before Confirm;
- v1.5.3 fast runtime successfully executed MessageBox ON + Path ON → StopPath → saved Confirm → destination reached.

Root cause: **CONFIRMED v1.5.2 ordering defect**.

Do not restore the Path-ON early return before semantic MessageBox handling. Do not replace semantic proof with blind sleeps/clicks.

## BUG-002 — Slow map transition causes repeated Bridge/ReadState timeouts
Status: **FIXED-BUILD PASS / RUNTIME UNTESTED v1.5.4**

Severity: High for orchestration reliability.

Runtime evidence:
- fast map transition can complete Confirm and reach destination;
- slow transition produces repeated `ReadState fail: Bridge timeout; fail-closed` for multiple seconds;
- during timeout snapshot is not authoritative and state-machine actions must not continue.

Root cause / boundary:
- **CONFIRMED orchestration defect:** pre-v1.5.4 controller had no persistent global unstable-client freeze and could write a newer Bridge request while an earlier timed-out request might still be pending/executing.
- **UNKNOWN:** exact internal game-thread reason for every slow loading delay; do not assume MessageBox observer alone is the cause.

Fix v1.5.4:
- per-PID FREEZE on explicit/expected transition or ReadState/Bridge timeout/busy;
- first successful Confirm click proactively enters freeze;
- no mutable actions while frozen;
- preserve timed-out `pendingSeq`, refuse overlapping newer request;
- resume only after fresh state + MapReady + !WaitingChangeMap + responsive window remain stable continuously for 2000 ms.

Build Verified In:
- Windows CI run `31931942494`, job `95128013208` PASS;
- controller SHA256 `582c723501234caef039ea00d9ace0be0ef6a86b37874927b1ac5a6da5c98d3b`.

Runtime Verified In: NONE yet.

Do-Not-Do:
- do not increase timeout and continue sending actions blindly;
- do not overwrite shared Bridge request while prior request may still be executing;
- do not treat one successful read immediately after loading as enough to resume all automation.

## BUG-003 — Second death/revive cycle can return without automatic portal Confirm
Status: **v1.5.6 COLD-RESTART BUILD PASS / RUNTIME UNTESTED**

Severity: High for unattended death recovery.

Runtime evidence:
- first v1.5.4 tested death/return gate successfully detected MessageBox while Path ON, StopPathed, clicked Confirm and entered Transition Freeze;
- after a later/second death, the character routed back out but the automatic Confirm event was no longer observed.

Verified supporting fact:
- Địa Phủ is **MapID 87** from canonical `database/MAPS.csv`.

Source gaps confirmed:
- no dedicated M87 ownership guard existed to force AutoFight OFF before post-revive routing;
- a fresh death did not explicitly reset the complete cross-map/Confirm generation immediately at death-cycle start.

Root cause status:
- exact reason Confirm was missed on the second cycle: **UNKNOWN/PARTIAL**;
- built-in AutoFight competing with post-revive routing: **LIKELY contributor**, not yet runtime-proven.

Build verification:
- Windows code-validation CI PASS — run `31934654526`, job `95134583587`, controller SHA256 `73451f4fbc244c9b05ef3f86ff3daeb445b3e4bffd0be6e534bc514cb0759d8e`.

Fix v1.5.5:
- reset route/Confirm generation on every new death;
- on M87, authoritative AutoFight ON must be stopped with saved StopAuto1/2 and verified OFF before normal route proceeds;
- retry stop sequence max 3 times, fail closed otherwise.

v1.5.6 superseding fix:
- replace per-field death reset with full `ResetRuntime()` at the first DEAD edge;
- store `deathSessionLatched` outside RuntimeState;
- full `ResetRuntime()` again on ALIVE edge so the next tick is equivalent to fresh Start;
- Windows CI PASS run `31938154271`, job `95143227671`, controller `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`.

Next runtime proof:
- perform at least three consecutive deaths and verify exactly one `NEW DEATH SESSION` and one `POST-REVIVE COLD START` per cycle, followed by fresh M87/route/MessageBox/Confirm flow.
