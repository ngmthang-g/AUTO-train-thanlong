# Source Integrity - NewCore v1.2.0 Revive Only

Proven parent:
`agent/newcore-v1.1.0-integrated-acceptance` / `b03304ccbd39a3b34c44fb3920296c975c6824f5`

Final branch must be squashed to exactly **one commit** over that parent. GitHub Actions on the final squashed head is authoritative.

## Key intended blobs before final squash

Protocol / build:
- `.github/workflows/build-windows.yml` `174741bb8df40fb9e289b4f85ef5581608619ee6`
- `build.cmd` `8d7260ee1577d3d05cbd20d37e91cbfa7e4bef50`
- `src/common/protocol.h` `5bb8b38f142c93dda0f60966435cba5e563cf231`
- `resources/app.manifest` `f4b063a1b02099a75fc813eb14a9902ccd9e1572`

Bridge:
- `src/bridge/bridge.cpp` `ce395cd1c3b4d83b55e987c83232582442c25e13` — wrapper inherited
- scanner/foundation parts 01..07 remain inherited from proven v1.1.0
- `src/bridge/bridge_part_08.inc` `2f72077a14cbe59ac992eb31df64d666b783d8fa`
- `src/bridge/action_capability_probe.inc` `5d4c132410b5c60d09b250f4d2ef4b62fb97e2fd` — metadata-only, inherited
- `src/bridge/harmless_action_envelope.inc` `93e629d615cf67075f5b77ff4034103f8589212d` — proven infrastructure envelope, inherited
- `src/bridge/revive_action_engine.inc` `a043395a7100dfa47aedc355078e8f5b9031b8f8`

Controller:
- `src/controller/main.cpp` `f0998181d68013ef26cb6ea67ca48f6469eb4356`
- `src/controller/main_part_01.inc` `0b62c41ee6604353dc3e60991a094c53d55a2cb7`
- `src/controller/main_part_03.inc` `26a723a2cf38901687a664159d7a620e30f2edfa`
- `src/controller/main_part_05.inc` `59e80ef9c766567b2f01e1a8fee2f720f1717724`
- `src/controller/main_part_06.inc` `a2f537dc1b7246ad03e856568bcf7e52ed2ab164`
- `src/controller/main_part_09.inc` `28a1ff907b7a51c4b15e477884f6f03852c12540`
- `src/controller/main_part_10.inc` `a13cb44af22ce527dc45ebf41e29ac0024336d66`
- `src/controller/main_part_11.inc` `55b62b4e97af570a0efa6b19d98170ad258689cf`
- `src/controller/main_part_14.inc` `1c68f058426e54354624959aeb8e05d32898cd2f`
- `src/controller/main_part_15.inc` `6ef530670cf7fee9270cf2b796b2784cbe5449ae`
- `src/controller/revive_control_gate.h` `9ebf844fd49a287783ef7fb4d2ed3c99237da2e0`
- `src/controller/revive_controller_methods.inc` `b91c6dca3c327b7f325c7c4ff954fb3752f709df`
- `src/controller/revive_selftest.cpp` `51acdf53ad2c240dec3a9a84de4deeef243b690b`

Docs/audit:
- `README.md` `4dfe12d17488690eb0277fab0fb305dfcba4259a`
- `STATIC_AUDIT.txt` `2241a69bff6d2d3134aa8ebdb24a71484511cb53`
- `CHANGELOG_v1.2.0.md` `6287eb25adc3c6a462dcccdfdc1e230c18976bfa`
- `BUILD_STATUS_v1.2.0.txt` `975b8b8ff804e6e091dcfc8776bff6b9c755784a`
- `docs/FIRST_RUNTIME_TEST.md` `477eff3c1b7784d40bce270de7923d90aab8b423`
- `docs/REVIVE_ACTION_EVIDENCE.md` `03e4bc8863a73606d83f57950b5d0effe15b458a`

## Mutation boundary

- Global `kGameplayMutationEnabled=false` is inherited and remains unchanged.
- Exclusive `kReviveMutationEnabled=true` is the only gameplay exception.
- Command 7 is the only gameplay mutation BridgeCommand.
- Revive engine may directly invoke only fresh-resolved `UIButton.HandleClickEvent()` and exactly once per dispatch attempt.
- No AutoFight/NPC/Sell/Path action call is present in the Revive engine.

## Runtime pointer boundary

No UIButton pointer crosses the command boundary. `UIObject.instances`, candidate button, method metadata and button state are reacquired/revalidated inside command 7 and discarded when the command returns.

## Queue / transition boundary

Revive moves queued -> active before command 7. A successful dispatch ACK leaves the queue active until real observer POST. Map transitions may preserve that one active Revive only to wait for POST; they never permit a second action.

## POST boundary

Revive POST permits MapID change but requires same RoleID. Two stable `dead=0`, MapReady=1, WaitingChangeMap=0 snapshots are required before CompleteActive().

## Automated gate observed before final squash

WIP compiler run #58 (`31876253273`) passed:
- architecture audit;
- direct Revive callback count=1;
- ControlSelfTest 48/48;
- ReviveSelfTest 16/16;
- Bridge build + LoadLibrary;
- Controller build + Verify.

That run is not the release checkpoint. Final artifact/hash must come from a new GitHub Actions run on the final one-commit squashed head.
