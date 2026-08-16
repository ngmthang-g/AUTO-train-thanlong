## v1.5.9 — Auto Rotate Train Spots (WINDOWS CI PASS / RUNTIME UNTESTED)
- Rotation members are complete shared train spots (`Name/MapID/X/Y`), not raw maps.
- Per-account checked pool; one item = stay, multiple = round-robin.
- Death evidence is a rolling window outside RuntimeState so death hard resets do not erase it. Default: >10 deaths / 10 minutes.
- Efficiency evidence is productive train time outside RuntimeState. Default: 15 minutes without a FULL-bag edge.
- Productive time requires alive + at target + AutoFight ON + authoritative FreeBagSpace.
- FULL bag resets only the efficiency timer; death history remains independent until a spot switch.
- Switching spots resets both trigger windows for the new spot.
- Live spot switch uses existing train-recovery Stop-Auto flow before routing; death switch changes target before revive return.
- Windows x64 staging CI **PASS** — run `31949891312`, job `95171640279`, artifact `9264382136`; route test 15/15, rotation test 8/8, Bridge/EXE/package verification PASS.
- v1.5.9 Windows controller SHA256 `11739f9d11d056107d6733a34f7d88ae60008a0c9ace2d2eaca97409a4831bbf`; canonical LF source `5900c02b44a62d72ba2e197cd92e8cf6736245d70bd355b9f72f331f7516d559`; EXE `d6bcc8fd9e02ae818499d776498877ee51099854633556febd94ee945ef8d2bc`; DLL `5d7adcb842378cdd91225ee476efb5e5e8dbe4bf67929af7afc97af6eb951a24`; source ZIP `7c1297f717ebb880932d83cccb90e0fc9236b49762ab8b14b1cc288ee37b3b1e`; artifact digest `efc36d0fb66593370b49e92e63bfabe67ce09edbbd70a34488ce6c6920ce807d`.
- Runtime auto-rotation is **NEEDS USER TEST**; build success must not be treated as runtime proof.

# CURRENT v1.5.6 KNOWLEDGE
- Repeated-death bug motivated replacing partial per-field death reset with full `ResetRuntime()`.
- `deathSessionLatched` MUST remain outside RuntimeState.
- DEAD edge: hard reset once, latch session, then revive.
- ALIVE edge: hard reset again, clear latch, next tick is equivalent to a fresh Start.
- Do not move saved settings/spot/click/macro data into RuntimeState; they must survive cold restart.
- Map 87 AutoFight guard and semantic Confirm remain subsequent gates.
- v1.5.6 controller target SHA256 `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`.
- Build status: WINDOWS CI PASS — run `31938154271`, job `95143227671`, artifact `9261270126`; controller `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`; EXE `5651cc8f440628a298391c01a627480eccca1e27b1458f1db739f373573b204b`.
- Runtime status: UNTESTED until multiple consecutive death cycles pass.

# CURRENT PROJECT KNOWLEDGE

## Project Purpose
Windows multi-account automation/controller for Thần Long PC client. Current scope: route to saved train spots, revive/cross-map confirmation, AUTO→Đánh quái, 3-minute train-coordinate guard, bag-full NPC sell macro, robust mount fallback and scheduled tool close.

## Current Version
**v1.5.5 — 2026-08-16**

## Current Status
- Route/mount pure self-test: **15/15 PASS**.
- Manual TEST of all six saved click points: **RUNTIME PASS** by user.
- v1.5.3 automatic Confirm: **RUNTIME PARTIAL PASS** — fast transition successfully detected `MessageBox=1 + Path=ON`, sent StopPath, clicked saved Confirm, crossed map and reached the train spot.
- Slow transition: **CONFIRMED BUG** — repeated `ReadState fail: Bridge timeout; fail-closed` for multiple seconds; snapshot is not authoritative during this interval.
- v1.5.4 Transition Freeze: **WINDOWS CI PASS / RUNTIME PARTIAL PASS** — user log confirms freeze during map loading/timeouts and `CLIENT ỔN ĐỊNH LIÊN TỤC 2s → mở khóa action`; first tested automatic Confirm path also succeeded.
- v1.5.4 second-death cycle: **RUNTIME FAIL/PARTIAL** — after a later death/revive, route resumed but automatic map Confirm was no longer observed.
- v1.5.5 Địa Phủ/death-cycle fix: **WINDOWS CI PASS / RUNTIME UNTESTED**. Code-validation run `31934654526`, job `95134583587`; controller SHA256 `73451f4fbc244c9b05ef3f86ff3daeb445b3e4bffd0be6e534bc514cb0759d8e`. Verified static client data says Địa Phủ is MapID 87.
- LocalAppData persistence + PID→Role merge implemented; restart persistence still needs user test.

