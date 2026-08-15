# Thần Long Auto - NewCore v1.2.1 Revive Dispatcher Hotfix

v1.2.1 sửa lỗi live của v1.2.0: sau khi nhân vật chết, foundation/scanner/MainThread/ActionQueue đều PASS nhưng command `InvokeReviveButton` không trả response trong 5 giây và client bị diss/unresponsive.

Bằng chứng v1.2.0 **không xác định chính xác sub-step nào treo** vì command 7 chưa có breadcrumb nội bộ. Surface rủi ro là toàn bộ Revive command chạy đồng bộ trong `WH_GETMESSAGE`: quét `UIObject.instances` bằng nhiều managed invocation rồi gọi `UIButton.HandleClickEvent()` trực tiếp. v1.2.1 loại bỏ cả hai rủi ro thay vì tăng timeout.

Action gameplay được mở duy nhất vẫn là **Revive / Đầu thai**. AutoFight, NPC, Sell, Path, Treatment, Buff vẫn khóa.

## Pipeline

`Resolver -> Scanner(read-only) -> Snapshot/State Store -> Observer -> FSM -> SafetyGuard -> ActionQueue(MAX=1) -> short WH_GETMESSAGE ingress -> MainThread.Execute(System.Action) -> game Update -> WAIT REAL POST -> VERIFY`

Core invariant:

`READ -> DECIDE -> VALIDATE -> ACTION -> WAIT REAL STATE CHANGE -> VERIFY -> NEXT`

## Proven base

Kế thừa nguyên v1.1.0/v1.2.0:

- IL2CPP/native foundation;
- continuous read-only scanner + 60/60;
- flags/identity map-transition guard + recovery 2/2;
- FSM + SafetyGuard;
- ActionQueue capacity exactly 1;
- harmless WH_GETMESSAGE/main-thread envelope + token/thread POST;
- current-client metadata probe v3;
- real death observation `dead=0→1`.

## Live v1.2.0 failure retained as evidence

Live PID 23432 đạt:

`60/60 -> harmless proof PASS -> dead=0→1 -> REVIVE PRE -> ActionQueue active=1 -> gửi InvokeReviveButton`

Sau đó command 7 không hoàn thành trong transport timeout 5 giây. Controller fail-closed, clear queue, không retry. Đây là runtime FAIL của v1.2.0; PR v1.2.0 không được merge.

Không tăng timeout trong v1.2.1.

## Protocol

- `kProtocolVersion = 0x00010201`
- `ReadGameSnapshot = 5`
- `ProveHookActionEnvelope = 6`
- `InvokeReviveButton = 7`

## Exclusive mutation boundary

- global `kGameplayMutationEnabled=false`;
- exclusive `kReviveMutationEnabled=true`;
- ActionQueue MAX=1;
- AutoFight/NPC/Sell/Path vẫn bị global dispatcher gate khóa.

## Bounded fresh UIButton resolver

Command 7 vẫn fresh-resolve UI, không cache pointer, nhưng hook work được giới hạn cứng:

- hard time budget **120 ms**;
- hard entry cap **4096**;
- lọc class UIButton trước;
- chỉ đọc visible `Text` cần thiết;
- chỉ match exact `Đầu thai`, `Hồi sinh`, `Revive`, `Relive`, `Respawn`;
- candidate phải active + interactable;
- PRE được đọc lại trước dispatch;
- hết budget/không thấy candidate => block fail-closed, **không click**.

## Không gọi HandleClickEvent trực tiếp trong WH_GETMESSAGE

v1.2.1 cấm source pattern:

`g_api.runtime_invoke(handleClick, ...)`

Thay vào đó bridge:

1. resolve fresh UIButton + `HandleClickEvent()` metadata;
2. tạo reflection objects cho `System.Action` và `MethodInfo`;
3. dùng `System.Delegate.CreateDelegate(System.Type,System.Object,System.Reflection.MethodInfo)` để tạo Action bound vào fresh UIButton;
4. resolve `FGStudio.Engine.Utilities.MainThread.get_Instance()`;
5. gọi `MainThread.Execute(System.Action)` để **queue** Action;
6. command 7 trả ACK ngay sau khi queue;
7. callback thật được game lấy từ `waitToBeProcess` và chạy ở `MainThread.Update`, không chạy đồng bộ trong hook.

ACK yêu cầu `queued=1` và `directInvoked=0`. Return của `MainThread.Execute` vẫn không được coi là gameplay success.

## Runtime POST

Success chỉ là observer state:

- same RoleID;
- `dead=0`;
- `MapReady=1`;
- `WaitingChangeMap=0`;
- 2 snapshot sống ổn định liên tiếp.

MapID được phép đổi do Đầu thai. Nếu transition xảy ra, active Revive được preserve duy nhất để chờ POST; mọi mutation mới vẫn khóa.

## Timeout breadcrumbs

Bridge ghi stage sống vào shared memory. Nếu command vẫn bị kẹt, controller báo:

`REVIVE COMMAND TIMEOUT • stage=N objects=X buttons=Y elapsedMs=Z`

Stage:

1 FreshPre; 2 ResolveTypes; 3 Instances; 4 Enumerator; 5 ScanButtons; 6 Candidate; 7 DelegateReflection; 8 CreateDelegate; 9 MainThreadInstance; 10 MainThreadEnqueue; 11 Queued.

Nhờ vậy lỗi tiếp theo sẽ được khoanh đúng sub-step thay vì `Probe timeout` mù.

## Automated gates

- Integrated Control Self Test: **48 checks**.
- Revive Control Self Test: **18 checks**.
- direct HandleClickEvent invoke in Revive engine: **0**.
- deferred `MainThread.Execute(System.Action)` bắt buộc.
- resolver budget 120ms bắt buộc.
- forbidden architecture token audit giữ nguyên.

## Runtime acceptance

Một lượt live duy nhất:

`60/60 -> harmless proof PASS -> chết -> dead 2/2 -> bounded fresh Đầu thai -> REVIVE ACTION QUEUED -> ACK directInvoke=0 -> game Update executes action -> optional map recovery -> dead=0 2/2 -> REVIVE POST PASS -> queue 0/1`

Nếu resolver/dispatcher block, gửi nguyên log stage/reason; không retry mù và không quay lại foundation đã PASS.

## Donor boundary

Legacy v0.9.0 (pre-NewCore) chỉ donor semantics/data. Không RemoteExecutor, gameplay CreateRemoteThread, il2cpp_thread_attach, sleep workflow, raw RVA executor, cached UI pointer hay blind retry.