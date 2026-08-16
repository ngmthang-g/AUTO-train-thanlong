# FEATURE: UNDERWORLD RECOVERY (ĐỊA PHỦ)

## Purpose
Make post-revive return routing deterministic by giving Địa Phủ a high-priority recovery ownership gate.

## Exact map identity
Canonical DATA: `database/MAPS.csv` → `87,Địa Phủ,siwang,0,Wild,1,siwang`.

## Current implementation — v1.5.5
When fresh snapshot says MapID 87:
1. require authoritative `ValidAutoFight`;
2. if AutoFight OFF, release normal route;
3. if ON, click saved `DỪNG AUTO 1`;
4. wait 700 ms;
5. click saved `DỪNG AUTO 2`;
6. verify fresh AutoFight getter;
7. if still ON after 1200 ms, retry;
8. max 3 full stop sequences, then hold for manual intervention.

## Priority
Runs after death handling but before normal map qualification, Auto Sell, recovery, cross-map Confirm, AutoFight watchdog and route FSM. Existing Transition Freeze is still above mutable click dispatch, so M87 clicks cannot fire while the client is unstable.

## Safety / proof
- invalid AutoFight getter: fail closed;
- missing StopAuto coordinates: fail closed;
- success is proof by fresh AutoFight OFF, not by elapsed time;
- no infinite retry loop.

## Relationship to repeated death
Each new death resets prior route/Confirm generation and M87 guard state. This prevents death #2 from inheriting stale route ownership/retry/debounce state from death #1.

## Runtime status
BUILD/CI: **PASS** — Windows run `31934654526`, job `95134583587`, controller SHA256 `73451f4fbc244c9b05ef3f86ff3daeb445b3e4bffd0be6e534bc514cb0759d8e`.
RUNTIME: NEEDS USER TEST with at least two consecutive death/revive/return cycles.
