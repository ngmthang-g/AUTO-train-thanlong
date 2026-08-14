# Game analysis notes - v0.8.3

## Build identity

- Unity IL2CPP metadata: v39
- `GameAssembly.dll` ImageBase: `0x180000000`
- PE timestamp: `0x6A410C14`
- SizeOfImage: `0x03DCB000`

## NPC identity model

`GetNearestNPC()` exposes nearby Lua map data with RoleID/Name. Native `ClickNPC(int)` resolves a GNPC by the integer compared against `GNPC.ResID`; therefore `ClickNPC(RoleID)` is semantically wrong.

The failing runtime RoleIDs reported for static NPCs (`1000000346`, `1000000378`) fit the client's billion-base NPC RoleID namespace. v0.8.3 uses `RoleID - 1,000,000,000` only as a candidate and validates it with the game's own `GetNearestNPC(candidate)` before storing/clicking. This avoids depending on a brittle guessed .NET Dictionary memory layout.

## UI/action architecture

The supplied analysis document identifies `LuaSystemAPI_GUI.MainCallUI/CallUI/MainFindUI/FindUI` and Lua script plumbing. This supports the longer-term architecture: action → wait for actual UI/script state → next action, rather than retaining stale UIButton pointers across page transitions.

Current NPC opening uses the high-level game action `ClickNPC(ResID)`. Treatment/shop follow-up still uses freshly-resolved live controls; no UI pointer is intentionally cached across page changes.

## AUTO architecture

The v0.8.2 deadlock came from coupling raw `EnableAutoF1` to navigation shutdown. v0.8.3 serializes ownership: navigation may proceed regardless of a stale raw AutoFight flag; AUTO callback scanning is reserved for the final train spot.

`AutoSetFlag(int RangerAuto)` is not used as activation because native inspection shows range-circle behavior rather than a proven “Đánh quái” selector.

## Asset corpus

The supplied pack contains `FGClientTool_Windows.dll` with `FG_Decrypt/FG_Encrypt`, `GameAssembly.dll`, `global-metadata.dat`, `Config.unity3d`, `Interface.unity3d`, `data.unity3d` and other bundles. These remain the corpus for later UI/portal/NPC database extraction. v0.8.3 does not invent KVD/PLT destination MapIDs without evidence.
