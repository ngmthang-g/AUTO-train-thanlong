# Revive / Đầu thai Action Evidence - NewCore v1.2.1

## Evidence chain

Inherited live evidence:
1. Scanner/Observer stable 60/60 and map-transition recovery.
2. Unity main-thread identity proof.
3. FSM + SafetyGuard + ActionQueue(MAX=1).
4. Harmless WH_GETMESSAGE action envelope with token/thread POST.
5. v1.2.0 live death truth `dead=0→1` and Revive PRE/queue admission.

Legacy v0.9.0 (pre-NewCore) remains donor-only; no donor executor/RVA/thread architecture is reused.

## v1.2.0 negative runtime evidence

PID 23432 reached `InvokeReviveButton` after all inherited gates passed, then command 7 failed to return within 5 seconds and the client disconnected/became unusable. Controller failed closed and did not retry.

Because v1.2.0 did not publish internal command progress, evidence cannot identify whether the stall occurred in UI enumeration, revalidation, or the direct `HandleClickEvent()` call. The design problem is therefore treated as the synchronous command boundary itself, not one guessed line.

v1.2.1 removes the two risky properties:
- potentially long managed UI traversal in the hook is hard-bounded;
- `HandleClickEvent()` is no longer directly invoked from WH_GETMESSAGE.

## Donor semantics retained

- death truth source: `LeaderRoleData.get_IsDeath()`;
- UI registry: `FGStudio.LuaSystem.Base.UIObject.instances`;
- control type: `FGStudio.LuaSystem.GUI.UIButton`/subclass;
- candidate must be active + interactable;
- visible labels include `Đầu thai` / `Hồi sinh`;
- action semantic is `UIButton.HandleClickEvent()`;
- stale UI pointers and blind retries are unsafe.

## Current-client metadata evidence

Controller requires both:
- `kReviveMetadataSupportMask`;
- `kMainThreadDispatcherMetadataSupportMask`.

Observed current-client dispatcher surface:
- `FGStudio.Engine.Utilities.MainThread.get_Instance()`;
- `MainThread.Execute(System.Action)`;
- queue field `ConcurrentQueue<System.Action> waitToBeProcess`.

Metadata is capability evidence only; runtime PRE + ActionQueue + real POST are still mandatory.

## Bounded fresh resolver

`revive_action_engine.inc`:
1. fresh PRE snapshot;
2. expected RoleID/MapID + dead=1 + map stable;
3. fresh `UIObject.instances`;
4. enumerate current values with 120ms hard budget / 4096-entry cap;
5. class-filter UIButton before expensive getter calls;
6. exact visible text matching only;
7. candidate must active + interactable;
8. PRE and candidate revalidated before dispatch.

No candidate or budget exhaustion => no action queued.

## Deferred managed action

The fresh `UIButton.HandleClickEvent()` method is not directly invoked by command 7.

Bridge builds a `System.Action` bound to the fresh UIButton using:

`System.Delegate.CreateDelegate(System.Type,System.Object,System.Reflection.MethodInfo)`

Reflection objects are obtained with IL2CPP metadata exports (`class_get_type`, `type_get_object`, `method_get_object`). The resulting Action is passed to:

`FGStudio.Engine.Utilities.MainThread.Execute(System.Action)`

Thus command 7 only queues work. Actual callback execution is deferred to the game's `MainThread.Update` processing of its action queue.

CI forbids direct `g_api.runtime_invoke(handleClick,...)` in the Revive engine.

## PRE

Controller requires:
- current session harmless proof PASS;
- Revive + MainThread metadata ready;
- scanner healthy and 60/60;
- observer stable;
- no transition;
- exact intent identity;
- dead=1 stable 2/2;
- ActionQueue empty.

## Dispatch ACK

ACK requires:
- token match;
- command ingress TID/current/main IDs match proven main thread;
- pre RoleID/MapID and preDead=1 match;
- `queued=1`;
- `directInvoked=0`.

ACK means only that the Action entered the game dispatcher. It is not gameplay success.

## Real POST

Success remains two consecutive observer snapshots with:
- same RoleID;
- dead=0;
- MapReady=1;
- WaitingChangeMap=0.

MapID may change. During map transition the one active Revive may be preserved solely to wait for POST; no new mutation is allowed.

## Diagnostic stages

Shared progress stages allow timeout localization:
1 FreshPre; 2 ResolveTypes; 3 Instances; 4 Enumerator; 5 ScanButtons; 6 Candidate; 7 DelegateReflection; 8 CreateDelegate; 9 MainThreadInstance; 10 MainThreadEnqueue; 11 Queued.

Timeout/failure always remains fail-closed with no retry in that death episode.

## Scope

This evidence milestone is Revive only. AutoFight, NPC, Sell, Path, Treatment and Buff remain locked.