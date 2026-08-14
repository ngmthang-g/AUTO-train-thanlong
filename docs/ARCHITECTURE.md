# NewCore Architecture

```text
Tool UI
  -> Session Manager
    -> ClientContext (per PID)
      -> Bridge snapshot (read)
      -> State Store
      -> Explicit FSM
      -> Safety Guard
      -> Action Queue (max active = 1)
      -> Main-thread Bridge
      -> Runtime-resolved IL2CPP / Lua action
      -> real game state
      -> next snapshot / POST verification
```

## Fail-closed

Unknown state is not an invitation to retry. Unknown state locks mutation until a fresh, safe snapshot is available.

## Main-thread proof

The bridge is injected with a thread-specific `WH_GETMESSAGE` hook, not a worker thread. It then verifies:

1. The executing TID equals the target game-window TID; and
2. `SynchronizationContext.Current` resolves to `UnitySynchronizationContext`.

There is deliberately **no fallback inference** from `FGStudio.Engine.Utilities.MainThread`, window ownership, or a hardcoded TID. If strict proof fails, mutation is blocked.

## Donor policy

v0.9.0 is reference-only for:

- API/method names
- known Lua UI/function names
- data offsets and enum semantics
- NPC/item knowledge
- server ACK behavior

Do not copy its `RemoteExecutor`, `remote_worker.S`, monolithic Worker state flags, fixed-delay transitions, or long-lived UI pointers into NewCore.
