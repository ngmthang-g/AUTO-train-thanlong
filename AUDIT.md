# AUDIT v0.8.8

- Lua asset xác nhận quick-sell ItemBox cuối cùng gọi `NPCShop_SellItemTab:RequestSellItem(dbItemData)`.
- `RequestSellItem` tạo `{DBID}:{NpcShopID}:{ShopID}` và gửi `CMD_NPC_SHOP_SELL_REQUEST` (200036).
- Vòng 90 ItemBox/UIButton đã bị loại khỏi Sell Engine.
- Bag được rescan sau mỗi server ACK; không giữ pointer UI của item để bán món kế tiếp.
- `GetItemsAtSite(Bag)` snapshot được GC-handle trong lúc đọc.
- DBID/ItemID/Position đọc trực tiếp từ LuaItemData offsets đã được đối chiếu với getter native của đúng GameAssembly.
- Trước mỗi Lua sell action, `GetItemData(DBID)` resolve lại object hiện hành.
- Chỉ khi `GetItemData(DBID)==null` mới coi request bán đã được server xác nhận.
- Sau request cuối không đóng shop ngay; có quiet window và một Lua close action.
- 69/69 signature trong Validate() đã được kiểm tra lại với GameAssembly hiện tại.
