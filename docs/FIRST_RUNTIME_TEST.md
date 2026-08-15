# One-shot Runtime Acceptance - NewCore v1.2.0 Revive Only

Không test từng phase nhỏ nữa. CI/self-test phải xử lý toàn bộ phần có thể mô phỏng; người dùng chỉ cần một lượt live để chứng minh fresh UIButton resolution + callback thật + POST thật.

## Trước khi chạy live

GitHub CI bắt buộc PASS:
1. architecture audit: global gameplay mutation FALSE, Revive ONLY TRUE, ActionQueue MAX=1;
2. harmless-envelope audit;
3. revive-engine audit: đúng một direct `runtime_invoke(handleClick)` và không có action khác;
4. integrated control self-test 48/48;
5. Revive control self-test 16/16;
6. bridge build + LoadLibrary;
7. controller build + artifact verify.

## Một lượt test duy nhất

1. Chạy tool, tick client và bấm **Kiểm tra nền + Scanner**.
2. Đứng ổn định tới `SCANNER CORE QUALIFIED 60/60`.
3. Harmless proof phải PASS như v1.1.0:
   - `ACTION QUEUE ... active=1`
   - `HARMLESS ACTION ENVELOPE PASS`
   - `HARMLESS DISPATCHER PROOF PASS`
   - queue trở về `0/1`.
4. Cho nhân vật chết **một lần**. Không bấm Đầu thai thủ công trừ khi tool báo Revive BLOCKED.
5. Mong đợi:
   - `dead=0→1`
   - `REVIVE PRE QUALIFY 1/2`
   - sau snapshot chết ổn định thứ hai: SafetyGuard/Dispatcher PASS
   - `ACTION QUEUE ... active=1 • intent=Revive`
   - gửi `InvokeReviveButton`
   - `REVIVE ACTION DISPATCHED • fresh UIButton=“Đầu thai” ... HandleClickEvent() called ONCE`
   - `REVIVE DISPATCH ACK PASS ... NO RETRY`
6. Nếu game đổi map do Đầu thai:
   - transition vẫn hoạt động bình thường;
   - log phải cho biết active Revive được PRESERVED chỉ để chờ POST;
   - không có lần gọi HandleClickEvent thứ hai;
   - map recovery vẫn phải 1/2 -> 2/2.
7. Sau khi game sống lại:
   - `dead=1→0`
   - `REVIVE POST QUALIFY 1/2`
   - snapshot sống ổn định thứ hai -> `REVIVE POST PASS 2/2`
   - ActionQueue trở về `0/1`.

## Fail-closed hợp lệ

Nếu resolver không tìm thấy đúng một UIButton revive active+interactable, log `REVIVE BLOCKED FAIL-CLOSED` là hành vi an toàn. Tool không được chọn đại, không retry spam trong cùng death episode và không được click tọa độ.

Scanner lỗi, RoleID đổi, callback/thread evidence sai hoặc POST timeout đều phải fail closed; callback không được gửi lại.

## Chưa test ở v1.2.0

AutoFight, NPC, Sell, Path, Treatment, Buff không nằm trong runtime acceptance này vì vẫn khóa. PASS v1.2.0 chỉ đóng gate của action thật đầu tiên: **Revive / Đầu thai**.
