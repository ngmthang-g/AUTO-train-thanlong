# VERSION v1.5.2 — 2026-08-16

## A. Identity / Lineage
Version: v1.5.2
Date: 2026-08-16
Based On: v1.5.1
Reason Created: automatic cross-map Confirm still does not reliably recognize when the map confirmation popup exists.
Last Known-Good: user reports older v0.9.0 detector and v1.4.0/v1.4.1 actual Confirm-route behavior recognized the needed moment well.
Regression From: recent CleanRoute versions; exact first bad binary remains UNKNOWN.
Supersedes: timing-only Confirm recognition.

## B. User Requests
- Treat `ngmthang-g/clinent-game-than-long-DATA-2222` as the client knowledge/data repository, not the CleanRoute source repo.
- Make uploaded `AI_PROJECT_KNOWLEDGE_PROTOCOL_V2_OPTIMIZED.md` and `AI_CLIENT_ANALYSIS_RULES.txt` mandatory memory for every version/task.
- Keep the project summary/lineage inside the CleanRoute source package.
- Inspect supplied v0.9.0 source only for **how it recognizes when Confirm is needed**.
- Do not transplant old worker/RVA/UI-click implementation.
- Find a robust solution for current CleanRoute.

## C. State Before Modification
Working:
- manual Confirm coordinate TEST works;
- six foreground click points are user-confirmed TEST PASS;
- route/mount/AutoFight/persistence features build in v1.5.1.

Broken:
- actual automatic map Confirm does not reliably trigger at the right time.

Current limitation:
- recent snapshot/orchestration had no authoritative real Confirm-popup presence field and therefore inferred from route/timing/stall.

## D. Investigation / Root Cause
Inspected:
- current CleanRoute source/knowledge;
- DATA repo `AI_INDEX.md`, `AI_BOOTSTRAP.md`, routing/runtime-map docs;
- user-supplied `ThanLongAutoTrain_Source_v0.9.0(2).zip` including real `src/main.cpp`.

### Direct donor-source finding
v0.9.0:
1. reads `MapReady` and `WaitingChangeMap`;
2. only when map state is stable calls top-level `MainFindUI("MessageBox")` and publishes `messageBoxVisible`;
3. cross-map Confirm gate additionally requires cross-map destination/route context, route progress stall/not-moving and cooldown;
4. after click, enters `awaitingMapTransition` and waits for real transition evidence.

Root Cause status:
- **CONFIRMED:** recent CleanRoute lacked authoritative Confirm UI existence proof.
- **CONFIRMED:** v0.9.0 used a dedicated semantic/existence `MessageBox` signal as part of the decision.
- **LIKELY:** missing semantic popup-presence evidence is a major cause of the recent automatic Confirm regression.
- exact end-to-end runtime cause remains **UNKNOWN until v1.5.2 live test**.

Evidence:
- `docs/evidence/EVID_V090_CROSS_MAP_CONFIRM_DETECTOR.md`.

## E. Changes Made
### `src/protocol.h`
Add `ValidConfirmUi` + `confirmUiVisible`; protocol v1.5.2.

### `src/bridge.cpp`
Add optional read-only semantic observer resolving `LuaSystemAPI_GUI` and preferring `MainFindUI(System.String)` for top-level `MessageBox`; `FindUI` is fallback only. If observer prerequisites fail, route core remains usable and `ValidConfirmUi` stays unset.

### `src/controller.cpp`
Cross-map click requires:
- route armed by this tool;
- actual route evidence;
- AutoPath stopped;
- authoritative moving state and a short stationary settle guard;
- `ValidConfirmUi=1`;
- `confirmUiVisible=1`.

Retry is allowed only while MessageBox remains visible. Timeout is debounce/re-arm only and can never establish dialog existence.

### Source-memory integration
Added:
- `clean-route/PROJECT_SUMMARY.md`;
- `clean-route/docs/DATA_REPO_USAGE.md`.

These travel with source so future AI knows DATA repo is external knowledge/evidence and the tool repo is the implementation/lineage source.

