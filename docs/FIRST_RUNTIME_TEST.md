# Runtime test - v1.0.10 AutoPath + Identity Guard

## 1. Foundation + Scanner

Expected:
- MAINTHREAD PROVEN
- SNAPSHOT PASS
- `SCANNER CORE QUALIFIED 60/60`

## 2. AutoPath diagnostic

Ở snapshot stable phải có log kiểu:

`AUTOPATH PROBE • 5/5 resolved • mask=31 • value=0 • resolver PASS`

Điều này chỉ chứng minh resolver/getter đọc thành công.

Sau đó **dùng chức năng tự chạy đường/AutoPath thật của game** (không phải AutoFight).
Expected một trong hai:
- `AUTOPATH OBSERVED ON ... semantics read-only PROVEN`, hoặc
- `STATE EDGE • autoPath=0→1`.

Nếu AutoPath thật đang chạy mà probe vẫn 5/5 nhưng value luôn 0, gửi log; khi đó semantics `get_IsAutoPathing` cần nghiên cứu lại, không được tự coi PASS.

## 3. Map identity-stable recovery

Đổi map vài lần.

Expected:
- `MAP TRANSITION`
- `MAP RECOVERY 1/2 • candidate role=... map=...`
- nếu map identity đổi muộn: `MAP RECOVERY RESET ...`
- chỉ PASS khi snapshot kế tiếp cùng identity:
  `MAP RECOVERY PASS 2/2 • same identity role=... map=...`
- sau recovery scanner qualification reset và chạy lại 60/60.

## 4. Transition flag miss / identity edge

Nếu RoleID/MapID đổi mà `MapReady/WaitingChangeMap` không báo transition, expected:

`IDENTITY REQUALIFY ... reset 60/60`

Tool vẫn không action.

## 5. Existing runtime edges

Moving / Riding / Dead / MapTransition đã PASS ở v1.0.9 nhưng có thể test lại.
Coverage cuối cần:
`RUNTIME EDGE COVERAGE • pending=none`

## Gate

v1.0.10 vẫn không có gameplay action. Chỉ khi:
- AutoPath semantics proven,
- identity recovery ổn định,
- scanner 60/60 PASS,
- coverage `pending=none`

thì mới chuyển sang scaffold `SafetyGuard + ActionQueue(MAX=1) + FSM`, vẫn khóa mutation ở bước đầu.
