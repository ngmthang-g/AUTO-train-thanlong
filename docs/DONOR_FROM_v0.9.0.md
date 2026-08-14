# Donor knowledge retained from v0.9.0

## Confirmed semantics retained

- `AutoFight_Main.StartAutoFight(1)` = Train.
- `AutoFight_Main.StartAutoFight(0)` = None/Stop.
- `GetAutoFightEnabled`: donor logic observed `false` while Train is active, so NewCore normalizes it to `snapshot.autoFight=true`.
- `LuaSystemAPI_Game.ClickNPC(npcID)` is the preferred NPC entry action.
- Donor role offsets (MapID `+0x50`, X `+0x54`, Y `+0x58`) are documented only for research comparison; NewCore core does **not** use them as runtime fallback.
- Sell logic to migrate later: `scan -> choose one DBID -> reacquire current object -> RequestSellItem -> wait DBID gone -> rescan`.

## Explicitly rejected from donor

- `CreateRemoteThread + il2cpp_thread_attach` as action executor.
- `UIButton*` / `UIToggle*` stored across UI transitions.
- fixed `Sleep` as state transition proof.
- monolithic boolean state combinations.
- automatic retry spam after ambiguous state.
