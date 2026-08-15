# Than Long Clean Route v1.4.0 — Mouse Lock + F4 + Auto Sell Macro

Route train gốc vẫn giữ: đọc Map/X/Y → lên ngựa → AutoPath → StopPath → xuống ngựa → AUTO → Đánh quái.

## Click v1.4: ưu tiên dùng được trước

- Gameplay click dùng foreground `SendInput` và **chiếm chuột thật**.
- Tool đưa đúng cửa sổ acc lên foreground, đưa chuột tới điểm, chờ 100 ms, giữ LEFTDOWN **140 ms**, nhả chuột, chờ 90 ms.
- **Không trả lại con trỏ và không restore foreground** sau click. Đây là chủ ý của bản v1.4 để tăng độ chắc khi client không nhận click quá nhanh.
- F8 vẫn là phím global để lấy tọa độ.
- **F4 global**: tạm dừng/tiếp tục toàn bộ acc đang RUN. Khi pause, tool gửi `StopAutoPath` để nhân vật không tiếp tục chạy đường; không tự đổi combat state.

## 6 điểm click riêng từng acc

1. XÁC NHẬN RA MAP
2. ĐẦU THAI
3. AUTO
4. ĐÁNH QUÁI
5. DỪNG AUTO 1
6. DỪNG AUTO 2

Hai điểm cuối phục vụ flow bán đồ: khi túi full, tool dừng chế độ đánh bằng chuỗi 2 click trước khi lên ngựa đi NPC.

## Auto bán đồ

Mỗi acc có checkbox `AUTO BÁN ĐỒ KHI TÚI FULL` và chọn một NPC preset:

- **Mã Kiêu Minh** — MapID 5 — tọa độ hiển thị 284,188 — NPC ID/ResID **373**.
- **Dược Đại Phu** — Hỏa Diệm Sơn MapID 55 — tọa độ hiển thị 96,168 — NPC ID/ResID **279**.

Các ID trên được bóc từ Config.unity3d của client người dùng cung cấp; Bridge không hardcode RVA mà resolve `LuaSystemAPI_Game.ClickNPC(Int32)` từ IL2CPP metadata khi chạy.

### Bag guard

Bridge đọc read-only `LuaSystemAPI_Game.GetFreeBagSpace()` vào snapshot. Khi Auto Sell bật và `FreeBagSpace <= 0`:

1. Dừng Auto đánh bằng 2 tọa độ `DỪNG AUTO 1/2`, có verify `get_EnableAutoF1` nếu getter khả dụng.
2. StopPath hiện tại, lên ngựa và AutoPath tới preset NPC.
3. Khi tới nơi/down ngựa, gọi `ClickNPC(npcID)` trên game window thread.
4. Chờ UI NPC rồi chạy bảng macro bán đồ.
5. Sau khi macro kết thúc, đọc lại `FreeBagSpace`; chỉ coi là bán xong khi số ô trống > 0 và ổn định 1,5 giây.
6. Nếu túi vẫn full, tool mở NPC + chạy macro lại tối đa lần thứ 2. Vẫn full thì fail-closed/chờ thủ công.
7. Khi đã có ô trống, tool AutoPath về đúng bãi train của acc, xuống ngựa và chạy lại flow AUTO → Đánh quái.

Nếu không đọc được `FreeBagSpace`, tool **không tự đoán bán xong**.

### Bảng macro tọa độ bán

Bảng hỗ trợ tối đa 64 dòng. Mỗi dòng có:

- mô tả;
- tọa độ click riêng;
- `Delay ms` (50–60000);
- số lần `Lặp` (1–999).

Nút: `+ THÊM`, `- XÓA`, `LƯU DÒNG`, `LẤY DÒNG (F8)`, `TEST DÒNG`.

Macro chỉ đảm nhiệm UI shop tạm thời. Việc xác định lúc bắt đầu/kết thúc bán vẫn dựa trên state túi thật, không dựa vào số click cố định.

## Tọa độ NPC và scale

Preset lưu tọa độ hiển thị 284,188 / 96,168. Runtime tự chọn scale 1 hoặc 100 dựa trên hệ tọa độ snapshot/bãi đang dùng. Log/runtime vẫn cho thấy tọa độ raw thực tế. Nếu client build khác dùng hệ tọa độ khác, cần runtime test trước khi tin preset.

## Multi-account / settings

- Thư viện bãi chung: `ThanLongCleanRoute.spots.tsv`.
- Profile từng RoleID: `ThanLongCleanRoute.accounts.ini`.
- Mỗi RoleID giữ riêng: bãi đang chọn, tolerance, option, 6 fixed-click points, Auto Sell/NPC preset và toàn bộ sell macro.
- Mỗi process có BridgeClient + RuntimeState riêng.

## Giới hạn còn chủ ý

- Xác nhận popup chuyển map vẫn là heuristic read-only cross-map stall; client chưa có getter đã xác minh kiểu `ConfirmPopupVisible`.
- Macro bán chưa biết UI shop đang mở ở bước nào; vì thế dùng delay cấu hình của từng dòng. Kết thúc macro được xác nhận lại bằng `GetFreeBagSpace`.
- `ClickAt` Win32-thread cũ vẫn giữ trong protocol để diagnostic compatibility nhưng gameplay v1.4 dùng mouse-lock foreground input.
