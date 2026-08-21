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
MessageBox UI probing is removed from ReadState. Core snapshot latency no longer depends on this modal UI lookup.

## Build status
Windows x64 staging CI **PASS** — run `31942843638`, job `95154396882`.
- audit PASS;
- route/mount self-test 15/15 PASS;
- Bridge PE/load/export PASS;
- EXE/package verification PASS;
- compile controller SHA256 `01cafd0a490a140da10db5a92ac23f7b70d794f88cb4936f002f07962dfb0ca0`;
- Bridge source SHA256 `64967db0d72cd584c909ce12dd4fdf20cace1128784b9e43214bd419014ed05f`.

Runtime status: **NEEDS USER TEST**. BUILD PASS is not RUNTIME PASS.
