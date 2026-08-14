# Game analysis notes - v0.8.2

## Build identity

- Unity IL2CPP metadata: v39
- `GameAssembly.dll` ImageBase: `0x180000000`
- PE timestamp: `0x6A410C14`
- SizeOfImage: `0x03DCB000`

## NPC identifier proof

The `ClickNPC(int)` native predicate compares its integer argument against the candidate `GNPC` field at `+0xC0`. `GNPC.get_ResID()` returns that exact field. Therefore the public parameter name `npcID` is semantically `ResID`, not the runtime RoleID.

Runtime RoleID remains useful for correlating `GetNearestNPC()`'s `LuaMapObjectData` with the corresponding object in `SessionData.NPCs` / `MovingNPCs`; it must not be passed to `ClickNPC`.

## AUTO architecture

`AutoSetFlag(int RangerAuto)` was rejected as an activation API after native inspection: it is used to draw/update the auto-fight range circle. AUTO combat activation remains UI-callback based until the exact Lua/control handler is proven.

Navigation and combat are serialized. AUTO menu operations are forbidden while AutoPath, portal confirmation, mount transition or map loading owns the session.

## Encrypted Unity bundles

The supplied game pack also contains `FGClientTool_Windows.dll` exporting `FG_Decrypt` / `FG_Encrypt`. Several `.unity3d` bundles have a transformed header rather than a plain `UnityFS` signature, while `data.unity3d` is a normal UnityFS bundle. This is useful for future static UI/prefab analysis, but v0.8.2 does not rely on guessing/decrypting those assets for production callbacks.
