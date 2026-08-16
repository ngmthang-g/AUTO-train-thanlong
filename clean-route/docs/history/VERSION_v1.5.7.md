# VERSION v1.5.7 — Route Ownership Reacquire

## Problem Reported
First Confirm works; later death/revive misses automatic Confirm; manual STOP→START restores it.

## Investigation
Comparing the two paths showed that manual STOP calls the Bridge `StopPath` action, while post-revive `ResetRuntime()` only resets controller state. If the client preserves `autoPathing=1` across revive, route logic sees an already-running path and waits instead of emitting a fresh StartPath.

## Root Cause
The controller loses `crossMapRouteArmed` during cold reset but the client may keep the previous AutoPath alive. Because `crossMapRouteArmed` is armed only after a successful tool-issued cross-map StartPath, stale Path ON can create a deadlock: route logic waits, no new StartPath is issued, and semantic Confirm fails closed even if MessageBox appears.

## Solution
Add a session Route Ownership Reset gate after Start and post-revive: require authoritative AutoPath state; StopPath stale AutoPath if ON; verify OFF; clear route/Confirm ownership; then allow normal M87 guard/mount/route. Only the next fresh tool StartPath may own the cross-map route.

## Safety
Do not weaken MessageBox unrelated-dialog protection. Do not treat a pre-existing game AutoPath as tool-owned. Maximum StopPath attempts: 3, spaced 1200 ms, then fail closed.

## Test Status
Static/source patch validation complete. Windows CI and repeated-death runtime test pending until this version is built.
