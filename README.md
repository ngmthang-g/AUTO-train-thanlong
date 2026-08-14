# Thần Long Mobile - Auto Train v0.8.8

## Sell Engine v2
Bản này thay toàn bộ vòng bán bằng `UIButton.HandleClickEvent()` của v0.8.7. Sau khi mở NPC/shop/bán nhanh/tab Trang bị, tool đọc dữ liệu Bag trực tiếp, giữ trang bị đầu tiên, chọn trang bị tiếp theo theo `Position`, gọi Lua action thật `NPCShop_SellItemTab.RequestSellItem(dbItemData)`, rồi chỉ chuyển món sau khi `GetItemData(DBID)` xác nhận server đã xóa món trước. Giới hạn tối đa 90 request.

Điểm an toàn mới:
- không giữ pool `UIButton*` suốt 90 giây;
- không click ItemBox trong vòng bán;
- snapshot `GetItemsAtSite(Bag)` được giữ bằng GC handle trong lúc đọc;
- trước mỗi request, item được resolve lại bằng DBID để tránh dùng object cũ;
- item type/sellable được cache theo ItemID để giảm remote-call;
- đóng shop sau quiet window và chỉ dùng một Lua close action (fallback tối đa một close control).

Build bằng `build.cmd`; output `dist\ThanLongAutoTrain_v0.8.8.exe`.
