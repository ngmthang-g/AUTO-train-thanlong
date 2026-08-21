# Auto Rotate Train Spots

Clean Route v1.5.9 adds a per-account rotation pool over the existing shared train-spot database (`ThanLongCleanRoute.spots.tsv`). A rotation item is a full saved train spot (`Name + MapID + X + Y`), not only a MapID.

## Selection
- The main UI shows all shared train spots in a checkbox ListView.
- Checked rows form that account's rotation pool.
- Rotation order follows the shared-spot list order.
- If exactly one spot is checked, automatic rotation is effectively disabled and the account remains on that spot.
- Legacy profiles automatically migrate their existing `SelectedSpot` into a one-item rotation pool.

## Trigger 1 — repeated deaths
Per account, each authoritative new `get_IsDeath=1` death edge is recorded in a rolling time window.

Defaults:
- limit: 10 deaths;
- rolling window: 10 minutes;
- trigger semantics: **more than** the configured limit, therefore default 10 triggers on the 11th death still inside the 10-minute window.

When triggered, the current target is replaced with the next checked train spot. Rotation metrics are reset for the new spot. Because the character is already dead, the normal revive/cold-start route resumes toward the new target after revival.

## Trigger 2 — bag does not become full
Default: 15 minutes.

The timer counts only **productive train time** when all of these are authoritative:
- character alive;
- current Map/X/Y readable;
- at the configured train target within tolerance;
- AutoFight ON;
- FreeBagSpace readable.

Travel time, map loading, death/revive, F4 pause, bridge/client freeze and time away from the train target are not accumulated.

A transition into `FreeBagSpace <= 0` is treated as a FULL-bag event and resets the no-FULL productive timer. After the bag gains free slots and training resumes, a fresh interval is accumulated. If the configured productive interval expires without another FULL event, the next rotation spot is selected.

Before leaving a live train spot, the existing train-recovery flow is used so AutoFight is stopped with the saved `DỪNG AUTO 1/2` clicks before pathing to the next target.

## Persistence
Per RoleID/account INI keys:
- `RotationCount`
- `RotationSpot_0..N`
- `RotateDeathLimit`
- `RotateDeathWindowMin`
- `RotateNoFullBagMin`

The shared spot database itself remains global across accounts.
