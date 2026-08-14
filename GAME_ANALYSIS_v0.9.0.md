# GAME_ANALYSIS v0.9.0

## Căn cứ từ Interface.unity3d

Các Lua action đã bóc được và đang dùng:

- Asset xác nhận `TopIcon.AutoTrainClick()` chỉ bọc `AutoFight_Main.StartAutoFight(C_AutoModel.Train)`.
- `TopIcon.AutoStopClick()` chỉ bọc `AutoFight_Main.StartAutoFight(C_AutoModel.None)`.
- Production v0.9.0 gọi thẳng `AutoFight_Main.StartAutoFight(Train/None)` bằng tham số `System.Int32` boxed (`Train=1`, `None=0`), bỏ popup AUTO transient và bỏ wrapper `TopIcon`.
- `GameDialog.FunctionButtonClicked(uiButton)` lấy Tag của button, gửi action dialog rồi destroy GameDialog.
- `NPCShop.ToggleTabHeaderSelected(uiToggle)` chuyển Buy/Sell tab.
- `NPCShop_SellItemTab.RequestSellItem(dbItemData)` dùng `dbItemData.ID`, `CurrentShopData.NpcShopID`, `CurrentShopData.ID` để gửi sell request.
- `NPCShop.ButtonCloseClicked()` destroy NPCShop.

Vì `AutoTrainClick/AutoStopClick` chỉ là wrapper gọi `AutoFight_Main.StartAutoFight`, production v0.9.0 bỏ cả wrapper lẫn `AutoFightGroup` transient và gọi thẳng action combat trên script `AutoFight_Main`.

## Bán đồ và nguyên nhân crash được giảm

Tài liệu phân tích cũ đã chỉ ra các nhóm rủi ro chính: pointer UI stale sau đổi màn, thao tác khi Lua UI chưa tạo xong, delay đoán mò thay cho state và gọi UI từ remote worker ngoài Unity main-thread.

v0.9.0 giảm các race có thể loại bỏ ở tầng hiện tại:

1. AUTO phải dừng trước khi latching selling trip và được kiểm tra lại ngay trước `OpenNpc`.
2. `ClickNPC` xong có quiet window 1.2 s.
3. Lua UI phải ổn định hai lần liên tiếp trước khi resolve control.
4. Shop chỉ scan trong đúng `GameDialog`/`NPCShop` tree, không global UI dictionary.
5. Sau dialog action có quiet window trước khi đụng NPCShop.
6. Bán một item → chờ `GetItemData(DBID)==null` → quiet window → scan lại.
7. Đóng shop sau quiet window, không destroy UI ngay cùng response cuối.

Sell Engine vẫn giữ mô hình an toàn:

`SCAN BAG → chọn 1 DBID → RequestSellItem → WAIT server ACK → SCAN BAG AGAIN`.

## Giới hạn kiến trúc còn lại

Các Lua action vẫn được invoke qua remote worker đã `il2cpp_thread_attach`, không phải một dispatcher đã chứng minh chạy trên Unity main thread. Vì vậy v0.9.0 giảm mạnh số UI mutation/probe nhưng không thể tuyên bố triệt tiêu crash 100% nếu bản thân server/client yêu cầu callback chạy đúng Unity main-thread.

Nếu crash vẫn bám đúng thời điểm `RequestSellItem`/Lua action, bước đúng tiếp theo là trace payload bán thật (`CMD_NPC_SHOP_SELL_REQUEST = 200036`) để lấy chắc `NpcShopID + ShopID`, hoặc dựng main-thread dispatch; không tiếp tục tăng/giảm Sleep mò.
