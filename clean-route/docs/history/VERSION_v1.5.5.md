# VERSION v1.5.5 — 2026-08-16

## A. Identity / Lineage
Version: v1.5.5
Based On: v1.5.4 Transition Freeze
Reason Created: first death/return Confirm worked, but a later/second death cycle routed back without observed automatic Confirm.
Related: BUG-003, EVID-007, EVID-008, EVID-009.

## B. User Request
- determine exact Địa Phủ MapID;
- if character is in Địa Phủ and AutoFight is ON, immediately stop it using existing two saved clicks, then continue auto train;
- preserve reliable Confirm/freeze behavior while fixing repeated-death recovery.

## C. State Before Modification
- v1.5.4 first tested semantic Confirm + post-click Freeze: runtime PASS for that gate;
- v1.5.4 stable-resume after timeout/loading: runtime PASS for tested log;
- second death cycle: automatic Confirm not observed.

## D. Investigation / Root Cause
VERIFIED static data: `database/MAPS.csv` maps Địa Phủ to MapID 87.

CONFIRMED source gaps:
1. fresh death did not explicitly reset the complete cross-map/Confirm generation immediately at new death start;
2. no dedicated M87 arbitration forced built-in AutoFight OFF before post-revive route.

LIKELY, not confirmed: AutoFight may compete with post-revive route/movement and prevent the same portal stall/MessageBox timing seen on the successful first cycle.

## E. Changes Made
### Fresh death generation
On new death reset route ownership/evidence, stall state, Confirm attempts/debounce/StopPath timing, suppression and M87 guard state.

### M87 AutoFight guard
- runs before normal map qualification/route/Confirm/sell/recovery logic;
- invalid AutoFight getter → wait;
- ON → StopAuto1 → 700 ms → StopAuto2 → verify;
- still ON after 1200 ms → retry;
- maximum 3 full attempts;
- OFF → release normal route.

## F. Important Implementation Details
- `kUnderworldMapId = 87`
- `kUnderworldStopStepMs = 700`
- `kUnderworldVerifyMs = 1200`
- `kUnderworldMaxStopAttempts = 3`
- no new direct combat mutation API; existing foreground saved clicks are used.

## G. Files / Components Changed
- controller source patch;
- build/version resources/workflow;
- project knowledge, changelog, bug/evidence/feature docs.

## H. Build / CI
**PASS** — Windows code-validation run `31934654526`, job `95134583587`, artifact `9260311819`, digest `sha256:50a034a9896c9475ac3d4551837a4b01a4ef3c366dad6ff4cc81ffe386bb7d9f`. Controller SHA256 `73451f4fbc244c9b05ef3f86ff3daeb445b3e4bffd0be6e534bc514cb0759d8e`; EXE SHA256 `bc8fbe8e6b40dda4e84590de065cfc9f898d767aed85f1a1ed376a7c7bbc5456`; Bridge SHA256 `81cfafa953a6f7b8a9c95965f79d618b66dd5a28c02a8ac771c08e0a6729eade`. Audit and route/mount self-test 15/15 PASS.

## I. Runtime Result
RUNTIME: UNTESTED.

Desired two-death trace:
`NEW DEATH CYCLE → revive → M87 → AutoFight ON → Stop1 → Stop2 → verify OFF → route → MessageBox ON → Confirm → Freeze → stable resume`.

## M. Handoff
If second-cycle Confirm still fails, inspect logs beginning at `NEW DEATH CYCLE` and compare: MapID, AutoFight validity/value, StartPath, crossMap arm/evidence, moving, MessageBox validity/value, StopPath and Freeze. Do not change Confirm click coordinates first.
