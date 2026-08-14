# Thần Long Mobile - Auto Train v0.8.9

Bản v0.8.9 là bản dọn kiến trúc UI/chat từ v0.8.8.

## Đã xóa tận gốc

- Toàn bộ `ChatPing` / điều hướng bằng chat.
- `ClickGamePoint`, `TypeUnicode`, tọa độ hiệu chỉnh chat và toàn bộ UI hiệu chỉnh chat.
- Toàn bộ Auto Chat: config, UI, worker, UIInput, Lua ChatBox.
- Code chết `OpenAutoRootButton`, `FindChatOpen`, `FindChatPanelAction`.
- Toàn bộ đường `UIRectTransform` / `InvokeRectLua` / callback với PointerEventData giả/null.

Điều hướng bãi hiện chỉ còn `AutoPathManager.Start/Stop` nội bộ.

## Trị liệu và shop

Các bước dễ lỗi trước đây không còn fallback sang UIRect/Lua giả pointer:

- NPC dialog: `GameDialog.FunctionButtonClicked(UIButton)`.
- Xác nhận trị liệu: `MessageBox.ButtonOKClicked()`.
- Tab Bán vật phẩm: `NPCShop.ToggleTabHeaderSelected(UIToggle)`.
- Bán từng món: `NPCShop_SellItemTab.RequestSellItem(DBItemData)` + chờ server xóa DBID rồi scan lại.
- Đóng shop: `NPCShop.ButtonCloseClicked()`.

`Bán vật phẩm nhanh` không có SelectHandler Lua riêng trong asset; tool đặt trạng thái trên `UIToggle` thật của game. Tab `Trang bị` cũng dùng `UIToggle` thật, để listener `BagItemsGrid.ToggleTabSelected` của game chạy theo control gốc.

## Build

Chạy `build.cmd` với Zig 0.15.2. Output:

`dist\ThanLongAutoTrain_v0.8.9.exe`

Workflow `.github/workflows/build-windows.yml` vẫn tự tìm `ThanLongAutoTrain_v*.exe`, không hardcode tên artifact ở bước verify.
