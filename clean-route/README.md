# Than Long Clean Route v1.5.9 — Auto Rotate Train Spots

## Rotation bãi train
Mỗi acc có thể tick nhiều bãi từ dữ liệu bãi chung. Một bãi = giữ nguyên bãi đó; nhiều bãi = chạy vòng theo thứ tự danh sách và quay lại bãi đầu. Mặc định đổi bãi khi **chết quá 10 lần trong 10 phút** hoặc khi có **15 phút train thực** mà không xuất hiện lần FULL túi mới. Ba ngưỡng đều lưu riêng từng RoleID và chỉnh được trên giao diện.

Bộ đếm 15 phút chỉ chạy khi nhân vật sống, đứng đúng bãi, AutoFight ON và FreeBagSpace đọc được; không tính thời gian chạy đường/chuyển map/chết/freeze/F4. Khi túi chuyển sang FULL (`FreeBagSpace <= 0`) đồng hồ hiệu quả được reset. Nếu đổi bãi khi đang sống/đánh, tool dùng recovery hiện có để tắt AutoFight trước rồi mới route sang bãi mới.

## Fixed Interval Confirm
Cơ chế v1.5.8 vẫn giữ nguyên: `Auto XN map` click tọa XÁC NHẬN theo timer cố định và nhường quyền khi client/click-flow đang bận.

## Bridge
Core ReadState no longer calls `MainFindUI/FindUI("MessageBox")`. Map Confirm does not depend on `ValidConfirmUi`, popup visibility, route ownership, movement, portal detection, or semantic UI state.

## Other active behavior
- Map 87 Địa Phủ AutoFight-OFF guard.
- Transition Freeze + serialized Bridge requests.
- dual-boundary death-session cold restart.
- route ownership cleanup.
- 60-second AutoFight watchdog and 3-minute train-coordinate guard.
- Auto Sell, shared spots, six saved click coordinates, persistent per-account settings.

## Validation
Windows x64 staging CI **PASS** — run `31949891312`, job `95171640279`, artifact `9264382136`.
- route/mount self-test **15/15 PASS**
- rotation self-test **8/8 PASS**
- Bridge/EXE/package/artifact verification PASS

Runtime auto-rotation remains **NEEDS USER TEST**.
