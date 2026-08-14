# Thần Long Mobile - Auto Train v0.9.0

v0.9.0 tách hẳn hai engine combat và ưu tiên ổn định khi chuyển trạng thái/ngựa/NPC.

## Hai chế độ train độc lập

- `Ctrl+Tab → skill đã chọn`: Ctrl+Tab là cơ chế chọn mục tiêu thật của mode này; khi target rỗng/chết mới gửi Ctrl+Tab rồi dùng skill đã chọn. Tọa độ bãi được đối chiếu liên tục.
- `AUTO → Đánh quái`: không dùng Ctrl+Tab. Sau khi tới bãi và xuống ngựa, tool gọi trực tiếp Lua `AutoFight_Main.StartAutoFight(Train)` rồi xác minh `EnableAutoF1=OFF`. Sau khi đã xác minh AUTO, X/Y chỉ được dùng để quyết định lệch bãi mỗi 5 phút; MapID vẫn guard liên tục.

## State machine lên ngựa

- Gọi ngựa một lần, chờ đủ 4 giây.
- Nếu chưa cưỡi: dọn quái 10 giây bằng đúng combat engine đang chọn rồi thử ngựa lại.
- Nếu lần thử ngựa thứ 2 vẫn thất bại: chạy bộ bằng AutoPath trong 1 phút, sau đó bắt đầu lại chu kỳ lên ngựa.
- Khi `riding=true`, combat/target bị khóa trong pha di chuyển. Nếu AUTO dọn quái còn chạy thì lệnh Dừng được gửi ngay ở lần scan đầu tiên thấy đã cưỡi; tuyệt đối không chờ hết timer 10 giây. Không Ctrl+Tab/skill/AUTO-train lại chỉ vì có quái gần đó.
- Với AUTO mode, pha dọn quái chỉ thử bật AUTO tối đa 2 lần; sau pha dọn quái phải xác minh `AUTO → Dừng` trước khi gọi ngựa. Nếu không dừng được thì không gọi ngựa, chuyển sang chạy bộ để tránh vừa cưỡi vừa bị AUTO kéo xuống đánh.

## Ổn định AUTO → Đánh quái

Không còn mở transient `AutoFightGroup` trước khi train/dừng. Asset `Interface.unity3d` cho thấy `TopIcon.AutoTrainClick()` gọi thẳng `StartAutoFight(Train)` và `TopIcon.AutoStopClick()` gọi `StartAutoFight(None)`, nên v0.9.0 bỏ wrapper `TopIcon` và gọi thẳng `AutoFight_Main.StartAutoFight(Train/None)` với enum đã xác nhận (`Train=1`, `None=0`), không tạo popup AUTO transient; sau đó chỉ kiểm tra trạng thái có giới hạn.

## Ổn định bán đồ

- Trước khi đi/mở NPC, nếu AUTO còn chạy thì phải dừng và xác minh; thất bại thì hoãn bán, không mở NPC.
- Sau `ClickNPC`, giữ yên 1.2 giây trước khi thăm dò UI.
- `GameDialog`/`NPCShop` phải tồn tại ổn định 2 lần liên tiếp.
- Không quét toàn bộ `UIObject.instances` cho shop nữa; chỉ dò button/toggle nằm dưới đúng Lua UI root hiện tại.
- Sau `GameDialog.FunctionButtonClicked`, giữ yên 900 ms để GameDialog bị destroy và NPCShop được tạo xong.
- Sell Engine vẫn là data-driven: `GetItemsAtSite(Bag)` → `RequestSellItem(DBItemData)` → chờ server xóa DBID → scan lại. Mỗi ACK có quiet window khoảng 1 giây trước món tiếp theo.

## Các đường cũ đã xóa từ v0.8.9

Không còn ChatPing, Auto Chat, click chuột giả chat, nhập Unicode giả, calibration chat, UIRect pointer-event giả/null.

## Build

Chạy `build.cmd` với Zig 0.15.2. Output:

`dist\ThanLongAutoTrain_v0.9.0.exe`