## Architecture
`Resolver → read-only Snapshot → Observer → State Machine → Safety Guard → serialized action → Bridge/game-window action → fresh state proof`.

Hard rules:
- v0.8.7/v0.9.0 are donor/reference only.
- Never restore CreateRemoteThread/remote_worker/WriteProcessMemory gameplay actions.
- Timeout = UNKNOWN/unstable, never success.
- At most one mutable action/request in flight per PID.
- AutoFight watchdog never interleaves with another action flow.
- DATA repo `ngmthang-g/clinent-game-than-long-DATA-2222` is external knowledge/evidence, not CleanRoute source.

## Active Behavior

### AutoFight
- Read-only `ValidAutoFight + autoFight`.
- Check once at train arrival.
- If OFF, use saved AUTO → Attack clicks and verify.
- Recheck every 60000 ms while steady training.
- Defer completely while revive/sell/navigation/mount/Confirm/recovery/map-transition/freeze owns the PID.

### Train coordinate / travel
- Check train coordinates every 180000 ms after AutoFight ON.
- Robust travel: mount #1 → 5s → mount #2 → 5s → foot AutoPath max 15s → repeat.

### Persistence
Primary config:
`%LOCALAPPDATA%\ThanLongCleanRoute\ThanLongCleanRoute.accounts.ini`

- legacy EXE-local INI migrates;
- PID data merges into Role profile field-by-field;
- six click points, sell macro and shared NPC coordinates save/flush.

### Auto Sell
- `GetFreeBagSpace()` is read-only bag trigger/verification.
- NPC X/Y are user-captured; never hard-coded.
- Mã Kiêu Minh: MapID 5, ID 373.
- Dược Đại Phu: MapID 55, ID 279.

## Cross-map Confirm — v1.5.3 mechanism retained
Runtime proved the portal MessageBox can exist while logical AutoPath remains ON. v1.5.2 failed because it returned on Path ON before consuming `confirmUiVisible`.

Current order:
1. tool-owned cross-map route is armed;
2. AutoPath ON is route evidence, not an early-return blocker;
3. Bridge observes `MainFindUI("MessageBox")` read-only (`FindUI` fallback);
4. require authoritative `MessageBox ON` + moving=OFF;
5. settle 200 ms;
6. if Path ON, issue one serialized StopPath;
7. fresh snapshot, wait ≥250 ms;
8. if MessageBox remains ON, use saved Confirm coordinate;
9. wait for real transition/map proof.

Fast v1.5.3 runtime demonstrated this sequence can cross the tested gate.

## v1.5.5 — Địa Phủ recovery + fresh death generation

Verified client fact:
- `database/MAPS.csv` in the canonical DATA repo contains `87,Địa Phủ,siwang,0,Wild,1,siwang`; **M87 is Địa Phủ**.

New death generation rule:
- when a new `get_IsDeath=1` cycle begins, reset `crossMapSeenAutoPath`, `crossMapRouteArmed`, `crossMapRouteMoved`, stall timing, Confirm attempt/debounce timing, Confirm StopPath timing, route suppression and M87 stop state;
- this prevents a second life from inheriting route/Confirm ownership from the previous life.

M87 ownership rule:
- after revive and stable state, if `MapID==87`, M87 guard runs before normal map qualification, sell, recovery, Confirm, watchdog and route logic;
- `ValidAutoFight` must be authoritative; otherwise wait and send no M87 stop clicks;
- AutoFight ON → saved `DỪNG AUTO 1` → 700 ms → saved `DỪNG AUTO 2` → verify fresh AutoFight state;
- if still ON after 1200 ms, retry, maximum 3 full stop sequences;
- only verified AutoFight OFF releases normal route back to the selected train spot.

