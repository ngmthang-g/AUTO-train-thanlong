# GAME_ANALYSIS v0.8.8

Recovered from `Interface.unity3d`:

```lua
function NPCShop_SellItemTab:RequestSellItem(dbItemData)
    local strCmd = String.Format("{0}:{1}:{2}", dbItemData.ID, CurrentShopData.NpcShopID, CurrentShopData.ID)
    Network.SendPacket(G_TCPPacketDefine.CMD_NPC_SHOP_SELL_REQUEST, strCmd)
end
```

`CMD_NPC_SHOP_SELL_REQUEST = 200036`.

`BagItemsGrid` receives server `RemoveItem` / `UpdateItemsList`, clears item boxes and rebuilds the layout. Therefore a long-lived pool of ItemBox/UIButton pointers is unsafe while selling. Sell Engine v2 avoids that UI loop and uses authoritative inventory state:

`scan Bag -> choose equipment after protected first position -> resolve DBID again -> RequestSellItem -> wait GetItemData(DBID)==null -> rescan`.

The supplied `GameAssembly.dll` also confirms the LuaItemData getters used by the scan are simple field getters:
- ID: +0x10
- ItemID: +0x14
- Position: +0x1C

The v0.8.8 scanner reads these fields directly from the held snapshot, reducing managed remote calls while server/UI updates are in progress.
