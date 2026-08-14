# Thần Long Mobile - Auto Train v0.8.0

v0.8.0 tiếp tục từ v0.7.2, tập trung sửa hồi quy NPC, hoàn thiện chuỗi tự bán đồ, thêm trị liệu / auto buff / auto chat và siết luồng Đầu thai khi bị PK chết lặp lại.

## Thay đổi chính

### NPC và tự bán đồ

- Sửa lỗi **Lưu NPC** của v0.7.2: `GetNearestNPC()` trả `LuaMapObjectData`; tool không còn ép object này sang `LuaMapSpriteData` hay bắt buộc ResID.
- Lưu NPC trở lại đường ổn định của v0.7.0: **RoleID + Name + MapID + X/Y**. Trường ResID trong file TXT chỉ được giữ để tương thích file cũ.
- Khi tới đúng tọa độ NPC, tool gọi lại `GetNearestNPC()`, so tên với NPC đã lưu rồi lấy **RoleID hiện tại** trước khi `ClickNPC()`. Nếu NPC gần nhất khác tên, tool dừng chuỗi thay vì mở nhầm NPC.
- Chuỗi bán: **NPC -> Mua thú cưỡi -> Bán vật phẩm -> Bán vật phẩm nhanh -> Trang bị**.
- Chỉ thao tác item nằm trong cây UI tay nải, không dựa vào tọa độ màn hình.
- Hỗ trợ item control dạng `UIButton` và `UIRectTransform` có Lua click handler.
- Mỗi item không bán được được thử tối đa **3 lần**, sau đó bỏ qua item đó; toàn chuỗi tối đa 90 callback.
- Bán xong đóng shop / tay nải bằng control Close thuộc đúng cây shop/bag rồi quay lại chu trình train.
- `GetFreeBagSpace()` tiếp tục là nguồn xác nhận số ô trống thật.

### Trị liệu

- Thêm **Trị liệu khi ra bãi** và NPC trị liệu lưu riêng.
- Khi bật, lúc bắt đầu / sau Đầu thai nhân vật sẽ đi NPC trị liệu trước khi ra bãi.
- Nếu vừa bán đồ xong và Trị liệu đang bật, tool cũng qua NPC trị liệu trước khi quay lại bãi.
- Metadata không có API C# riêng tên Treat/Heal NPC; thao tác trị liệu vì vậy dùng callback UI nội bộ và dừng an toàn nếu không nhận diện duy nhất.

### Auto buff

- Thêm danh sách chọn nhiều skill buff.
- Mỗi skill được cast rồi xác minh buff bằng `GetBuffs()` / `HasBuff()`; tool học quan hệ **skillID -> buffID** trong phiên chạy bằng cách so snapshot buff trước/sau.
- Nếu buff đã tồn tại thì chuyển sang skill tiếp theo.
- Một skill không tạo/refresh được buff sau **5 lần** sẽ bị bỏ qua trong lượt đó.
- Hết danh sách, tool kiểm tra lại toàn bộ sau **5 phút**.

### Auto chat

- Dùng thẳng `FGStudio.Game.Logic.UIFactory.SendDefaultChat(string)` để gửi nội dung vào kênh mặc định/đang dùng, không cần mở ô chat, gõ chữ và click Send bằng tọa độ.
- Có ô nội dung và chu kỳ phút riêng.
- Chưa khẳng định API chọn một kênh chat cụ thể vì hai file game hiện chỉ chứng minh được đường `SendDefaultChat`.

### AUTO -> Đánh quái

- Giữ nguyên engine đi bãi / qua cổng / Đầu thai / ngựa đã ổn ở chế độ **Dùng skill đã chọn**; hai chế độ chỉ khác ở bước kích hoạt chiến đấu.
- Control **AUTO gốc** giờ cũng được resolve đa loại `UIButton / UIToggle / UIRect-Lua`, không còn giả định AUTO root luôn là UIButton.
- `AUTO -> Đánh quái` chỉ thành công khi đọc lại `EnableAutoF1/AutoFight=ON`.
- Khi đi bán, trị liệu, rời bãi hoặc Stop: dùng đúng **AUTO -> Dừng** và xác nhận AutoFight=OFF.
- Không dùng direct setter AutoFight để giả trạng thái.

### Ngựa và chết lặp lại

- Chờ xuống ngựa: **4 giây**.
- Lên ngựa: gọi ngựa -> chờ 4 giây; chưa lên thì dọn quái khoảng 10 giây -> với AUTO phải `AUTO -> Dừng` -> thử lên ngựa lại.
- Khi chết do PK, tool không mở AUTO/menu ngay trên cạnh chuyển trạng thái chết. Nó chỉ dừng AutoPath, chờ `IsDeath + MapReady` ổn định hai lần rồi mới tìm mới nút **Đầu thai** và gọi callback.
- Sau khi gọi Đầu thai, khóa UI/path cho tới khi `IsDeath=false`; chỉ cho phép tối đa một retry muộn sau 15 giây và phải re-scan nút mới.
- Khi sống lại phải ổn định hai lần rồi mới guard map / trị liệu / ra bãi. Thiết kế này nhằm loại race với death overlay/stale UIObject khi chết nhiều lần liên tiếp.

### Giao diện

- Nút chức năng và chữ nút được thu nhỏ khoảng 60% so với kích thước trước.
- Đổi tên thành **LƯU NPC BÁN ĐỒ GẦN NHẤT**.
- Thêm **LƯU NPC TRỊ LIỆU GẦN NHẤT**.
- Dòng trạng thái giữ phần `Túi đồ trống: N ô` ở vùng rộng hơn.
- Tab giới thiệu: **Phần mềm được thiết kế bởi Thắng Nguyễn - ĐỒ LONG**.

## Build Windows

Cài Zig vào PATH rồi chạy:

```bat
build.cmd
```

Kết quả dự kiến:

`dist\ThanLongAutoTrain_v0.8.0.exe`

Workflow `.github/workflows/build-windows.yml` tự tìm `dist/ThanLongAutoTrain_v*.exe`, không hardcode version trong bước verify/upload.

## Kiểm tra source

- C++17 + `-Wall -Wextra -Werror`: PASS.
- Clang Static Analyzer: **0 diagnostics**.
- `remote_worker.S`: Windows x64 COFF PASS; worker vẫn hỗ trợ 5 native args đúng Win64 ABI.
- Toàn bộ 56 signature đang dùng để guard runtime đã đối chiếu byte với đúng `GameAssembly.dll` người dùng cung cấp: PASS.

## Phần cần test trong client thật

Các phần sau có nền tảng metadata/binary rõ nhưng vẫn phải test server/runtime trước khi gọi là hoàn thiện tuyệt đối:

- Tên/cây control thật của `Mua thú cưỡi`, `Bán vật phẩm`, `Bán vật phẩm nhanh`, `Trang bị`.
- Nút trị liệu của NPC cụ thể.
- Layout collection trả về từ `GetBuffs()` trên runtime và việc suy ra buff của từng skill.
- Control thực tế của `AUTO -> Đánh quái` trên UI server.
- Chuỗi Đầu thai khi bị PK chết lặp lại nhiều lần.
