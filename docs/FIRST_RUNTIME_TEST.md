# One-shot Runtime Acceptance - NewCore v1.2.1 Revive Dispatcher Hotfix

Không test lại từng phase. Foundation/scanner/map guard/harmless hook proof đã live PASS. Mục tiêu duy nhất của lượt này là chứng minh Revive không còn giữ `WH_GETMESSAGE` bằng action trực tiếp và game thực sự sống lại qua deferred MainThread queue.

## Trước live

CI bắt buộc PASS:
1. global gameplay mutation FALSE / Revive ONLY TRUE / ActionQueue MAX=1;
2. direct `runtime_invoke(handleClick,...)` = 0;
3. `Delegate.CreateDelegate` + `MainThread.Execute(System.Action)` present;
4. resolver budget 120ms;
5. ControlSelfTest 48/48;
6. ReviveSelfTest 18/18;
7. Bridge LoadLibrary + EXE verify.

## Một lượt duy nhất

1. Chạy tool, tick client, bấm **Kiểm tra nền + Scanner**.
2. Chờ `SCANNER CORE QUALIFIED 60/60`.
3. Harmless proof phải PASS và queue về 0/1.
4. Cho nhân vật chết một lần. Không bấm Đầu thai thủ công nếu tool đang xử lý.
5. Mong đợi:
   - `dead=0→1`
   - `REVIVE PRE QUALIFY 1/2`
   - `ACTION QUEUE ... active=1 • intent=Revive`
   - `gửi InvokeReviveButton`
   - `REVIVE ACTION QUEUED • fresh UIButton=“Đầu thai” • System.Action -> MainThread.Execute() • direct HandleClickEvent invoke=0 ...`
   - `REVIVE DISPATCH ACK PASS ... deferred MainThread queue • directInvoke=0`
6. Sau ACK, game tự thực thi Action từ `MainThread.Update`. Tool không gửi action lần hai.
7. Nếu Đầu thai đổi map:
   - active Revive được PRESERVED để chờ POST;
   - map recovery 1/2 -> 2/2;
   - không có Revive enqueue/callback lần hai.
8. Sau khi sống:
   - `dead=1→0`
   - `REVIVE POST QUALIFY 1/2`
   - `REVIVE POST PASS 2/2`
   - queue trở về 0/1.

## Nếu block

`REVIVE BLOCKED FAIL-CLOSED` là an toàn. Gửi nguyên reason cùng `stage / objects / buttons / elapsedMs`. Không click thủ công cho đến khi đã copy log nếu muốn chẩn đoán chính xác.

## Nếu timeout

v1.2.1 phải báo dạng:

`REVIVE COMMAND TIMEOUT • stage=N objects=X buttons=Y elapsedMs=Z`

Stage map:
- 1 FreshPre
- 2 ResolveTypes
- 3 Instances
- 4 Enumerator
- 5 ScanButtons
- 6 Candidate
- 7 DelegateReflection
- 8 CreateDelegate
- 9 MainThreadInstance
- 10 MainThreadEnqueue
- 11 Queued

Nếu timeout stage 5 thì lỗi nằm trong UI enumeration; stage 7/8 là delegate reflection; stage 9/10 là MainThread dispatcher. Không tăng timeout và không retry action.

## Chưa test

AutoFight, NPC, Sell, Path, Treatment, Buff vẫn khóa và không thuộc acceptance v1.2.1.