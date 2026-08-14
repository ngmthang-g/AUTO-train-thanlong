# Verification status

## Static/source verification performed

- No `CreateRemoteThread`, `remote_worker.S`, or `il2cpp_thread_attach` exists in the NewCore action path.
- Controller serializes mutation through one per-client `ActionQueue`.
- SAFE PAUSE clears only queued-not-sent actions; an already-sent action is still drained/ACKed.
- Mutation commands are blocked unless the bridge proves `UnitySynchronizationContext` on the target window thread.
- Runtime IL2CPP class/method lookup is used for the migrated actions; hardcoded gameplay RVAs are not used.
- Managed strings, boxed values, UI objects and argument arrays used across nested invokes are GC-rooted.
- `Object[]` population uses reflected `System.Array.SetValue`; no hardcoded IL2CPP array-data offset.
- `UNICODE/_UNICODE` is forced in the Windows build to keep ListView W APIs consistent.

## Runtime verification that cannot be performed in this environment

This environment cannot launch the user's Windows game client. Therefore it cannot prove that the game's window thread exposes `UnitySynchronizationContext`, nor can it measure real disconnect/crash rate. The bridge intentionally fails closed if that proof is absent.

Advanced donor features (sell/heal/revive/buff/path/mount) are not enabled in this foundation until the main-thread bridge is validated in the real client. This is intentional: migrating them before validating the execution layer would reintroduce the exact class of instability NewCore is meant to remove.
