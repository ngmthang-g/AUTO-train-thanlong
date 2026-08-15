# CURRENT PROJECT KNOWLEDGE

## Project Purpose
Windows multi-account automation controller for the Thần Long game client. The current Clean Route architecture reads game state through a guarded IL2CPP bridge, makes decisions in the external controller, and serializes actions back onto the game/window thread. The project automates travel to a train spot, AUTO → Đánh quái, death/revive flow, bag-full selling, NPC travel, and return to train.

## Current Version
**v1.5.0 — 2026-08-16**

## Current Status
- BUILD: **PASS** — GitHub Actions run `31907163682`, audit/Route FSM/compiler/package/verify/upload all passed.
- RUNTIME: **NEEDS USER TEST**.
- User-confirmed runtime fact from v1.4.2: all 6 saved click points can be captured and manual `TEST` click correctly.
- User-confirmed runtime bug from v1.4.2: `XÁC NHẬN RA MAP` manual test works, but the automatic real flow does NOT reliably press it.

## Architecture
`Resolver → read-only Scanner → Snapshot/State Store → Observer → State Machine → Safety Guard → Action Queue (max one logical action) → Unity/Game Window Thread Dispatcher → Internal Action Engine`

The controller may use foreground `SendInput` for the explicitly configured UI click points because this is the currently verified practical input path. Internal route/NPC actions continue to use the bridge.

## Major Components
- `src/controller.cpp`: multi-account UI, profile persistence, route/sell/death/fight state machines, foreground click execution.
- `src/bridge.cpp`: IL2CPP metadata resolution, read-only snapshot, guarded `ToggleRide`, `StartAutoPath`, `StopAutoPath`, `ClickNPC`.
- `src/protocol.h`: shared controller/bridge command and snapshot protocol.
- `src/route_logic.h`: minimal pure route decision helpers.
- `ThanLongCleanRoute.accounts.ini`: RoleID profiles and shared sell-NPC positions.
- `ThanLongCleanRoute.spots.tsv`: shared train-spot library.

## Confirmed Technical Facts
### CONFIRMED — click points
The user runtime-tested all six click slots and confirmed capture + manual test are working:
1. Confirm
2. Revive
3. AutoMenu
4. Attack
5. StopAuto1
6. StopAuto2

Therefore, when an automatic click fails while the manual test succeeds, investigate **flow timing/state gating first**, not coordinate capture or `RealInputClick` first.

### CONFIRMED — NPC presets
- Mã Kiêu Minh: MapID 5, NPC ID/ResID 373.
- Dược Đại Phu: MapID 55, NPC ID/ResID 279.
- NPC route X/Y are **not hard-coded**. The user supplies X/Y manually or captures the current Map/X/Y in the tool.

### CONFIRMED — bag state
`LuaSystemAPI_Game.GetFreeBagSpace()` is used read-only as the bag-full trigger and sale-completion guard.

### CONFIRMED — NPC open action
`LuaSystemAPI_Game.ClickNPC(npcID)` is used by the bridge to open the selected NPC after travel.

## Important Methods / Functions
- Controller: `TickAccount`, `HandleDeath`, `HandleCrossMapConfirm`, `HandleFightClicks`, `HandleAutoSell`, `DriveTravelWithMountFallback`, `BeginTrainCorrection`, `HandleTrainCorrection`.
- Bridge/runtime: `GetFreeBagSpace`, `ClickNPC`, `SendToggleRideState`, `StartAutoPath`, `StopAutoPath`, `get_IsDeath`, `get_EnableAutoF1`.

## Important IDs
- NPC 373 — Mã Kiêu Minh — MapID 5.
- NPC 279 — Dược Đại Phu — MapID 55.
- Protocol commands: ReadState=1, ToggleRide=2, StartPath=3, StopPath=4, ClickAt=5 diagnostic compatibility, ClickNpc=6.

## Runtime Flow — v1.5.0
### Train stable state
`arrive target → StopPath → dismount → AUTO → Đánh quái → start 180s coordinate timer`

