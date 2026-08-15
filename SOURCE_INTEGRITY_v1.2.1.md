# Source Integrity - NewCore v1.2.1 Revive Dispatcher Anti-Diss Hotfix

Proven parent:
`agent/newcore-v1.2.0-revive-action` / `226f649001773ef8295dd502ff63a0725eee860a`

Final branch must be exactly **one squashed commit** over that parent. GitHub Actions on the final squashed head is authoritative.

## Runtime evidence driving change

v1.2.0 live PID 23432 passed scanner/MainThread/harmless/queue gates, then timed out after sending command 7. Exact internal sub-step is unknown because v1.2.0 lacked progress breadcrumbs. v1.2.1 removes synchronous direct click and bounds UI work rather than increasing timeout.

## Key implementation blobs before final squash

- `src/common/protocol.h` `a9935047f72a0d989aaf17a04e278ad7c1885f1b`
- `src/bridge/revive_action_engine.inc` `2ea5c70a5d21c1c5d47cd8c6c9aa7ba25e70eb4d`
- `src/controller/revive_control_gate.h` `bc7e073fbeaf1c910f8e9de3afe419e962ebda26`
- `src/controller/revive_controller_methods.inc` `eea1757a2afc812e33a258fabe1569e7c0a25c4d`
- `src/controller/main_part_03.inc` `70d6335b9b448662f67235f32e8d5b0fc4890ada`
- `src/controller/main_part_05.inc` `3532d5f2d7f4705e1def797eaa4f964f14b23363`
- `build.cmd` `6133b2292475f9bd1bce74ba3e541d44b93296e6`
- `.github/workflows/build-windows.yml` `6c50a57855dce06f1feeaac0b11ec2a73e6160bd`
- `resources/app.manifest` `54bd0d8f5c9a47199ed35241483002c8a40ed9b3`
- `src/controller/main_part_01.inc` `bb2158174bc316d3ba489e7a9c878bd74ab147dc`
- `src/controller/main_part_14.inc` `022591f35272eb743611a65b664672b0c29f5bfe`
- `src/controller/main_part_15.inc` `d8aa8cf3378a21c5f3ce1ad3564110941f56b921`

Docs/audit after update:
- `README.md` `dc9ae8d1363631ba7271140a56e542ef60f25027`
- `docs/FIRST_RUNTIME_TEST.md` `6ee9d3b9f729c0b49adf8c616fa3251a400f405c`
- `docs/REVIVE_ACTION_EVIDENCE.md` `c61eacc3390ea398c6bda8b20947a34a7b5919e5`
- `STATIC_AUDIT.txt` `5826418dbf004115a1f80a807fa08006ce4d9253`

## Hard anti-diss invariants

1. `WH_GETMESSAGE` may perform only bounded resolve/queue work.
2. Revive resolver hard budget is 120ms and hard entry cap is 4096.
3. Direct `g_api.runtime_invoke(handleClick,...)` is forbidden.
4. Fresh UIButton `HandleClickEvent` is converted to a managed `System.Action`.
5. Action is queued with `FGStudio.Engine.Utilities.MainThread.Execute(System.Action)`.
6. ACK requires queued=1 and directInvoked=0.
7. Gameplay success remains real observer dead=1→0 POST, two stable snapshots.
8. MapID may change; RoleID may not.
9. ActionQueue remains MAX=1.
10. No retry in the same death episode.
11. AutoFight/NPC/Sell/Path remain locked.

## Diagnostic invariant

Revive command writes progress stage/object/button/time evidence into SharedBlock while executing. Any future command timeout must expose those breadcrumbs so the blocking stage can be isolated without guessing.

## WIP compiler evidence

GitHub Actions run #67 / `31879947871` passed on the WIP implementation:
- anti-diss architecture audit;
- direct HandleClickEvent invoke=0;
- deferred MainThread.Execute(Action) required;
- ControlSelfTest 48/48;
- ReviveSelfTest 18/18;
- Bridge LoadLibrary;
- controller v1.2.1 build/Verify.

This WIP artifact is not the final release checkpoint. Final CI must run again after the one-commit squash.