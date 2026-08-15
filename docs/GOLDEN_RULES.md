# NewCore Golden Rules

These rules are architectural invariants. A feature is not allowed to bypass them for convenience.

1. **Scanner is read-only.** It may observe and publish `GameSnapshot`; it may never mutate game state or directly trigger an action.
2. **One client = one isolated context.** Resolver, scanner, state store, observer, FSM, safety guard, action queue and runtime references are process-local. Only static databases may be shared.
3. **One current FSM state per client.** Do not recreate the legacy web of overlapping trip/recovery booleans.
4. **All Unity/game mutations go through a proven Unity main-thread dispatcher.** Never revive, click, move, sell, fight, or invoke Unity gameplay from the scanner/worker thread.
5. **ActionQueue max active mutation = 1.** No subsystem may independently fire a concurrent game action.
6. **Every action has PRE and POST conditions.** Flow is `READ -> DECIDE -> VALIDATE -> ACTION -> WAIT REAL STATE CHANGE -> VERIFY -> NEXT STATE`.
7. **Fail closed on unknown/unstable state.** Lock actions, rescan and recover to a known state; never spam retries or try alternate clicks blindly.
8. **Do not cache UI pointers across UI/state transitions.** Reacquire the current object/action when the relevant UI is present, invoke once, then discard the reference.
9. **IL2CPP metadata/runtime resolution is the primary mechanism.** Verified legacy RVA/offset/signature data may be used as donor evidence or validation, not as the core gameplay execution architecture.
10. **Prefer the game's normal internal flow and real ACK/state response.** Observer/server/UI changes prove success; sleeps are not state transitions.
11. **Map transitions are hard scanner boundaries.** Check map-ready/change-map state before touching deeper character/AutoPath state and require stable recovery before declaring the observer healthy.
12. **Watchdog faults become SAFE PAUSE / action lock.** A stalled scanner, dead dispatcher, hung action, unexpected map change or queue invariant violation must not turn into retry spam.
13. **Phase gates are mandatory.** A later feature phase is not added until the previous architectural layer has passed runtime testing.

## Legacy donor definition

`Legacy v0.9.0` means the **old pre-NewCore automation source/architecture**. It is not a NewCore version number.

It may donate verified knowledge such as namespace/class/method names, semantics, offsets/RVAs for comparison, NPC/map/item data, Lua/UI names, callback/ACK discoveries and already-proven read logic.

It must not donate the old execution architecture: `RemoteExecutor`, remote gameplay worker, `CreateRemoteThread` action execution, giant Worker flow, sleep-driven transitions, long-lived UI pointers, scanner-triggered mutations, or competing subsystems that fire actions directly.
