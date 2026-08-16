# Death Session Cold Restart

Purpose: make every death/revive cycle start with the same runtime conditions as pressing `BẮT ĐẦU`, without losing per-account configuration.

The lifecycle latch is stored on `Account`; all transient automation state remains in `RuntimeState`. Full `ResetRuntime()` is executed on both the DEAD edge and the subsequent ALIVE edge. This deliberately removes prior-life map qualification, route ownership, Confirm debounce, fight phases, train watchdogs, recovery/travel phases, sell phases and freeze timing instead of trying to enumerate individual reset fields.

A temporary invalid LifeState while the death session is latched is fail-closed: automation waits and preserves the latch rather than returning to normal actions.