While the 180-second timer is running:
- snapshots are still read;
- death handling remains immediate;
- bag-full handling remains immediate;
- controller **does not compare Map/X/Y for route correction** until the timer expires.

At each 180-second check:
- if Map/X/Y are still within target tolerance → reset timer for another 180 seconds;
- if deviated → begin train correction.

### Train correction
`3-minute check detects deviation → StopAuto1 → StopAuto2 → verify auto-fight off → travel back to train target → AUTO → Đánh quái → restart 180s timer`

### Mount retry / walk fallback
Used by deviation correction and Auto Sell travel (also return-to-train after selling):
1. If already riding → StartAutoPath.
2. If not riding → send mount attempt #1.
3. Wait 5 seconds and verify `riding` snapshot.
4. If still not riding → send mount attempt #2.
5. Wait another 5 seconds.
6. If still not riding → allow StartAutoPath on foot for at most 15 seconds.
7. If target not reached in 15 seconds → StopPath if needed, reset mount cycle, repeat from attempt #1.
8. If riding becomes true at any time → clear fallback cycle and continue mounted AutoPath.

### Auto Sell
`FreeBagSpace <= 0 → stop AUTO using StopAuto1/2 → mount retry/walk fallback → user-configured NPC X/Y → ClickNPC(ID) → coordinate macro → verify FreeBagSpace > 0 stable → mount retry/walk fallback back to train → AUTO → Đánh quái`

### Confirm / map-transition retry
Because manual test of the Confirm point is runtime-confirmed working but automatic confirmation was unreliable, v1.5.0 no longer treats one OS click as proof of UI success.

Cross-map flow:
- after AutoPath was seen and then stalls on the wrong map, wait for the popup window timing;
- first Confirm attempt after 2.5 s of stable stall;
- retry Confirm every 1.8 s, maximum 4 attempts;
- keep the portal state during retries instead of clearing it after 2.5 s;
- only after the full retry window fails, release the state so AutoPath can reacquire the portal.

Death/Đầu thai flow:
- after clicking Revive/Đầu thai, first Confirm attempt waits 1.4 s;
- retry every 1.8 s, maximum 4 attempts while the character remains dead and no map transition has started.

## Working Mechanisms
- Foreground `SendInput` with cursor left at target, 140 ms mouse hold.
- F8 capture.
- F4 global controller pause + StopPath.
- Per-RoleID 6 fixed click points.
- User-configured shared NPC X/Y.
- Read-only BagSpace guard.
- Internal `ClickNPC` open action.
- Dynamic sell macro with delay/repeat.

## Failed / Unsafe Mechanisms
- `CreateRemoteThread` / `remote_worker` for gameplay action: **DEPRECATED / forbidden** due architecture and disconnect risk.
- Direct `WriteProcessMemory`: **forbidden**.
- `RequestSellItem` / direct inventory mutation: **not used**.
- Hard-coded NPC X/Y from v1.4.1: **FAILED / DEPRECATED**. User explicitly requires obtaining NPC coordinates from the game and configuring them in the tool.
- Continuous per-tick train-position correction from v1.4.2: **SUPERSEDED** by the 180-second cadence requested in v1.5.0.

## Architectural Rules
1. v0.9.0 remains donor/reference only for facts already verified.
2. No `CreateRemoteThread` / `remote_worker` gameplay action path.
3. No memory writes for movement/UI/gameplay.
4. At most one logical action should be active for an account at a time.
5. Treat `BUILD PASS` and `RUNTIME PASS` as different states.
6. Do not hard-code sell-NPC X/Y; user supplies/captures them.
7. Manual click-test success does not prove automatic-flow timing success.

## Known Bugs / Current Limitations
- v1.5.0 runtime behavior is not confirmed until user tests it.
- Confirm popup visibility has no verified semantic getter; confirmation remains a guarded timing/state retry strategy.
- Sell-shop macro remains coordinate-driven; semantic shop button actions are not yet verified.
- Foot AutoPath is expected to provide temporary walking fallback, but must be runtime-tested in the actual client.

