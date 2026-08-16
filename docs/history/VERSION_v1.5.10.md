# VERSION v1.5.10 — Internal Confirm + Revive v0.8.7

Base: v1.5.9 Auto Rotate Train Spots.

## User request
- transplant the smooth v0.8.7 map Confirm and Revive implementation;
- remove map Confirm coordinate and periodic checkbox/timer;
- remove Revive coordinate;
- retain the four AutoFight calibration clicks only: AUTO / Attack / Stop1 / Stop2;
- prioritize clean integration to reduce client disconnect risk.

## Source changes
- protocol version -> `0x00010510`;
- new Bridge commands `ClickInternalConfirm=7`, `ClickInternalRevive=8`;
- remove generic unused Bridge `ClickAt`;
- restore donor-exact `MainFindUI("MessageBox")` observer in ReadState;
- add exact donor UI RVAs and PE/signature validation;
- cache `MessageBox` managed string with GCHandle;
- reject disposed UIObject entries;
- internal Confirm uses the donor accent-aware positive/negative matcher and calls `UIButton.HandleClickEvent()` only on a unique MessageBox button;
- internal Revive re-checks `get_IsDeath`, scans `UIObject.instances`, uses donor-style two-pass label matching, locks after success, and permits only one late retry after 15s;
- re-enable `HandleCrossMapConfirm` in `TickAccount`;
- remove train-map-ID reset from Confirm handler so tool-owned sell/recovery cross-map routes do not lose ownership;
- remove all old Confirm/Revive coordinate/timer UI/config paths; saving a profile deletes legacy Confirm/Revive coordinate/timer INI keys;
- click calibration arrays shrink from 6 to 4.

## Validation in editing workspace
- route/mount self-test 15/15 PASS;
- rotation self-test 8/8 PASS;
- static v1.5.10 integration audit 33/33 PASS;
- lexical delimiter/balance audit PASS;
- Windows PE build not run because Zig/Windows toolchain is unavailable in the workspace.

## Runtime status
NEEDS USER TEST. Build/static success is not runtime proof.
