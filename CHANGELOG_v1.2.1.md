# v1.2.1 - Revive Dispatcher Anti-Diss Hotfix

- Runtime v1.2.0 proved foundation/scanner/MainThread/queue but timed out inside first `InvokeReviveButton`; client disconnected/unresponsive in same death episode.
- Do not claim an exact v1.2.0 sub-step caused the failure: there was no internal breadcrumb.
- Protocol bumped to `0x00010201`.
- Command 7 stays Revive-only; AutoFight/NPC/Sell/Path stay locked.
- Revive UI enumeration reduced from potentially 32768 synchronous entries to a **120ms / 4096-entry hard bounded scan**.
- Hotfix uses exact visible label matching only to reduce managed calls.
- Direct `runtime_invoke(handleClick,...)` is now forbidden.
- Fresh UIButton `HandleClickEvent()` is bound to a managed `System.Action` via `Delegate.CreateDelegate(Type,Object,MethodInfo)`.
- The Action is queued through `FGStudio.Engine.Utilities.MainThread.Execute(System.Action)` and executes from the game's Update dispatcher instead of synchronously inside WH_GETMESSAGE.
- Revive metadata gate additionally requires current-client MainThread dispatcher support.
- Dispatch ACK requires `queued=1`, `directInvoked=0`, matching token/thread/identity/PRE.
- Added live shared-memory progress stages 1..11 with object/button/time counters for precise timeout diagnostics.
- Real success remains observer `dead=1→0` for two stable snapshots; MapID may change, RoleID may not.
- ActionQueue stays MAX=1 and no action retry is allowed in the same death episode.
- Revive self-test increased from 16 to **18 checks**.