## F. Important Implementation Details
- UI name: `MessageBox`.
- preferred observer: `MainFindUI("MessageBox")`.
- fallback: `FindUI("MessageBox")`.
- detector validity bit is separate from visible value.
- `MessageBox ?` = observer unavailable; no automatic click.
- `MessageBox OFF` = authoritative absence; no click.
- `MessageBox ON` is necessary but still gated by tool-owned route evidence + stationary/AutoPath state.
- same saved Confirm coordinate and foreground input engine are preserved.
- donor timing constants are not copied as canonical proof; timing is subordinate to semantic UI state.

## G. Files / Components Changed
Modified:
- `src/protocol.h` / reconstructed Bridge and controller source;
- `.github/workflows/build-clean-route.yml`;
- `resources/app.rc`;
- `build.cmd`;
- `README.md`;
- `PROJECT_KNOWLEDGE.md`/`CHANGELOG.md` lineage already carries v1.5.2 state.

Added/updated:
- `clean-route/PROJECT_SUMMARY.md`;
- `docs/DATA_REPO_USAGE.md`;
- `docs/features/CROSS_MAP_CONFIRM.md`;
- `docs/evidence/EVID_V090_CROSS_MAP_CONFIRM_DETECTOR.md`;
- this version history.

## H. Build / CI History
Initial v1.5.2 CI attempts exposed packaging/reconstruction issues before compile; these are preserved as BUILD/CI history, not runtime evidence.
Corrections during CI: workflow YAML here-string parse issue fixed; corrupted one-file controller transport bypassed by reconstructing from the CI-proven v1.5.1 split transport; final Bridge/controller source reconstructed and verified before compile.
Final Build: **PASS** — Windows Server 2025 / Zig 0.15.2.
CI: **PASS** — run `31925186185`, job `95111532981`, head `6e95eded2805dc88b98a510311af36e9e95dd424`.
Artifact: `9257640088`, digest `sha256:b5b39f791782b447e69148816fe26da306ff8d660e6f45f32b0ccde8de489033`.
Hashes: EXE `89db246f00c00d8a006e700fa1352e4c455824f7b99bcf2601ed051457421e03`; Bridge `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`; source ZIP `c29485bcc91a392fcf5550bee559411a083b23e7ee244f897d4a9de6dc26760d`; controller source `6988504564366c6649992a6b855d46c5d48d555e876616bb574e996fe8dea3bd`; bridge source `a7f35862224e8174f36182569056871495b5575ca676e03726e8be287420c629`.

## I. Runtime Result
RUNTIME: UNTESTED

Confirmed Working:
- manual saved Confirm click coordinate from prior user test.

Still Failing:
- v1.5.1 actual automatic Confirm from user report.

Awaiting Test:
- v1.5.2 semantic `MessageBox` detector and automatic route sequence.

## J. Regression / Revert / Failed Attempts
Approach: timing/stall-driven confirmation as primary existence proof.
Result: manual click works but actual flow remains unreliable.
Why superseded: timing can show a route is stalled but cannot prove a specific popup exists.
Can retry: timing remains useful only as debounce/settle/failure guard together with semantic UI presence.

## K. Known-Good Established
None newly established by v1.5.2 yet. Runtime evidence is required.

## L. Remaining Bugs / New Knowledge / Decisions
- automatic cross-map Confirm remains open until user runtime test;
- external DATA repo rule files are mandatory startup memory every version/task;
- donor source is reference/evidence only, never a source transplant.

## M. Handoff
Inspect first in live log:
1. cross-map route armed;
2. AutoPath/route evidence;
3. stationary settle;
4. `MessageBox ON`;
5. saved-coordinate Confirm click;
6. `MessageBox OFF`;
7. `WaitingChangeMap` / MapReady transition;
8. destination MapID + ready position.

Do not change first:
- saved Confirm coordinate;
- foreground input click engine;
- unrelated AutoFight/AutoSell flows;
- current DATA repo research rules.