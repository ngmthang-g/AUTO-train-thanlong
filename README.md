# Thần Long Mobile - Auto Train v0.8.3

Bản này tập trung sửa ba lỗi runtime của v0.8.2: NPC đã đứng sát nhưng không mở được, AUTO mode chặn luôn navigation khi cờ AutoFight còn ON nhưng không tìm được nút AUTO, và bổ sung tuyến cổng riêng cho train Liên Server.

## 1. NPC bán đồ / trị liệu

`GetNearestNPC()` trả `LuaMapObjectData` có RoleID/Name, nhưng `ClickNPC(int)` trong native client so tham số với `GNPC.ResID`. Vì vậy RoleID chỉ dùng để nhận diện NPC gần nhất, không được truyền trực tiếp vào `ClickNPC`.

v0.8.3 xử lý NPC tĩnh theo namespace RoleID của client (`1,000,000,000 + ResID`) nhưng **không tin phép trừ một cách mù**: candidate ResID luôn được kiểm tra lại bằng chính `GetNearestNPC(resID)` và phải trả đúng RoleID/Name. Nếu không xác minh được, tool mới fallback sang `SessionData.NPCs/MovingNPCs` cho NPC đặc biệt/di động.

Khi lưu, dòng trạng thái hiện cả RoleID và ResID. Khi tới NPC, ResID đã lưu cũng được xác minh lại trước `ClickNPC(ResID)`. Hai danh sách bán đồ/trị liệu vẫn độc lập.

## 2. Train Liên Server từ Map 10000

Nếu nhân vật đang ở Lạc Dương Liên Server (`MapID 10000`) và bãi đích là một trong ba khu Liên Server, navigation dùng cổng trung gian trước rồi mới đi tọa độ train:

- Thanh Liên Trại: cổng `10000 / 15600 / 8250`; target MapID đã biết `10005`.
- Khô Vinh Đạo: cổng `10000 / 8195 / 1190`.
- Phàm Liên Trại: cổng `10000 / 1215 / 8475`.

MapID đích của Khô Vinh Đạo và Phàm Liên Trại chưa được cung cấp trong yêu cầu, nên source không bịa số. Hai route này được nhận biết bằng tên bãi lưu (`Khô Vinh Đạo/KVD`, `Phàm Liên Trại/PLT`). Khi có MapID thật, chỉ cần bổ sung vào `ResolveCrossServerPortal()` để nhận diện hoàn toàn bằng ID.

Chuỗi: Map 10000 → đi đúng cổng → chờ MessageBox thật → xác nhận nội bộ → chờ map transition/MapReady ổn định → mới AutoPath tới tọa độ bãi đã lưu.

## 3. AUTO → Đánh quái không còn chặn đường đi

Lỗi v0.8.2 là dùng cờ `AutoFight=ON` làm điều kiện bắt buộc phải tìm `AUTO → Dừng` trước khi gửi AutoPath/ngựa/cổng. Khi UI AUTO không định vị được, session đứng im mãi. v0.8.3 bỏ deadlock này.

Navigation có quyền ưu tiên tuyệt đối. Khi chưa ở bãi, tool không quét/mở AUTO để điều khiển combat. Nếu AUTO trước đó chính session này đã xác minh bật thành công thì khi rời bãi chỉ thử `AUTO → Dừng` đúng một lần theo kiểu best-effort; dù không tìm được UI, AutoPath/ngựa/cổng vẫn tiếp tục.

Ở bãi, AUTO activation dùng thứ tự giống nhánh v0.7.x: tìm `Đánh quái` đang sống trước; nếu chưa có mới mở root AUTO; chỉ chấp nhận thành công khi `get_EnableAutoF1()==ON`. Sau 3 thất bại retry giãn 15 giây. Không gọi UIRect/Lua mù cho AUTO combat.

Pha lên ngựa/đi map dùng chung state machine với chế độ `Dùng skill đã chọn`. Nếu sau 4 giây vẫn chưa lên ngựa, chỉ dùng selected-skill cleanup khi có skill được cấu hình; không mở AUTO trong pha navigation.

## Build

Chạy `build.cmd` trên Windows hoặc workflow GitHub.

Output: `dist\ThanLongAutoTrain_v0.8.3.exe`

Workflow tìm `ThanLongAutoTrain_v*.exe`, không hardcode version.

## Test ưu tiên

1. Lưu lại NPC bán đồ và NPC trị liệu. Status phải có cả `RoleID ... • ResID ...`.
2. Cho full túi và chạy tới NPC; kiểm tra không còn lỗi `Không thấy RoleID ... trong SessionData...`.
3. Test bãi Thanh Liên Trại từ Map 10000: phải đi cổng 15600/8250 trước, qua map rồi mới đi bãi.
4. Test AUTO mode từ map khác: AutoPath/ngựa phải phản hồi ngang chế độ Skill, không đứng chờ `AUTO → Dừng`.
5. Nếu tới bãi AUTO vẫn không mở/chọn Đánh quái, giữ log “đang mở ... nút” để bắt đúng control runtime; không bật lại UIRect callback toàn cục.
