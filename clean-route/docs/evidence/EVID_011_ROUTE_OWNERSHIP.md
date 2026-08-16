# EVID-011 — manual STOP→START isolates game-side AutoPath ownership

Runtime report: first Confirm works, later death/revive may miss Confirm, but manual STOP then START restores it.

Source comparison shows `StopAccount()` calls Bridge StopPath, while post-revive `ResetRuntime()` does not touch the game client's real AutoPath. Route logic returns Wait for an already-ON AutoPath, and `SendDecision(StartPath)` arms `crossMapRouteArmed` only on a successful fresh cross-map StartPath.

This creates a direct causal path for the repeated-death deadlock:
`ResetRuntime → ownership flags cleared → stale AutoPath still ON → route waits → no fresh StartPath → crossMapRouteArmed=false → semantic Confirm fails closed`.

This evidence motivates v1.5.7 Route Ownership Reacquire. Runtime verification is still required.