## Open Research Questions
- Can a reliable popup-visible state or semantic confirm action be resolved from IL2CPP/UI hierarchy to replace timing retries?
- Can seller/shop semantic methods replace the coordinate sell macro without unsafe direct inventory mutation?

## Next Development Priorities
1. Runtime-test v1.5.0 3-minute cadence.
2. Runtime-test mount attempts 1/2 and 15-second walking fallback.
3. Runtime-test automatic Confirm after Đầu thai and cross-map portal.
4. Capture logs for exact state/timing if any of the above still fails.

# DECISION LOG

## DECISION-001
**Date:** 2026-08-16  
**Status:** ACTIVE  
**Decision:** Once train AUTO→Đánh quái is active, Map/X/Y route correction runs only every 180 seconds.  
**Context:** Per-tick coordinate checks caused unwanted repeated route intervention while training.  
**Reason:** User explicitly requires a 3-minute cadence after the character is already at the train spot and fighting.  
**Consequences:** Death and bag checks stay real-time; route deviation can intentionally remain uncorrected for up to 180 seconds.

## DECISION-002
**Date:** 2026-08-16  
**Status:** ACTIVE  
**Decision:** Travel recovery uses two mount attempts, each followed by a 5-second verification window, then up to 15 seconds walking, then repeats.  
**Context:** Mount may fail transiently, and route must not remain stuck forever waiting for riding state.  
**Alternatives Considered:** spam mount continuously; walk forever; give up after two mount failures.  
**Why Rejected:** spam is unstable; permanent walking is slow; giving up breaks unattended automation.  
**Consequences:** The same fallback is reused for train correction and sell travel/return.

## DECISION-003
**Date:** 2026-08-16  
**Status:** ACTIVE  
**Decision:** Automatic Confirm uses timed retries because manual coordinate test is confirmed working while automatic flow timing is not.  
**Context:** v1.4.2 automatic Confirm can fail although TEST click succeeds.  
**Reason:** Source shows the previous flow assumed a sent click meant success and cross-map state could be cleared before the UI was actually ready.  
**Consequences:** Retry window is bounded and map-transition state remains the stop condition.

# CORRECTION LOG

## CORRECTION-001
**Old Knowledge:** NPC sell coordinates could be shipped as defaults (`284,188`, `96,168`).  
**New Finding:** User requires sell-NPC coordinates to be captured/configured from the game; hard-coded values caused incorrect travel.  
**Evidence:** User runtime feedback.  
**Affected Versions:** v1.4.1.  
**Impact:** v1.4.2+ contains no default NPC X/Y.  
**Status:** DEPRECATED.

## CORRECTION-002
**Old Knowledge:** A single Confirm click after a fixed delay was sufficient if manual Test worked.  
**New Finding:** Manual Test works for Confirm, but automatic real flow does not reliably activate it.  
**Evidence:** User runtime test on v1.4.2.  
**Affected Versions:** v1.4.2 and earlier.  
**Impact:** v1.5.0 adds bounded retries and preserves cross-map stall state longer.  
**Status:** SUPERSEDED.

# VERSION HISTORY

## VERSION 1.4.0 — 2026-08-16
### Technical Findings
- Added mouse-lock foreground click, F4 pause, six click slots, BagSpace, ClickNPC and dynamic Auto Sell macro.
### Test Results
- BUILD PASS in CI.
- Overall runtime status at release: NEEDS USER TEST.

## VERSION 1.4.1 — 2026-08-16
### User Requests
- Intended to correct NPC travel coordinates.
### Failed Attempts
- Hard-coded user-reported NPC coordinates into presets.
### Result
- **FAILED / DEPRECATED** because the user intended to obtain/configure coordinates in-game instead of shipping defaults.

## VERSION 1.4.2 — 2026-08-16
### Solution
- Removed hard-coded NPC X/Y; added user X/Y + `LẤY VỊ TRÍ`, shared by NPC.
### Test Results
- BUILD PASS in CI.
- User runtime: all six manual click tests PASS.
- User runtime: automatic Confirm real flow FAIL/PARTIAL despite manual Confirm TEST PASS.
### Known Limitation
- Train target was still evaluated continuously by `TickAccount` rather than at the requested 3-minute steady-state interval.

