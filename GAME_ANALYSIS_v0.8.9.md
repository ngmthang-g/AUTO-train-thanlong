# GAME_ANALYSIS v0.8.9

## Lua actions dùng cho NPC/shop

Từ `Interface.unity3d` đã giải mã:

- `GameDialog:FunctionButtonClicked(uiButton)` lấy `uiButton.Tag`, gửi `CMD_SHOW_GAMEDIALOG`, rồi destroy GameDialog.
- `MessageBox:ButtonOKClicked()` destroy MessageBox rồi chạy OK callback.
- `NPCShop:ToggleTabHeaderSelected(uiToggle)` bật đúng Buy/Sell tab root.
- `NPCShop_SellItemTab:RequestSellItem(dbItemData)` gửi sell request từ `dbItemData.ID`, `CurrentShopData.NpcShopID`, `CurrentShopData.ID`.
- `NPCShop:ButtonCloseClicked()` destroy NPCShop.
- `BagItemsGrid:ToggleTabSelected(uiToggle)` lọc lại inventory và `RebuildLayout()`.

Vì vậy v0.8.9 bỏ hoàn toàn UIRect pointer-event fallback. Với các bước có Lua action rõ ràng, tool gọi đúng action. Với `ToggleQuickSell`, asset không khai báo SelectHandler riêng nên control thật `UIToggle` là đường đúng; không bịa thêm Lua function không tồn tại.

## Bán đồ

Sell Engine v2 giữ mô hình:

`GetItemsAtSite(Bag)` → chọn data item → `RequestSellItem` → đợi `GetItemData(DBID)==null` → scan lại.

Không giữ ItemBox pointer qua lúc server `RemoveItem/UpdateItemsList` và UI `RebuildLayout()`.
