# Clean Route v1.5.9 — Auto Rotate Train Spots audit

## Preserved v1.5.8 guarantees
- Saved Confirm coordinate remains per-account and persistent.
- `ConfirmIntervalSec`: default 5s, range 1–300s.
- Automatic Confirm does not invoke semantic `HandleCrossMapConfirm`.
- Bridge ReadState does not call `ReadSemanticUiPresent` or `MainFindUI`.
- Transition/WaitingChangeMap/Freeze/read failure and higher-priority click phases still block periodic Confirm.
- No direct item-sell/combat mutation API added.

## v1.5.9 rotation guarantees
- Rotation pool contains existing shared train spots (`Name + MapID + X + Y`).
- Rotation pool and thresholds are per RoleID/account and persistent.
- Legacy single-spot profiles migrate to a one-item pool.
- One checked spot never changes target because of a rotation trigger.
- Multiple checked spots advance round-robin and wrap back to the first.
- Default death trigger: count becomes greater than 10 inside a rolling 10-minute window.
- A death is counted only once per authoritative new death session.
- Death history is outside RuntimeState and survives death/alive cold resets until a spot switch.
- Default efficiency trigger: 15 minutes of productive train time without a new FULL-bag edge.
- Productive train time requires alive + at current target + AutoFight ON + authoritative FreeBagSpace.
- Travel, transition, freeze, death/revive and F4 pause are not intentionally accumulated.
- A new `FreeBagSpace <= 0` edge resets the no-FULL productive timer.
- Live efficiency rotation uses existing train recovery to stop AutoFight before route.
- Death-triggered rotation changes target before normal revive/cold-start routing resumes.
- Switching to a new rotation spot resets both rotation trigger windows for the new spot.

## Validation
Windows x64 staging CI **PASS** — run `31949891312`, job `95171640279`, artifact `9264382136`.
- CleanRoute static/safety audit PASS.
- Existing route/mount self-test: **15/15 PASS**.
- Rotation self-test: **8/8 PASS**.
- Bridge PE verification, EXE build, package and artifact verification PASS.
- Runtime rotation still requires real-client testing.