Evidence boundary:
- missing dedicated M87 AutoFight-stop arbitration and lack of explicit new-death cross-map reset were confirmed source gaps;
- AutoFight competition is a plausible contributor to the second-death Confirm miss, but the exact causal chain remains **LIKELY**, not CONFIRMED, until v1.5.5 runtime logs are tested.

## v1.5.4 Transition Freeze / Client-Unstable Gate

### User requirement
If game is changing map or temporarily not responding, **send no actions**. Resume only after client/map is stable continuously for 2 seconds.

### Freeze triggers per PID
Enter `clientFreezeActive` when:
- `MapReady == 0`;
- `WaitingChangeMap == 1`;
- ReadState/Bridge timeout or busy;
- mutable Bridge action timeout/busy;
- immediately after the first successful saved Confirm click, because a transition is expected before the game may publish transition flags.

### While frozen
- read-only recovery/polling may continue;
- no route/mount/AutoFight/AutoSell/ClickNPC/saved-coordinate gameplay action runs;
- do not inject extra StopPath from F4/tool-close into an already unstable Bridge;
- do not overwrite a timed-out Bridge request.

### Bridge request serialization
`BridgeClient` records timed-out `pendingSeq`.
- Until `completedSeq == pendingSeq`, no new request payload/sequence is written.
- If bridge is not busy, controller may only re-post the same wake request at bounded intervals.
- Timeout stays failure/unknown.

### Resume gate
Automation resumes only after **2000 ms continuously** of:
1. successful fresh ReadState;
2. `MapReady == 1`;
3. `WaitingChangeMap == 0`;
4. responsive game window via bounded WM_NULL probe.

Any failed read, transition state or unresponsive-window probe resets the 2-second stability timer.

Expected trace:
`FREEZE ACTION → state recovers → MAP/CLIENT ĐÃ PHẢN HỒI → stable 2s → CLIENT ỔN ĐỊNH LIÊN TỤC 2s → mở khóa action → auto continues`.

## Failed / Superseded
- v1.5.0 blind/timing Confirm retry heuristic — failed.
- v1.5.2 Path-ON early return before semantic MessageBox — failed.
- one successful state read immediately after loading is NOT enough to resume — superseded by 2-second stable gate.
- hard-coded NPC X/Y and scale guessing — deprecated.
- remote-worker gameplay actions — unsafe/deprecated.

## Build Evidence — v1.5.4
Windows code-validation CI **PASS**:
- run `31931942494`
- job `95128013208`
- artifact `9259572967`
- artifact digest `sha256:f3c07bd0ca7270fcec806c9e01a28cd9b67913334a183cd1fef453e8d7a5be2a`
- controller SHA256 `582c723501234caef039ea00d9ace0be0ef6a86b37874927b1ac5a6da5c98d3b`
- EXE SHA256 `76cf90583c552549052a1c3b615c436b75f8840b313756d19c33ac6d30608e49`
- Bridge SHA256 `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`

This proves build/static/self-test contracts only. **v1.5.4 slow-map runtime remains NEEDS USER TEST.**

## Open Runtime Tests
1. Slow map: verify no mutable action during timeout/loading freeze and resume only after 2 stable seconds.
2. If first Confirm click genuinely fails, verify retry occurs only after freeze clears and fresh MessageBox state still justifies it.
3. AutoFight 60-second watchdog concurrency.
4. Restart persistence for six clicks + sell macro.
5. 3-minute drift + robust mount fallback.

## CORRECTION-007 — v1.5.3 runtime result
Prior status `runtime untested` is deprecated. v1.5.3 Confirm is **RUNTIME PARTIAL PASS**: fast transition crossed and reached train spot; slow transition exposed the separate timeout-storm reliability issue.

## DECISION-010 — ACTIVE — global transition/unresponsive freeze
When map transition is explicit/expected or state transport is non-authoritative, the PID owns a global FREEZE state. No mutable action may run until fresh state, ready map, no waiting transition, and responsive game window remain continuously stable for 2000 ms. A timed-out Bridge request may never be overwritten by a newer request sequence.
