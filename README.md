# Thần Long Mobile - Auto Train v0.8.2

Bản này sửa hai lỗi runtime lớn của v0.8.1: NPC đã tới đúng tọa độ nhưng game báo **“Không tìm thấy NPC tương ứng!”**, và chế độ `AUTO → Đánh quái` can thiệp vào lúc đi map/ngựa khiến luồng chậm, treo hoặc dễ diss hơn chế độ `Dùng skill đã chọn`.

## Sửa NPC bán đồ / trị liệu

Phân tích native của đúng `GameAssembly.dll` xác nhận `LuaSystemAPI_Game.ClickNPC(int npcID)` **không nhận RoleID**. Predicate bên trong `ClickNPC` so tham số với `GNPC.ResID` (`candidate + 0xC0`), đúng cùng offset mà `GNPC.get_ResID()` trả về.

v0.8.2 vì vậy đổi đường NPC thành:

1. `GetNearestNPC()` lấy object gần nhất và RoleID/Name để nhận diện nhanh.
2. Tìm object GNPC/GMovingNPC tương ứng trong `SessionData.NPCs` / `SessionData.MovingNPCs` bằng RoleID.
3. Lấy và xác minh **ResID thật** qua `GNPC.get_ResID()`.
4. Lưu `Name + ResID + RoleID + MapID + X + Y`.
5. Khi mở NPC, xác minh `GetNearestNPC(ResID)` còn thấy đúng loại NPC trong phạm vi tương tác.
6. Gọi `ClickNPC(ResID)` — không còn truyền RoleID.

File NPC cũ chưa có ResID vẫn được hỗ trợ: khi đứng gần NPC, tool tự resolve RoleID → ResID hiện tại trước khi mở. NPC bán đồ và NPC trị liệu vẫn dùng hai file độc lập.

## Sửa AUTO → Đánh quái / đi map

Từ v0.8.2, navigation có quyền điều khiển độc quyền khi nhân vật chưa ở bãi. AUTO UI không được mở/quét trong lúc:

- AutoPath đang đi;
- lên/xuống ngựa;
- chờ qua cổng / map đang load;
- hồi phục sau treo / death UI đang dựng lại;
- pha 10 giây dọn quái trước khi thử lên ngựa lại.

Pha dọn quái khi chưa lên được ngựa sử dụng đường `Dùng skill đã chọn` nếu có skill cấu hình; **không mở AUTO**. Vì vậy phần đi map/ngựa của hai chế độ dùng chung state machine và không còn gửi callback AUTO song song với AutoPath/ngựa.

Nếu AutoFight vẫn ON khi cần rời bãi, tool dừng path trước, chạy `AUTO → Dừng`, chờ một scan mới xác nhận OFF rồi mới cho navigation tiếp tục. Không gửi AutoPath và callback AUTO trong cùng một pha.

Ở lúc vừa tới đúng bãi, AUTO mode giờ **xuống ngựa trước đúng cùng nhánh với `Dùng skill đã chọn` và chờ 4 giây**, không mở/quét `AUTO` để dọn state trong lúc mount transition còn hoạt động. Chỉ ở scan kế tiếp khi `riding=false` thì mới được xét AUTO combat.

Ở đúng bãi + đã xuống ngựa, `AUTO → Đánh quái` chạy theo chuỗi xác định:

`UIButton AUTO → chờ submenu → UIButton/UIToggle Đánh quái → xác minh get_EnableAutoF1() = ON`

Không quét/call UIRect-Lua trong production path của AUTO. Sau 3 lần thất bại, retry được giãn 15 giây để tránh spam UI và diss.

## Một kết luận tĩnh quan trọng

`LuaSystemAPI_Game.AutoSetFlag(int RangerAuto)` **không phải API chọn “Đánh quái”**. Disassembly cho thấy tham số được truyền vào `MapRenderer.DrawCicleAutoFight(64, range, range-5, position)`, tức là bán kính/vòng hiển thị auto. v0.8.2 không dùng API này để bật combat và không đoán một flag sai.

## Build

Chạy `build.cmd` trên Windows hoặc `.github/workflows/build-windows.yml`.

Output:

`dist\ThanLongAutoTrain_v0.8.2.exe`

Workflow dùng wildcard `ThanLongAutoTrain_v*.exe`, không hardcode version.

## Runtime test cần thực hiện

1. Lưu lại một NPC bán đồ và một NPC trị liệu bằng v0.8.2; kiểm tra file có ResID > 0.
2. Đi full túi tới NPC và xác nhận không còn popup “Không tìm thấy NPC tương ứng!”.
3. Test trị liệu đủ `Trị liệu → Xác nhận → Ta biết rồi`.
4. Chọn AUTO → Đánh quái, đi khác map ít nhất 3 lần; so tốc độ map/ngựa với `Dùng skill đã chọn`.
5. Nếu AUTO mở root nhưng vẫn không chọn được `Đánh quái`, cần bắt runtime đúng class/name/handler của control con một lần; không nên bật lại quét UIRect toàn cục.
