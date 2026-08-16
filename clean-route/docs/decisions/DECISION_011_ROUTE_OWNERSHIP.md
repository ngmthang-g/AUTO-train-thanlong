# DECISION-011 — Route ownership must be reacquired after any controller cold reset

**Status:** ACTIVE candidate for v1.5.7 pending runtime validation.

A controller `RuntimeState` reset is not sufficient to claim ownership of a game-side AutoPath that may survive death/revive. Fresh Start and post-revive cold-start must force/verify AutoPath OFF first; only a subsequent tool-issued StartPath may arm semantic cross-map Confirm ownership.

Do not weaken MessageBox unrelated-dialog guards to compensate for stale path ownership. If the ownership reset cannot prove AutoPath OFF after bounded retries, fail closed instead of continuing route/Confirm actions.
