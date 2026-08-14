# Thần Long Mobile - Auto Train v0.8.1

Bản này tiếp tục trực tiếp từ v0.8.0 và tập trung vào bốn lỗi runtime được phản hồi: Auto chat chưa hoạt động, NPC bán đồ/trị liệu dùng chung dữ liệu, Auto buff thử lặp quá nhiều, và AUTO → Đánh quái bị hồi quy/treo game.

## Thay đổi chính

- Auto chat không còn dùng `UIFactory.SendDefaultChat()` làm đường chính. Tool thao tác đúng cây UI nội bộ của game: mở Chat → tìm `UIInput` → `UIInput.set_Text()` → gọi `Gửi tin nhắn` → gọi nút đóng Chat. Không đưa cửa sổ game lên trước và không dùng chuột thật.
- Trị liệu theo đúng chuỗi: mở NPC → `Trị liệu` → `Xác nhận` → `Ta biết rồi` → đóng UI.
- Auto buff: mỗi skill chỉ cast đúng **1 lần trong mỗi lượt kiểm tra 5 phút**. Nếu không xác minh được buff sau lần cast đó thì bỏ qua skill và chuyển tiếp.
- NPC bán đồ và NPC trị liệu đã tách hẳn thành hai kho dữ liệu:
  - `ThanLongAutoTrain.sell_npcs.txt`
  - `ThanLongAutoTrain.heal_npcs.txt`
  File `ThanLongAutoTrain.npcs.txt` cũ chỉ được dùng một lần làm dữ liệu tương thích cho danh sách bán đồ; danh sách trị liệu không còn lấy chung.
- AUTO → Đánh quái: phục hồi cách mở nút AUTO ổn định của bản cũ (`UIButton.HandleClickEvent()`), sau đó chỉ tìm `Đánh quái` bằng `UIButton/UIToggle`. **Không quét/call UIRect-Lua trong luồng AUTO** để tránh quét nặng/call nhầm control gây treo hoặc diss.
- AUTO chỉ báo thành công khi đọc lại `get_EnableAutoF1() = ON`; thất bại sẽ giãn retry 3 giây thay vì spam mỗi 1 giây.
- AUTO → Dừng dùng cùng nguyên tắc Button/Toggle và phải xác minh `AutoFight=OFF`.
- Toàn bộ di chuyển, qua map, Đầu thai, lên/xuống ngựa giữ nguyên state machine đang dùng tốt ở chế độ `Dùng skill đã chọn`; chế độ AUTO chỉ thay đúng đoạn kích hoạt/tắt combat.

## Build

Chạy `build.cmd` trên Windows hoặc dùng `.github/workflows/build-windows.yml`.
Output:

`dist\ThanLongAutoTrain_v0.8.1.exe`

Workflow dùng wildcard `ThanLongAutoTrain_v*.exe`, không hardcode version.

## Runtime test cần thực hiện

1. Auto chat: xác nhận bảng chat mở, text được điền, tin được gửi và bảng đóng.
2. Trị liệu: test đủ ba nút `Trị liệu → Xác nhận → Ta biết rồi`.
3. AUTO → Đánh quái: test 10–15 lần Start/Stop và ít nhất 3 lần đổi map để xem còn treo/diss không.
4. Nếu AUTO mở được nhưng không nhận diện `Đánh quái`, không bật fallback Rect-Lua ngay. Dùng log/runtime inspection để xác định class/handler của đúng control rồi mới thêm đường gọi riêng.
