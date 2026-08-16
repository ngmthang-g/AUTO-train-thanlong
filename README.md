# Than Long Clean Route v1.5.10 — Internal Confirm + Revive v0.8.7

## Mục tiêu bản này
v1.5.10 giữ nền/route/rotation/AutoSell của v1.5.9 nhưng thay **toàn bộ phần Xác nhận ra map và Đầu thai** bằng đúng hướng callback nội bộ đã chạy mượt ở donor v0.8.7.

### Đã bỏ hoàn toàn
- tọa độ click `XÁC NHẬN`;
- checkbox `Auto XN map`;
- `ConfirmIntervalSec` / timer click Confirm định kỳ;
- tọa độ click `ĐẦU THAI`;
- `ClickSlot::Confirm` và `ClickSlot::Revive`;
- generic bridge `ClickAt` không còn được dùng.

### 4 điểm click vẫn giữ
1. `AUTO`;
2. `ĐÁNH QUÁI`;
3. `DỪNG AUTO 1`;
4. `DỪNG AUTO 2`.

Auto Sell vẫn có macro/tọa độ riêng như v1.5.9; thay đổi này không xóa nghiệp vụ bán đồ.

## Xác nhận ra map — donor v0.8.7
Bridge chạy trên game-window thread và chỉ mở action khi đúng GameAssembly donor:
- PE `TimeDateStamp = 0x6A410C14`;
- `SizeOfImage = 0x03DCB000`;
- byte-signature của donor UI (gồm `MainFindUI`, `UIObject`, `UIButton`, `UIButton.HandleClickEvent`) phải khớp; detector MessageBox chỉ gọi `MainFindUI` như 0.8.7.

Observer giữ một GC-rooted managed string `MessageBox` như donor, không cấp phát chuỗi mới ở mỗi poll.

Luồng:
`tool-owned cross-map StartPath → MainFindUI("MessageBox") → moving=OFF → debounce 200ms → StopPath nếu cần → snapshot mới ≥250ms → tìm UIButton đồng ý duy nhất → UIButton.HandleClickEvent() → Transition Freeze`.

Không có MessageBox authoritative, route không thuộc tool, UI signature sai hoặc nút mơ hồ => **không click**.

## Đầu thai — donor v0.8.7
Mỗi lần chuẩn bị bấm, Bridge đọc lại `get_IsDeath` ngay trên callback thread. Chỉ khi `IsDeath=true` mới quét `UIObject.instances`, lọc UIButton active/interactable, tìm duy nhất `Đầu thai` (có fallback đọc descendant label), rồi gọi `UIButton.HandleClickEvent()`.

Các entry UI đã disposed bị loại trước khi gọi class/UI API để giảm nguy cơ đụng stale object.

## Những phần v1.5.9 giữ nguyên
- vòng xoay nhiều bãi train;
- death-session cold restart;
- Map 87 Địa Phủ AutoFight-OFF guard;
- route ownership cleanup;
- Transition Freeze + Bridge request serialization;
- AutoFight watchdog 60 giây;
- check tọa độ train 3 phút;
- robust mount fallback;
- Auto Sell;
- shared spots và setting riêng từng account.

## Validation hiện tại
- route/mount pure self-test: **15/15 PASS**;
- rotation pure self-test: **8/8 PASS**;
- static source audit: **PASS**;
- lexical brace/paren balance: **PASS**.

Môi trường đóng gói hiện tại không có Zig/Windows toolchain nên **controller/Bridge v1.5.10 chưa được compile thành PE trong phiên này**. Chạy `build.cmd` trên Windows có Zig để thực hiện audit + self-test + build DLL/EXE. Runtime với client thật vẫn cần test trước khi coi là PASS.
