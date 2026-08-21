# EVIDENCE — v0.9.0 cross-map Confirm detector

Type: `SOURCE / USER_RUNTIME`

Date: 2026-08-16

Source supplied by user:
`ThanLongAutoTrain_Source_v0.9.0(2).zip`

User runtime statement:
- old source recognized when cross-map Confirm was needed very well;
- current CleanRoute manual Confirm TEST works, but automatic timing/detection does not.

## Direct source observations

The supplied ZIP contains `src/main.cpp`.

### Authoritative MessageBox observer

v0.9.0 creates a managed `MessageBox` name and resolves the top-level UI using:

`LuaMainFindUI("MessageBox")`.

`ReadMessageBoxVisible()` returns true only when that UI object exists.

### Snapshot/transition ordering

`RefreshLive()` first reads:
- `LuaIsMapReady`;
- `SessionWaitingChangeMap`.

When map is not ready or `WaitingChangeMap` is true, it does not treat MessageBox as a Confirm candidate.

When map is stable it publishes:
- MapID/X/Y;
- moving;
- AutoPath running;
- MessageBox visible;
- map-ready / waiting-change-map.

### Actual Confirm-needed gate

The navigation worker requires a combination of:
- route/destination is cross-map;
- current map still differs from destination;
- route progress has stalled / character is not moving;
- mount/navigation state is settled;
- `messageBoxVisible == true`;
- retry cooldown satisfied.

Only then does it perform the Confirm action.

After the click it enters an `awaitingMapTransition` state and waits for real transition evidence instead of assuming the click succeeded.

## What this evidence supports

**CONFIRMED:** old detector did not use elapsed stall time alone as proof that a Confirm popup existed. It had a dedicated semantic/existence `MessageBox` signal and combined it with route context.

**CONFIRMED:** `MainFindUI("MessageBox")` was the donor's observer path.

**LIKELY:** the missing semantic popup-presence observer is a major reason recent CleanRoute versions know the coordinate but fail to know the correct moment to click.

## What this evidence does NOT prove

- that every `MessageBox` is specifically a map-transfer dialog;
- that v0.9.0 remote worker/RVA/UIButton implementation should be reused;
- that the exact old 7-second stall constant is required in CleanRoute;
- that v1.5.2 will runtime-pass before user testing.

## Design consequence for CleanRoute

Keep only the detector principle:

`tool-owned cross-map route + real route evidence + stable map state + AutoPath/movement settle + semantic MessageBox present => saved Confirm click`

Use current CleanRoute Bridge/snapshot/state-machine architecture. Do not transplant old execution architecture.
