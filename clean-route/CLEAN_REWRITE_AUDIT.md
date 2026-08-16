## v1.5.6 death-session audit
- Full RuntimeState reset on DEAD edge: REQUIRED.
- Full RuntimeState reset on ALIVE edge: REQUIRED.
- Account-level external latch: REQUIRED; must not be stored in RuntimeState.
- Settings/profile persistence across reset: REQUIRED.
- Existing M87/Confirm/Freeze safety preserved.
- Windows CI PASS: run `31938154271`, job `95143227671`; exact final controller SHA256 `840bc0376961d2838e1ca46598c43b07d91ccec846f375efece8f2ae569b6711`; 15/15 route self-test PASS. Runtime repeated-death behavior remains UNTESTED.

# Clean Route v1.5.5 — Địa Phủ AutoFight Guard audit

## Scope preserved
- v1.5.4 per-PID Transition Freeze and Bridge `pendingSeq` serialization remain.
- v1.5.3 semantic Path-ON MessageBox Confirm remains.
- six saved click points, 60s AutoFight watchdog, 180s train-position guard, robust travel, Auto Sell, persistence and timer remain.

## New exact client fact
Canonical DATA `database/MAPS.csv`: `87,Địa Phủ,siwang,0,Wild,1,siwang`.

## New death-generation contract
A fresh dead transition resets prior-life cross-map ownership, route movement evidence, stall timer, Confirm attempts/debounce/StopPath timing, suppression and M87 guard state.

## M87 action gate
Before normal route/Confirm arbitration on Map 87:
- require authoritative `ValidAutoFight`;
- AutoFight OFF → release guard;
- AutoFight ON → saved StopAuto1, wait 700ms, saved StopAuto2, verify getter;
- still ON after 1200ms → retry; maximum 3 full sequences;
- missing stop coordinates or invalid getter → fail closed.

## Safety
- user-captured clicks only; no new unverified game mutation API;
- existing Freeze still blocks click dispatch during map/client instability;
- no route is allowed while M87 AutoFight is authoritatively ON;
- no infinite click loop; max 3 sequences then manual hold.

## Evidence boundary
- v1.5.4 first Confirm/freeze path is runtime-supported.
- second-death automatic Confirm miss is user-runtime evidence.
- missing explicit new-death cross-map reset and missing dedicated M87 AutoFight guard are confirmed source gaps.
- AutoFight competition as the exact cause of the second-death miss is LIKELY, not CONFIRMED.

## Validation
- source audit markers: `kUnderworldMapId = 87`, `HandleUnderworldAutoFightGuard`, `NEW DEATH CYCLE`, max 3 stop attempts.
- v1.5.5 Windows code-validation CI: **PASS** — run `31934654526`, job `95134583587`; audit + route self-test 15/15 + Bridge/EXE/package verification passed.
- v1.5.5 two-consecutive-death runtime: **NEEDS USER TEST**.
