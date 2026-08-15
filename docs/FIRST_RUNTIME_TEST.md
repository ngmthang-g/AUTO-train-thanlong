# Runtime test - v1.0.9 Scanner Qualification

## 1. Foundation

Tick client -> `Kiểm tra nền + Scanner`.

Expected:
- HOOK PASS
- IL2CPP metadata exports PASS
- FGStudio MainThread metadata PASS
- UnityMainThreadDispatcher metadata PASS
- MAINTHREAD PROVEN
- SNAPSHOT PASS

## 2. Stability qualification

Không thao tác game khoảng 30-40 giây.

Expected milestones:
- `SCANNER QUALIFYING 1/60`
- `SCANNER QUALIFYING 10/60`
- `SCANNER QUALIFYING 30/60`
- `SCANNER CORE QUALIFIED 60/60 (~30s)`

Không được báo qualified sau snapshot đầu.

Nếu query lỗi/partial/đổi map trong khoảng này, qualification phải reset thay vì giả PASS.

## 3. Moving edge

Đi bộ thủ công rồi đứng lại.
Expected `STATE EDGE` có `moving=0→1` và `moving=1→0`.

## 4. Riding edge

Lên/xuống ngựa thủ công.
Expected `STATE EDGE` cho `riding`.

## 5. AutoPath edge

Bật/tắt đường chạy tự động của game.
Expected `STATE EDGE` cho `autoPath`.

## 6. Dead/revive edge

Cho nhân vật chết và hồi sinh thủ công. Tool không được tự bấm gì.
Expected `STATE EDGE` cho `dead`.

## 7. Map transition guard

Đổi map thủ công.
Expected:
- `MAP TRANSITION`
- không fail vì LeaderRoleData/AutoPath trong lúc scene rebuild
- `MAP RECOVERY 1/2`
- `MAP RECOVERY PASS 2/2`
- qualification bắt đầu lại từ đầu sau transition

## 8. Coverage

Log `RUNTIME EDGE COVERAGE • pending=...` phải bỏ dần các mục:
`moving, riding, autoPath, dead, mapTransition`.

Khi hết: `pending=none`.

## Gate

Dù 60/60 + coverage PASS, v1.0.9 vẫn **không có gameplay action**. Kết quả này chỉ cho phép chuyển sang scaffold SafetyGuard + ActionQueue(max=1) + FSM ở phase tiếp theo.
