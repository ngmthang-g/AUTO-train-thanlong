# Revive / Đầu thai Action Evidence - NewCore v1.2.0

## Evidence chain

v1.2.0 opens the first real gameplay mutation only after these prior NewCore gates have live evidence:

1. Scanner/Observer stable qualification and map-transition recovery.
2. Unity main-thread identity proof.
3. FSM + SafetyGuard + ActionQueue(MAX=1).
4. Harmless hook/main-thread envelope with real token/sequence/thread POST.

Legacy v0.9.0 (pre-NewCore) remains donor-only. Its executor/RVA/thread architecture is not reused.

## Donor semantics retained

Legacy analysis established these semantic facts:

- death truth source is `LuaLeaderData.get_IsDeath()`;
- revive UI uses `FGStudio.LuaSystem.Base.UIObject.instances`;
- relevant control type is `FGStudio.LuaSystem.GUI.UIButton` or subclass;
- safe candidate requires active + interactable;
- label sources include UIButton Name/Text, with **Đầu thai** preferred over Hồi sinh/Revive/Relive/Respawn;
- final action is instance `UIButton.HandleClickEvent()`;
- stale UI pointers across death-overlay/map changes are unsafe;
- uncertain or duplicate candidates must not be clicked blindly.

Only those semantics are retained. Donor RVA/address execution is forbidden.

## Current-client metadata evidence

The v1.1.0 metadata-only action probe must report `kReviveMetadataSupportMask` ready before the controller permits Revive. Required surface includes:

- UIObject type + static `instances` field;
- active/children surface;
- UIButton type;
- UIButton interactable/text getters;
- `UIButton.HandleClickEvent()` method.

Presence of metadata alone is not permission to call the method. v1.2.0 additionally requires the proven hook/main-thread envelope and all runtime PRE conditions.

## Fresh runtime resolver

`src/bridge/revive_action_engine.inc` resolves every runtime target inside the single command 7 callback:

1. Re-run `ReadGameSnapshot()` on the proven Unity main thread.
2. Require expected RoleID + MapID, `dead=1`, `MapReady=1`, `WaitingChangeMap=0`.
3. Resolve current Assembly-CSharp UIObject/UIButton metadata.
4. Resolve `UIObject.instances` and read its static value with optional `il2cpp_field_static_get_value`.
5. Enumerate the current Dictionary Values through managed read-only getter/enumerator calls.
6. Filter UIButton/subclasses that are active + interactable.
7. Normalize Name/Text for Vietnamese matching.
8. Rank exact `Đầu thai` highest; reject ties.
9. Re-read game PRE and revalidate the selected UIButton immediately before the callback.
10. Call `UIButton.HandleClickEvent()` exactly once.
11. Discard the pointer when command 7 returns.

No runtime UIButton pointer is stored in SharedBlock, controller state or across map transitions.

## Exclusive mutation gate

Global `kGameplayMutationEnabled=false` remains unchanged. v1.2.0 introduces separate `kReviveMutationEnabled=true` solely for command 7.

CI rejects Revive source if it contains any of:

- ClickNPC
- StartAutoFight
- RequestSellItem
- StartPath / StopPath
- HandlePointerClick
- CreateRemoteThread
- il2cpp_thread_attach
- Sleep-driven workflow

CI also counts direct `g_api.runtime_invoke(handleClick, ...)` and requires exactly one occurrence.

## PRE

Controller requires:

- same current client/session;
- bridge attached;
- current-session harmless hook/main-thread proof PASS;
- scanner healthy + 60/60 qualified;
- observer stable;
- no active map transition;
- RoleID/MapID match intent;
- `dead=1` stable for two consecutive full snapshots;
- current-client Revive metadata capability ready;
- ActionQueue empty before enqueue.

## Dispatch ACK

After command 7 returns, queue remains active. Controller verifies:

- token matches intent;
- callback actually invoked;
- callback TID matches proven hook TID;
- managed current/main IDs match the proven Unity main thread;
- pre RoleID + pre MapID match intent;
- preDead=1.

Mismatch is fatal fail-closed. Resolver/action failure before a verified callback cancels the queue and blocks further Revive attempts for that same death episode; there is no retry spam.

## Real POST

`HandleClickEvent()` returning is not success. Success is observer evidence.

Revive may move the character to another MapID, therefore MapID is not required to stay equal after the callback. RoleID must stay equal.

If map transition occurs, active Revive is preserved only as an outstanding POST wait. Every new mutation remains locked.

After transition/recovery, controller requires two consecutive stable snapshots satisfying:

- same RoleID as PRE;
- `dead=0`;
- `MapReady=1`;
- `WaitingChangeMap=0`.

Only after the second stable alive snapshot does `ActionQueue.CompleteActive()` return occupancy to zero.

## Failure policy

- UI candidate absent/ambiguous: block this death episode, no blind click.
- Scanner failure while waiting POST: fail closed, no callback retry.
- RoleID drift: fail closed.
- Dispatch token/thread/PRE mismatch: fail closed.
- POST timeout 30 seconds: fail closed, no callback retry.

This milestone proves Revive only. AutoFight, NPC, Sell, Path, Treatment and Buff remain outside the action engine.
