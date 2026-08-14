# Thần Long Mobile - Auto Train v0.7.0

Bản v0.7.0 tiếp tục trực tiếp từ v0.6.0 cho đúng cặp `GameAssembly.dll` + `global-metadata.dat` người dùng cung cấp ngày 14-08-2026. Tool vẫn giữ mô hình nhiều PID độc lập, kiểm tra đúng PE/game build trước khi gọi IL2CPP và không sửa file game trên ổ đĩa.

## Thay đổi chính từ v0.6.0

- Bỏ hoàn toàn hai nút test **ĐẦU THAI NỘI BỘ** và **AUTO → ĐÁNH QUÁI NỘI BỘ** khỏi giao diện chính. Hai chức năng chỉ còn chạy tự động trong state machine.
- Resolver `AUTO → Đánh quái / Dừng` quét cả `UIButton` và `UIToggle`; gọi `UIButton.HandleClickEvent()` hoặc `UIToggle.set_Selected(true)` theo control thật.
- Không gọi `UIRectTransform.HandlePointerClick()` bằng `PointerEventData=null/fake`; nhánh pointer-click có dereference event data và bị chặn để tránh crash/disconnect.
- Đọc trực tiếp `LuaSystemAPI_Game.GetFreeBagSpace()` để lấy số ô trống tay nải, không cần mở túi/vuốt/đếm hình ảnh.
- Thêm **Tự bán đồ / chu kỳ kiểm tra / NPC** riêng theo từng RoleID/PID.
- Nút **LƯU NPC GẦN NHẤT** lấy `GetNearestNPC()`, đọc `LuaMapObjectData.RoleID` + `Name`, lưu cùng MapID/X/Y realtime. File NPC dùng chung; mỗi nhân vật chọn NPC riêng.
- Khi tay nải full (`GetFreeBagSpace=0`), đúng PID đó tắt train/AutoPath, đi NPC, gọi `ClickNPC(npcID)`, mở **Bán vật phẩm**, bật **Bán vật phẩm nhanh**, rồi chỉ gọi item cell khi chứng minh được đó là `UIButton` thuộc cây bag/inventory.
- Nếu item cell chỉ là `UIRectTransform` hoặc cây UI mơ hồ, tool chuyển **BÁN ĐỒ TẠM DỪNG AN TOÀN** và không bấm mù.
- Khi `GetFreeBagSpace` tăng, tool quay lại bãi và tiếp tục train.
- Dòng trạng thái bổ sung `Túi trống: N` theo lần kiểm tra gần nhất; không gọi API túi ở mọi vòng worker.

## RVA mới đã đối chiếu

| API | RVA |
|---|---:|
| `LuaSystemAPI_Game.GetFreeBagSpace()` | `0x6716F0` |
| `LuaSystemAPI_Game.GetNearestNPC()` | `0x673A90` |
| `LuaSystemAPI_Game.ClickNPC(int npcID)` | `0x66ADC0` |
| `LuaMapObjectData.get_RoleID()` | `0x41F000` |
| `LuaMapObjectData.get_Name()` | `0x41F3F0` |
| `UIObject.get_Parent()` | `0x530270` |
| `UIToggle.get_Interactable()` | `0x688580` |
| `UIToggle.get_Selected()` | `0x6885D0` |
| `UIToggle.get_Text()` | `0x688710` |
| `UIToggle.set_Selected(bool)` | `0x6888E0` |

Các RVA cũ của v0.6.0 cho RoleData/Map/X/Y/ngựa/AutoPath/IsDeath/WaitingChangeMap/MessageBox/UIButton vẫn giữ nguyên và tiếp tục được kiểm tra chữ ký runtime.

## Logic Tự bán đồ

