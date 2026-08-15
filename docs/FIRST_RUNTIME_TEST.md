# Runtime test - v1.0.11 Unified Map Transition Guard

## 1. Stable scanner

Đứng trong một map khoảng 30-40 giây. Expected:
- `AUTOPATH PROBE • 5/5 resolved ... resolver PASS`
- `SCANNER CORE QUALIFIED 60/60`

## 2. Identity-source map transition

Đổi map theo cách trước đây chỉ làm MapID đổi nhưng flags không bật. Expected:
- `STATE EDGE • map=A→B`
- `MAP TRANSITION • source=identity • flags missed/delayed ...`
- runtime coverage bỏ `mapTransition` khỏi pending
- `MAP RECOVERY 1/2 • source=identity • candidate role=... map=...`
- chỉ PASS ở snapshot kế tiếp cùng identity: `MAP RECOVERY PASS 2/2 • source=identity ...`

Nếu MapID tiếp tục đổi A→B→C trong lúc recovery, expected `MAP RECOVERY RESET` rồi lấy C làm candidate mới.

## 3. Flag-source map transition

Nếu gặp kiểu chuyển map có `MapReady=0` hoặc `WaitingChangeMap=1`, expected:
- `MAP TRANSITION • source=flags`
- bridge vẫn chỉ scan MapReady/WaitingChangeMap trong transient scene
- `MAP RECOVERY 1/2 • source=flags`
- `MAP RECOVERY PASS 2/2 • source=flags`

Nếu flags đến muộn sau identity source, expected `MAP TRANSITION SIGNAL • source=flags joined existing source=identity`; không được tăng thành hai transition session độc lập.

## 4. Dead semantic

Cho nhân vật chết thủ công; tool không được tự bấm. Expected `STATE EDGE • dead=0→1` (và `1→0` nếu hồi sinh thủ công).

## Gate

Kết quả mong đợi cuối:
- scanner `60/60` trên map ổn định;
- AutoPath semantics proven;
- map transition proven từ flags hoặc identity;
- dead semantics proven;
- `RUNTIME EDGE COVERAGE • pending=none`;
- không có gameplay action.
