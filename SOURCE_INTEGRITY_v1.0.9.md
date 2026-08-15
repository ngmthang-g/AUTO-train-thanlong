# Source integrity - NewCore v1.0.9

Large controller source is published as ordered `.inc` fragments because the GitHub connector previously corrupted large text payloads. The wrapper `src/controller/main.cpp` includes the fragments in numeric order. The fragments concatenate byte-for-byte to the intended local v1.0.9 controller translation unit.

## Controller wrapper

- `src/controller/main.cpp`: `eebc55f1543bbbfc729176efb8477ccb5404cda2`

## Controller fragments

- `main_part_01.inc`: `ff9d5687fbdeba3bf2359289747e58e4d5f9326d`
- `main_part_02.inc`: `962bf46619688fee9626ad17bbba6aedf388e38c`
- `main_part_03.inc`: `5a296e22b8bfc5291f71fb3180db21d3fea3a477`
- `main_part_04.inc`: `7a0ae3aa7de96bc0d5576ad73125f5873435cb1d`
- `main_part_05.inc`: `f78f6458d0d36832e7c4ffa40a51db01684e8abc`
- `main_part_06.inc`: `396912c9108afd0c5fdb2f755ef0da6b180d68cd`
- `main_part_07.inc`: `25ecab1de23a63927cebd18014c3f6b85d11c4a3`
- `main_part_08.inc`: `119496ceb916589d75a79da064d5bb5b7f7aeb21`
- `main_part_09.inc`: `14af6d02efdf33d3e671cd9e69730aff6e914158`
- `main_part_10.inc`: `ddb6f8b5a1d9463186c8a5fbdc2cec186e3c2b92`
- `main_part_11.inc`: `3b849a6c4dd1c45ef0486fbb14f3b745741ae38a`
- `main_part_12.inc`: `25eedfcf8e19a8502f971a6b8ff4cf96f67c9ce0`
- `main_part_13.inc`: `1bfe8131d23c53d59a13f3c425f57fe6692dc8a0`
- `main_part_14.inc`: `669ba267b2e1df712a45b87d7163734b8a6e0abe`
- `main_part_15.inc`: `8fd2d6a57088c91a98a6d15e6c7612a1c1b2ea74`

## Bridge

Bridge implementation is unchanged from the v1.0.8 CI-passing publication. Its fragment hashes remain pinned in `SOURCE_INTEGRITY_v1.0.8.md`.

## Audit invariant

`build.cmd` MUST scan `*.cpp`, `*.h`, and `*.inc` for forbidden architecture tokens. A publication that omits `.inc` from the architecture audit is invalid.