1. Khi bắt đầu và bật Tự bán đồ, `GetFreeBagSpace()` được probe ngay một lần.
2. Nếu còn ô trống, train bình thường và **không probe lại ở mọi vòng worker**; lần check tiếp theo đúng theo số phút trên giao diện.
3. Nếu bằng 0, đúng PID đó dừng đánh và đường cũ ngay.
4. Tool đi NPC bằng cùng cơ chế AutoPath/chat ping + guard chuyển map đang dùng cho bãi.
5. Tới NPC: dừng path, xuống ngựa, gọi `ClickNPC(RoleID)`.
6. Quét UI nội bộ để chọn **Bán vật phẩm**, rồi bật **Bán vật phẩm nhanh** bằng UIButton/UIToggle thật.
7. Item cell chỉ được invoke khi tên control + chuỗi parent chứng minh nó thuộc bag/inventory. Tối đa 90 lượt; tiếp tục quét sau mỗi món bán được để tránh tình trạng chỉ bán 1 món rồi quay về.
8. Nếu cây item biến mất sau khi đã giải phóng được ô, chuỗi được coi là đã bán thành công; nếu chưa bán được gì thì tool dừng an toàn và không bấm mù.
9. Sau chuỗi bán, tool chỉ thử đóng nút **Đóng/Close** nếu nút đó thuộc cây shop và được nhận diện duy nhất; sau đó quay về đúng bãi và bật train lại.

## Lưu NPC bán đồ

Đứng sát NPC cần dùng → chọn đúng cửa sổ game → bấm **LƯU NPC GẦN NHẤT**. Tool ghi:

`Tên NPC<TAB>RoleID<TAB>MapID<TAB>X<TAB>Y`

vào `ThanLongAutoTrain.npcs.txt`. Không hardcode NPC ID đoán.

## AUTO → Đánh quái v0.7.0

- Mở `AUTO` bằng `UIButton.HandleClickEvent()` như v0.6.0.
- Control con **Đánh quái** và **Dừng** được tìm trong cả `UIButton` và `UIToggle`.
- Với `UIToggle`, tool kiểm tra Interactable, đọc Text/Selected rồi chỉ gọi `set_Selected(true)` khi đang off.
- Sau khi kích hoạt vẫn đọc `get_EnableAutoF1()` để xác nhận.
- Nếu nhiều control cùng điểm nhận diện, tool từ chối bấm thay vì chọn đại.

## Chức năng cũ giữ nguyên

- Multi-window: mỗi PID có session/worker/config/skill/bãi/state độc lập.
- Start/Stop chỉ áp dụng các cửa sổ được tick.
- Profile riêng theo RoleID; file bãi/NPC dùng chung.
- Sai tọa độ: tắt train ngay → hủy path → chạy về bãi → đúng tọa độ mới xuống ngựa/bật train.
- Chuyển map dùng `WaitingChangeMap` + `IsMapReady`, không chờ cứng 6 giây sau xác nhận/hồi sinh.
- Game treo/mất phản hồi/remote packet bận → tạm dừng an toàn, không ghi đè packet.
- Hồi sinh/Đầu thai chỉ khi `IsDeath=true`, gọi `UIButton.HandleClickEvent()` thật.
- Ba cách bật train: F1 nền, skill đã chọn, AUTO → Đánh quái nội bộ.
- Tab GIỚI THIỆU giữ “Phần mềm xây dựng bởi Nguyễn Mạnh Thắng.Long.”

## File cấu hình

- `ThanLongAutoTrain.spots.txt`: `Tên<TAB>MapID<TAB>X<TAB>Y`.
- `ThanLongAutoTrain.npcs.txt`: `Tên<TAB>RoleID<TAB>MapID<TAB>X<TAB>Y`.
- `ThanLongAutoTrain.ini`: profile `AutoTrain.Role<RoleID>`; thêm `AutoSell`, `BagCheckMinutes`, `SellNpcName`.

## Giới hạn cần test trong game thật

Phân tích tĩnh xác nhận RVA/chữ ký; source đã qua C++17 syntax + `-Wall -Wextra -Werror` trong môi trường kiểm tra. Layout Lua/UI động của server không có ở đây, nên lần test đầu cần quan sát:

1. `AUTO → Đánh quái` được nhận thành UIButton hay UIToggle và `EnableAutoF1` có ON.
2. `LƯU NPC GẦN NHẤT` trả đúng tên/ID NPC đứng cạnh.
3. Sau khi full túi, item cell của shop là UIButton hay UIRectTransform. Nếu là UIRectTransform, v0.7 cố ý dừng an toàn để lấy đúng event path ở bản sau, không giả click.

## Build source

Cài Zig có target `x86_64-windows-gnu`, thêm `zig` vào PATH rồi chạy `build.cmd`.
Kết quả: `dist\ThanLongAutoTrain_v0.7.0.exe`.
