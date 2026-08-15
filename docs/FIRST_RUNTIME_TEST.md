# Runtime test - v1.0.8 Continuous Read-Only Scanner

Mục tiêu: chứng minh donor-state parity và map-transition guard. Không test action vì action chưa tồn tại.

## 1. Khởi động

1. Build bằng `build.cmd`.
2. Mở game và vào map ổn định.
3. Chạy tool Administrator.
4. `Quét client`.
5. Tick đúng PID.
6. Bấm **Kiểm tra nền + Scanner**.

PASS nền mong đợi:

- `ValidateNative` PASS
- metadata MainThread/Dispatcher PASS
- `MAINTHREAD PROVEN`
- snapshot đầu PASS
- cuối cùng `SCANNER PASS`

Dòng snapshot phải có dạng gần giống:

`SNAPSHOT PASS seq=... role=... map=... pos=... HP=... bag=... dead=0 riding=0 moving=0 auto=0 autoPath=0 mapReady=1 waitingMap=0. READ-ONLY; action LOCKED.`

Nếu cuối cùng là `SCANNER PARTIAL@, gửi nguyên log; đặc biệt xem nó báo thiếu `position`, `moving` hay `autoPath`.

## 2. Test Moving

- Đứng im vài giây: `Moving=NO`.
- Tự kéo nhân vật chạy bằng game: `Moving=YES`.
- Dừng lại: về `Moving=NO`.

Log phải có edge `moving=0→1` và `moving=1→0`.

## 3. Test Riding

- Tự bấm lên ngựa bằng game.
- `Riding` phải chuyển NO -> YES.
- Tự xuống ngựa.
- `Riding` phải về NO.

Tool không được tự gọi ngựa.

## 4. Test AutoPath

- Dùng chính game để bắt đầu chạy đường tự động tới một vị trí/NPC.
- `AutoPath` phải lên ON.
- Khi đường dừng/kết thúc, phải về OFF.

Nếu luôn `?`, gửi log `SCANNER PARTIAL`; đó là probe cần sửa tiếp, không được fallback sang donor RVA.

## 5. Test Map transition - quan trọng nhất

- Tự qua cổng/đổi map bằng game.
- Trong lúc đổi map tool phải chuyển `CHỜ MAP ỔN ĐỊNH` hoặc log `SNAPSHOT TRANSITION`.
- Không được báo `LeaderRoleData=null` chỉ vì scene đang rebuild.
- Khi map ổn định, scanner phải qua `MAP RECOVERY 1/2`, rồi `MAP RECOVERY PASS 2/2`, sau đó mới trở về `SCANNER PASS/PARTIAL`.
- MapID/X/Y phải cập nhật theo map mới.

## 6. Test Dead

Nếu tiện test an toàn trong game:

- khi nhân vật chết, `Dead=YES`;
- sau khi tự hồi sinh bằng game, `Dead=NO`.

Tool không tự bấm Đầu thai/Hồi sinh ở v1.0.8.

## 7. Dấu hiệu phải dừng và gửi log

- `VALIDATOR FAIL`;
- `SCANNER PARTIAL` kéo dài;
- AutoPath luôn `?`;
- Moving/Riding không đổi theo thao tác thật;
- map transition làm bridge fault;
- game giật rõ hoặc disconnect đúng lúc scanner bắt đầu.
