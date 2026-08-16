# VERSION v1.5.8 — Fixed Interval Confirm Click

## User decision
Stop trying to infer when the map-confirm popup exists. Use a deterministic periodic click at the already captured Confirm coordinate.

## Behavior
- checkbox `Auto XN map`;
- interval edit, default 5s, 1–300s, per-account persistent;
- first click occurs after one full interval after Start/cold reset;
- if due while blocked, it stays due and fires on the first safe tick;
- no MessageBox/route/movement evidence is required.

## Yield conditions
Pause periodic Confirm during map transition, client/Bridge freeze/unresponsive state, death/revive, M87 stop-Auto, route ownership reset, active AutoFight click sequence, sell click/UI phases, recovery click phases, or within 1000ms of any REAL INPUT across all accounts. Ordinary AutoPath travel is allowed.

## Bridge
Remove MessageBox UI probing from ReadState. This is both unnecessary for v1.5.8 and avoids making core snapshot latency depend on modal UI lookup.

## Status
Local self-test 15/15 PASS. Windows CI and runtime pending.
