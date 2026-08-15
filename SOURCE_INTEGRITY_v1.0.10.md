# Source integrity - NewCore v1.0.10

v1.0.10 preserves the proven v1.0.9 publication layout to minimize diff surface.

## Intended full translation units
- bridge.cpp Git blob SHA (local reconstructed full source): `e75b23d21299629beac889d8f7d9b660d3e1f0e5`
- controller/main.cpp Git blob SHA (local reconstructed full source): `d3c56217e5ee42eb174b8928f86d4958c9dbc729`

## Changed bridge fragments
- `src/bridge/bridge_part_07.inc` `9615519be90876d1668b81f959abc91746d4b3bc`
- `src/bridge/bridge_part_08.inc` `9fefd57a356c4aaaba50765ad4dd4f382780a816`

Bridge parts 01-06 and the 8-part wrapper are inherited unchanged from v1.0.9.
Wrapper SHA: `ce395cd1c3b4d83b55e987c83232582442c25e13`.

## Changed controller fragments
- `src/controller/main_part_01.inc` `efe42a33a776f961de5399cea5f595f3cdcc7be1`
- `src/controller/main_part_05.inc` `746b275ff8200cfd47bd350aeb3f1dbfdcd53afe`
- `src/controller/main_part_07.inc` `5b4cbc0b8b402e18df54b3d893d20f01b0b5eecf`
- `src/controller/main_part_09.inc` `43ab3f08e554bd3e577c77798c15e4ca4e6e9eb6`
- `src/controller/main_part_10.inc` `9879b3cd97b568b980cb10c8113cbe333e34f52b`
- `src/controller/main_part_11.inc` `efeb0e42c9b9ca8a3d72e8203159c0aa0a5abf59`

All other controller fragments and the 15-part wrapper are inherited unchanged from v1.0.9.

## Protocol
- `src/common/protocol.h` remote Git blob SHA: `51747ff5116a83b10409db2afb06239f9a8658c6`

`build.cmd` audits `.cpp`, `.h`, and `.inc` for forbidden architecture tokens.
