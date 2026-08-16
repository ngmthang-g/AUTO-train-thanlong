# Route Ownership Reacquire — v1.5.7

## Problem
`ResetRuntime()` clears controller ownership flags but does not stop the game client's real AutoPath. After death/revive the client may still report AutoPath ON. `Decide()` then waits instead of issuing a fresh StartPath. Because semantic Confirm requires `crossMapRouteArmed`, the popup is ignored even when visible.

## Evidence
Manual STOP→START restores Confirm. STOP calls game-side StopPath; post-revive cold reset did not.

## Flow
Start/post-revive → require ValidAutoPath → if ON, StopPath (max 3, 1200 ms spacing) → verify OFF → clear route/Confirm ownership → one-cycle barrier → M87 guard → mount/route → fresh StartPath arms ownership → MessageBox Confirm.

## Safety
Do not infer ownership from a pre-existing AutoPath. Do not weaken MessageBox unrelated-dialog guard.
