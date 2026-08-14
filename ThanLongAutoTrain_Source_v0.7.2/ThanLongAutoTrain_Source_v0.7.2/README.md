# Thần Long Mobile - Auto Train v0.7.2

Bản v0.7.2 tiếp tục từ v0.7.1 và tập trung dọn luồng train/ngựa, chỉ giữ Đầu thai khi chết, làm gọn giao diện và xóa code/thông tin mô tả không còn cần thiết.

## Thay đổi chính

- Chế độ train cũ dựa vào phím nền đã được loại khỏi giao diện, cấu hình và runtime; AUTO chỉ điều khiển qua chuỗi nội bộ `AUTO → Đánh quái / Dừng`.
- Chế độ train còn 2 lựa chọn: **Dùng skill đã chọn** và **AUTO → Đánh quái**.
- AUTO → Đánh quái vẫn gọi control nội bộ (UIButton / UIToggle / UIRect-Lua) và chỉ coi thành công khi cờ trạng thái đánh quái thật sự ON.
- Khi rời bãi, đi bán đồ, dừng tool, chuyển map an toàn hoặc cần ngắt train: chế độ AUTO dùng đúng chuỗi **AUTO → Dừng**; không sửa trực tiếp backing state.
- Logic lên ngựa: gọi ngựa → chờ **4 giây** → nếu vẫn chưa lên ngựa thì đánh tiếp khoảng **10 giây** → với AUTO sẽ gọi **AUTO → Dừng** → thử lên ngựa lại. Chu trình lặp cho đến khi lên được ngựa hoặc trạng thái khác ưu tiên hơn.
- Trong 10 giây dọn quái, nếu chọn AUTO thì giữ AUTO → Đánh quái; nếu chọn skill thì tiếp tục dùng skill đã chọn.
- Chờ xuống ngựa vẫn giữ 6 giây; yêu cầu 4 giây chỉ áp dụng cho **lên ngựa**.
- Khi nhân vật chết, resolver chỉ nhận đúng nhãn **Đầu thai**; không còn nhánh nút chết thứ hai.
- Tự bán đồ tiếp tục dùng `GetFreeBagSpace()` thật, NPC ResID + RoleID hiện tại, callback shop an toàn và giới hạn 90 lượt.
- Dòng trạng thái được dịch sang trái và mở rộng; hiển thị đầy đủ **Túi đồ trống: N ô**.
- Tab GIỚI THIỆU: **Phần mềm được thiết kế bởi Thắng Nguyễn - ĐỒ LONG**.
- Bỏ các dòng mô tả tĩnh không có tác dụng thao tác như “Mỗi PID…”, “Bãi đã tick…”, mô tả phụ ở tab giới thiệu và mô tả NPC.
- Xóa các wrapper test cũ/dead code còn sót; compile với `-Wall -Wextra -Werror` không còn cảnh báo.

## Tương thích cấu hình cũ

Giá trị profile cũ được giữ tương thích: `TrainActivation=1` là dùng skill, `TrainActivation=2` là AUTO. Giá trị mode cũ `0` sẽ tự chuyển sang AUTO.

## Build Windows

Chạy `build.cmd` với Zig trong PATH.

Kết quả: `dist\ThanLongAutoTrain_v0.7.2.exe`.

Workflow GitHub Actions dùng wildcard `dist/ThanLongAutoTrain_v*.exe`, nên không còn hardcode version khi verify/upload artifact.

## Kiểm tra tĩnh

- `main.cpp`: C++17 + `-Wall -Wextra -Werror` PASS.
- `remote_worker.S`: Windows x64 COFF PASS.
- Không còn phím train nền/direct AUTO setter; khi chết resolver chỉ nhận đúng `Đầu thai`.

## Cần test trong client thật

- AUTO → Đánh quái / AUTO → Dừng trên UI server hiện tại.
- Chu kỳ 4 giây lên ngựa → 10 giây dọn quái → thử ngựa lại.
- Shop UIRect-Lua và bán nhanh trong client thật.
