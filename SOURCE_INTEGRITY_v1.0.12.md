# Source integrity - NewCore v1.0.12

v1.0.12 keeps the v1.0.11 bridge/scanner implementation and adds a controller-side control-plane scaffold only.

## New controller file
- `src/controller/control_scaffold.h` `52c937d62a3c88f86bc56280f68f1eeaf1ad2e26`

## Changed controller fragments
- `src/controller/main_part_01.inc` `807d6fcea1e444771d0481b75131c30a3598ea5c`
- `src/controller/main_part_05.inc` `9881d0c203504c28bff4b9245108cef0c3c2d0c6`
- `src/controller/main_part_07.inc` `e82d08e83cd1e8d047e43d5d6ae9da9b83484249`
- `src/controller/main_part_09.inc` `7487e6d2de1cb9d17c8a62521c92c4bf9569692c`
- `src/controller/main_part_10.inc` `9db95b60057beb4992754f0ae9d280fbba3816cd`
- `src/controller/main_part_11.inc` `5fb9ffd6e282fda6e55d3e0f03ec2c40097839df`
- `src/controller/main_part_14.inc` `f48ca43cf51634bf7e4706c33161af5d055fd3ff`
- `src/controller/main_part_15.inc` `70aef96e34749253176c3100987ec0d9fdef95bc`

`src/controller/main.cpp` remains the proven 15-part wrapper SHA `eebc55f1543bbbfc729176efb8477ccb5404cda2`. All unlisted controller fragments are inherited unchanged from v1.0.11.

## Common/build
- `src/common/protocol.h` `51f103fb83daf3bd7c0602ed2a8528e6139ee07b`
- `build.cmd` `d6a502b298121d21a536989673abf56562b2186a`
- `.github/workflows/build-windows.yml` `afd17f770418daab75de9de2aeea60a1e02c983e`
- `resources/app.manifest` `26584095d4d5c573b61333c0b7e01285645b5cb9`

## Architecture invariants pinned by source
- `kGameplayMutationEnabled = false` plus `static_assert`.
- `ActionQueueScaffold::kCapacity = 1` plus `static_assert`.
- BridgeCommand surface still ends at `ReadGameSnapshot`.
- No bridge/gameplay mutation implementation added.
- Existing bridge fragments are inherited unchanged from v1.0.11.

GitHub CI must still compile and verify the published tree before runtime use.
