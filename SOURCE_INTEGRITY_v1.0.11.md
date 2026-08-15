# Source integrity - NewCore v1.0.11

v1.0.11 preserves the proven v1.0.10 publication layout and changes only the controller fragments needed for unified map-transition classification plus protocol/docs/build metadata.

## Intended full translation units
- bridge.cpp Git blob SHA (unchanged reconstructed full source): `e75b23d21299629beac889d8f7d9b660d3e1f0e5`
- controller/main.cpp Git blob SHA (local reconstructed full source): `e1c8717a5c19b0cf4e3c7c9e0c256278ee6b1f1f`

## Changed controller fragments
- `src/controller/main_part_01.inc` `db92df71a3c16101215a1742c14df950af7ab637`
- `src/controller/main_part_05.inc` `38ee9fe8e6e9e35518fc98052013c1f37c0cb4d9`
- `src/controller/main_part_09.inc` `6a29608a0818e81c1531fa03bd80a360b613a24f`
- `src/controller/main_part_10.inc` `c46d8bf9d4cdbd6217f9a297948dac02134e07e2`
- `src/controller/main_part_11.inc` `fb99108a75585d01ae1d29910a343f5a371ef0cf`

All other controller fragments and the 15-part wrapper are inherited unchanged from v1.0.10.

## Bridge publication
All bridge fragments and the 8-part wrapper are inherited unchanged from v1.0.10. The bridge binary still rebuilds because the shared protocol version changed.

## Protocol
- `src/common/protocol.h` local Git blob SHA: `93d339ea12effe31a1a09366540e96ee3a2ddffb`

`build.cmd` audits `.cpp`, `.h`, and `.inc` for forbidden architecture tokens.
