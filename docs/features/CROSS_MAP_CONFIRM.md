# FEATURE: CROSS-MAP INTERNAL CONFIRM — v1.5.10

## Purpose
Use the real MessageBox and the real UIButton callback from the runtime-proven v0.8.7 donor instead of a saved screen coordinate or periodic blind click.

## Detector + action
1. a successful cross-map `StartPath` issued by this tool arms ownership;
2. `ReadState` checks the donor-exact `MainFindUI("MessageBox")` path and publishes `ValidConfirmUi + confirmUiVisible`;
3. Path ON is allowed and treated as route evidence;
4. MessageBox must be authoritative and character movement must be authoritative OFF;
5. wait 200 ms popup settle;
6. if AutoPath still reports ON, issue one serialized `StopPath` and wait at least 250 ms;
7. re-use the live MessageBox tree, inspect active/interactable buttons, select one unambiguous positive candidate;
8. call donor `UIButton.HandleClickEvent()` on the game-window callback thread;
9. immediately enter Transition Freeze and wait for stable map/client recovery.

## v0.8.7 donor boundary
Fixed RVAs are deliberately transplanted only for this proven UI slice. They are gated by exact GameAssembly timestamp/SizeOfImage and byte signatures. A mismatch leaves `ValidConfirmUi` unset and blocks the internal action.

The donor's external remote worker is **not** transplanted. v1.5.10 uses the existing serialized in-process Bridge hook and current safety/state machine.

## Do-Not-Break
- never re-add saved Confirm coordinates or periodic Confirm timer;
- never click Confirm without tool-owned cross-map route context;
- never let AutoPath ON hide a real MessageBox;
- never issue another mutable action in the same cycle after internal Confirm;
- timeout/missing UI/signature mismatch/ambiguous button = fail closed;
- keep post-click Transition Freeze.
