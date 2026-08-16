## Current candidate: v1.5.6 Death Session Cold Restart
Repeated deaths are now isolated by a full dual-boundary `ResetRuntime()` with an Account-level lifecycle latch. Every post-revive route starts from the same transient state as a fresh Start while keeping account configuration. Runtime validation pending.

# CLEAN ROUTE — PROJECT SUMMARY

## Current version
**v1.5.6 — Death Session Cold Restart**

## Runtime evidence driving this version
- v1.5.4 first tested return path: semantic `MessageBox=1` while Path ON → StopPath → saved Confirm click → Transition Freeze; this gate worked.
- v1.5.4 freeze logs show map/Bridge instability is held until the client is stable continuously for 2 seconds.
- on a later/second death cycle, the character routed back out but automatic Confirm was no longer observed. Exact causal chain remains unproven.

## Verified map identity
Canonical DATA repo `database/MAPS.csv` contains:
`87,Địa Phủ,siwang,0,Wild,1,siwang`

Therefore **Địa Phủ = MapID 87**.

## v1.5.5 behavior
1. New death (`get_IsDeath=1` new cycle) resets all route/Confirm ownership, movement evidence, stall/debounce/retry timing and M87 recovery state.
2. After revive, if fresh state says `MapID==87`, the M87 guard owns the PID before normal route logic.
3. If `ValidAutoFight` is unavailable, fail closed.
4. If AutoFight ON: saved `DỪNG AUTO 1` → 700 ms → `DỪNG AUTO 2` → verify getter.
5. If still ON after 1200 ms, retry, maximum 3 sequences.
6. Only verified AutoFight OFF releases the normal route/Confirm FSM.
7. v1.5.4 Transition Freeze and v1.5.3 semantic Path-ON Confirm remain intact.

## Architecture
`Resolver → read-only Snapshot → Observer → per-PID ownership/State Machine → Safety Guard → serialized action → fresh proof`

Hard rules:
- no overlapping mutable requests/actions;
- timeout/invalid getter = unknown, never success;
- no blind M87 stop clicks if AutoFight getter is invalid;
- no route while M87 AutoFight is authoritatively ON;
- do not restore CreateRemoteThread/remote_worker/WriteProcessMemory gameplay architecture.

## Status
- MapID 87 = Địa Phủ: **VERIFIED static client data**.
- v1.5.4 first Confirm/freeze behavior: **RUNTIME PARTIAL PASS**.
- second-death Confirm miss: **RUNTIME FAIL/PARTIAL observed**.
- v1.5.5 source: **IMPLEMENTED**.
- v1.5.5 Windows code-validation CI: **PASS** — run `31934654526`, job `95134583587`, artifact `9260311819`.
- v1.5.5 runtime: **NEEDS USER TEST**.
- v1.5.6 dual-boundary full RuntimeState reset: **WINDOWS CI PASS / RUNTIME UNTESTED** — run `31938154271`, job `95143227671`, artifact `9261270126`.
