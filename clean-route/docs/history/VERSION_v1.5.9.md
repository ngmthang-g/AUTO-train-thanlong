# VERSION v1.5.9 — Auto Rotate Train Spots

## Requested behavior
- If an account dies more than 10 times inside 10 minutes, immediately move its train target to the next selected train spot.
- Allow 1, 2, 3, 4, 5, 6 or more selected spots from the existing shared data.
- One selected spot means stay there.
- Multiple selected spots rotate continuously and wrap back to spot 1.
- Also rotate when productive training reaches 15 minutes without a FULL-bag event.

## Implementation
- Added per-account checkbox rotation pool over shared train spots.
- Added persisted, configurable death limit, death rolling window and no-FULL-bag productive minutes.
- Death history and productive-train counters live outside RuntimeState so the v1.5.6 death-session hard resets do not erase rotation evidence.
- Death trigger is recorded only on a new authoritative death edge.
- No-FULL-bag timer accumulates only while alive, at the current target, AutoFight ON and FreeBagSpace authoritative.
- A new FULL-bag edge resets the productive no-FULL timer.
- Live efficiency rotation reuses `BeginTrainRecovery` to stop combat before traveling to the new spot.
- Death-triggered rotation changes the target before revive; normal post-revive cold-start routing then goes to the new target.
- v1.5.8 periodic saved-coordinate Confirm remains unchanged.

## Validation
Local pure rotation test: 8 cases prepared. Windows x64 CI pending.
