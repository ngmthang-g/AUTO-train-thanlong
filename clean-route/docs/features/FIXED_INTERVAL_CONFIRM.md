# Feature — Fixed Interval Map Confirm

Purpose: deterministic saved-coordinate Confirm click without semantic UI detection.

Scheduler:
`running + enabled + point valid + interval due + not busy -> RealInputClick(Confirm)`

The timer is not reset when blocked. Only a successful click (or a click failure that must be rate-limited) advances the timestamp.

Busy priority prevents mouse contention with higher-priority automatic click flows and transition/freeze recovery. AutoPath itself is intentionally not a blocker.
