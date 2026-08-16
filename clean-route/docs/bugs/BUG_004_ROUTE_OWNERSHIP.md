# BUG-004 — repeated death loses cross-map route ownership

**Status:** FIX CANDIDATE v1.5.7, runtime unverified.

## Observed
First cross-map Confirm works. After a later death/revive, automatic Confirm can stop appearing. Manual STOP → START restores Confirm immediately.

## Root Cause From Source Comparison
Manual STOP sends game-side `StopAutoPath`, while death cold restart only calls `ResetRuntime`. If the client preserves `autoPathing=1` across revive, controller ownership flags are cleared but route logic waits on the existing Path ON and never issues a fresh StartPath. Therefore `crossMapRouteArmed` stays false and semantic Confirm intentionally refuses MessageBox.

## Fix Candidate
v1.5.7 Route Ownership Reacquire forces stale AutoPath OFF and verifies it before allowing a fresh route after Start/revive.

## Runtime Boundary
BUILD PASS will not prove the bug fixed. Must test at least three consecutive death/revive/return cycles.