## VERSION 1.5.0 — 2026-08-16
### User Requests
- After reaching train target and starting auto fight, check Map/X/Y only once every 3 minutes.
- On deviation: stop auto, mount attempt #1, wait 5s; attempt #2, wait 5s; if still not mounted, walk at most 15s, then repeat.
- Apply the same mount fallback when bag is full and travelling to sell.
- Fix automatic Confirm because manual test works but actual flow fails.
- Apply the AI Project Knowledge Protocol and preserve version knowledge.

### Initial State
- v1.4.2.
- Six manual click tests confirmed working.
- Confirm integration runtime failure reported.
- No `PROJECT_KNOWLEDGE.md` or `CHANGELOG.md` existed in the Clean Route project root.

### Root Cause / Investigation
- **CONFIRMED source behavior:** train position was compared every controller tick.
- **CONFIRMED source behavior:** route logic only mounted before StartPath; there was no two-attempt + walk fallback state machine.
- **CONFIRMED source behavior:** death confirm was sent once after 900 ms and then treated as done if `SendInput` succeeded.
- **CONFIRMED source behavior:** cross-map confirm state could be cleared and route reacquired before a robust multi-click confirmation window completed.
- **LIKELY runtime cause:** automatic Confirm is being sent outside the popup-ready timing window, not to a wrong saved coordinate, because manual Test at that coordinate passes.

### Implementation Details
- Added `kTargetRecheckMs = 180000` and steady-train cadence state.
- Added `DriveTravelWithMountFallback` with 5s / 5s / 15s cycle.
- Added train-correction stop-auto flow.
- Auto Sell NPC travel and return-to-train use the same travel fallback.
- Reworked cross-map Confirm to 2.5s initial wait + 1.8s retries, max 4.
- Reworked post-Đầu-thai Confirm to 1.4s initial wait + 1.8s retries, max 4.
- Initialized `PROJECT_KNOWLEDGE.md` and `CHANGELOG.md` per project protocol.

### Files Changed
- `src/controller.cpp`
- `resources/app.rc`
- `build.cmd`
- `README.md`
- `CLEAN_REWRITE_AUDIT.md`
- `PROJECT_KNOWLEDGE.md` (new)
- `CHANGELOG.md` (new)
- GitHub build workflow/version packaging

### Test Results
- Static source review: PASS.
- BUILD PASS proof: GitHub Actions run `31907163682` (source/binary build before the KB-only proof metadata refresh).
- Controller reconstruction SHA-256: `b9f420c77d022da4ad7525b24a77dea92fb99eab8089419ea2f687381baf8366`.
- Route FSM self-test: 8/8 PASS.
- Bridge PE DLL + LoadLibrary/export verify: PASS.
- EXE SHA-256: `29dc6d673abe060d500c4516b027de93dc1efc119adea4015482c06e86e52bf8`.
- DLL SHA-256: `fea4c2c6df908a4f0742b44fe345d311b48565fb3784f4b30491b1a8b6af2aa8`.
- Historical source ZIP SHA-256 from build-proof run `31907163682`: `bb9fd3eded92789ffaacd9d72c80a115c21c4b84f76f0b0bb52167a5877000ae`. This is intentionally not treated as a self-hash of later KB-only packaging refreshes.
- Historical Actions artifact ZIP SHA-256 from build-proof run `31907163682`: `bdfb3940ea85da204d40a3175d607f8a59f802858ca17929f306958e7dc26313`.
- Runtime: NEEDS USER TEST.

### Things Future AI Must Preserve
- Do not revert to per-tick train coordinate correction while stable AUTO train is running unless the user explicitly changes the 3-minute rule.
- Do not remove the 5s/5s/15s mount fallback without a replacement decision.
- Do not assume manual click Test success proves automatic timing success.
- Do not hard-code sell NPC X/Y.
