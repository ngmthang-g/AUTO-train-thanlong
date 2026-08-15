# Donor state migration from Legacy v0.9.0 (pre-NewCore) -> NewCore v1.0.9

## Nguyên tắc

`Legacy v0.9.0` ở tài liệu này là **bản auto cũ, pre-NewCore**. Nó không phải một version của NewCore.

Legacy v0.9.0 là donor đã runtime-test cho **state semantics / state-machine guards**. NewCore không sao chép executor `Remote()` hoặc remote worker.

| State | Legacy v0.9.0 donor | NewCore v1.0.9 |
|---|---|---|
| MapID | RoleData / donor offset | managed `get_MapID()` |
| X/Y | RoleData +0x54/+0x58 | managed getters/backing object only |
| Dead | `LuaLeaderIsDeath` | `LeaderRoleData.get_IsDeath()` |
| Riding | `LuaIsRiding` | `LeaderRoleData.get_IsRiding()` |
| Moving | `LuaIsMoving` | `LuaSystemAPI_Game.IsMoving()` |
| AutoFight | donor inverted AutoF1 semantic | same semantic normalized into `autoFight` |
| Bag | `GetFreeBagSpace` | same read-only API through metadata |
| MapReady | `LuaIsMapReady` | `LuaSystemAPI_Game.IsMapReady()` |
| WaitingChangeMap | `SessionWaitingChangeMap` | `SessionData.get_WaitingChangeMap()` |
| AutoPathing | `AutoPathIsRunning` | `AutoPathManager.get_IsAutoPathing()` |

## Guard được port nguyên ý nghĩa

Donor đọc `MapReady/WaitingChangeMap` trước và return sớm khi scene đang chuyển. v1.0.9 áp dụng lại đúng guard này trước mọi Leader/AutoPath read.

## Những thứ tuyệt đối không port

- remote worker / remote thread executor;
- raw donor RVA làm gameplay action;
- giữ pointer UI qua transition;
- `Sleep()` để suy đoán state;
- retry action mù.

## AutoPath metadata evidence

Supplied `global-metadata.dat` có:

- `FGStudio.Engine.Logic`
- `AutoPathManager`
- `get_IsAutoPathing`

Donor RVA `AutoPathIsRunning` chỉ được dùng để đối chiếu ý nghĩa; runtime NewCore resolve metadata.
