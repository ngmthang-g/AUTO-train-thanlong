# Thần Long Auto - NewCore v1.2.0 Revive Action

v1.2.0 là mốc NewCore đầu tiên mở **một gameplay mutation thật** sau khi scanner/observer/control-plane/hook-main-thread envelope đã được chứng minh bằng runtime ở các mốc trước.

Action được mở duy nhất: **Revive / Đầu thai**.

AutoFight, NPC, Sell, Path, Treatment, Buff và mọi action khác vẫn khóa.

## Pipeline

`Resolver -> Scanner(read-only) -> Snapshot/State Store -> Observer -> FSM -> SafetyGuard -> ActionQueue(MAX=1) -> proven Hook/Unity MainThread -> Internal Action Engine -> WAIT REAL POST -> VERIFY`

Core invariant vẫn là:

`READ -> DECIDE -> VALIDATE -> ACTION -> WAIT REAL STATE CHANGE -> VERIFY -> NEXT`

## Proven base inherited from v1.1.0

v1.2.0 không nghiên cứu lại foundation đã PASS:

- IL2CPP/native foundation;
- continuous read-only scanner;
- stable 60/60 qualification;
- map transition source=flags + source=identity;
- same-identity map recovery 2/2;
- AutoPath read-only resolver semantics;
- FSM + SafetyGuard;
- ActionQueue capacity exactly 1;
- harmless WH_GETMESSAGE / Unity-main-thread action envelope;
- live token/sequence/thread POST verification.

## Protocol

- `kProtocolVersion = 0x00010200`
- `ReadGameSnapshot = 5`
- `ProveHookActionEnvelope = 6` — infrastructure-only
- `InvokeReviveButton = 7` — first and only gameplay mutation command

## Exclusive mutation boundary

Global `kGameplayMutationEnabled=false` vẫn giữ nguyên cho toàn bộ action cũ/tương lai.

v1.2.0 thêm gate riêng:

`kReviveMutationEnabled=true`

Do đó Revive có thể được dispatch khi đủ evidence, nhưng AutoFight/NPC/Sell/Path vẫn bị global DispatcherGate từ chối.

CI kiểm tra trực tiếp source và yêu cầu:

- ActionQueue MAX=1;
- command 7 tồn tại;
- Revive gate TRUE;
- global gameplay gate FALSE;
- `revive_action_engine.inc` có đúng **một** direct `g_api.runtime_invoke(handleClick, ...)`;
- file Revive không chứa ClickNPC / StartAutoFight / RequestSellItem / StartPath / StopPath / HandlePointerClick;
- không CreateRemoteThread;
- không il2cpp_thread_attach;
- không Sleep-driven workflow.

## Revive fresh-object resolver

Revive không cache UIButton.

Khi PRE đã đủ, command 7 chạy ngay trên proven hook/Unity-main-thread callback và:

1. đọc snapshot mới;
2. yêu cầu đúng RoleID/MapID, `dead=1`, `MapReady=1`, `WaitingChangeMap=0`;
3. resolve current `UIObject.instances`;
4. lấy Dictionary Values và enumerate object hiện tại;
5. chỉ nhận UIButton/subclass active + interactable;
6. đọc Name/Text;
7. normalize tiếng Việt;
8. ưu tiên exact **Đầu thai**, sau đó Hồi sinh / Revive / Relive / Respawn;
9. nếu nhiều candidate đồng điểm => block fail-closed;
10. đọc lại snapshot + revalidate selected button ngay trước callback;
11. gọi `UIButton.HandleClickEvent()` đúng **một lần**;
12. bỏ pointer ngay khi command trả về.

Không có click tọa độ, raw RVA hay long-lived UI pointer.

## Revive controller lifecycle

Controller chỉ phát Revive khi:

- harmless hook/main-thread proof của session hiện tại đã PASS;
- current-client `kReviveMetadataSupportMask` READY;
- bridge/MainThread/scanner/observer healthy;
- scanner đã 60/60;
- không map transition;
- RoleID/MapID PRE khớp;
- `dead=1` ổn định 2 snapshot liên tiếp;
- ActionQueue trống.

Sau dispatch, queue giữ `active=1`.

Bridge ACK phải xác minh:

- token đúng;
- callback thật sự invoked;
- callback TID đúng proven hook TID;
- managed current/main đúng Unity main thread;
- pre RoleID/MapID đúng;
- preDead=1.

`HandleClickEvent()` return **không được coi là success**.

## Real POST

Revive được phép làm nhân vật đổi MapID. Vì vậy POST không bắt MapID phải giữ nguyên; chỉ RoleID phải giữ nguyên.

Nếu callback gây map transition:

- transition vẫn là hard scanner boundary;
- active Revive được preserve duy nhất để chờ POST;
- không action mới nào được phép vào queue;
- không gửi callback Revive thứ hai.

Sau recovery, cần **2 snapshot sống ổn định**:

- same RoleID;
- `dead=0`;
- `MapReady=1`;
- `WaitingChangeMap=0`.

Snapshot thứ hai mới cho phép `CompleteActive()` và queue trở về `0/1`.

## Fail closed

- Không thấy đúng một UIButton revive => block death episode, không click mù.
- Candidate đồng điểm => block.
- Scanner lỗi trong lúc chờ POST => fail closed.
- RoleID drift => fail closed.
- Dispatch token/thread/PRE mismatch => fail closed.
- POST quá 30 giây => fail closed.
- Không retry callback trong cùng death episode.

## Automated tests

Build chạy cả hai bộ:

- Integrated Control Self Test: **48 checks**.
- Revive Control Self Test: **16 checks**.

Revive tests bao phủ exclusive gate, dispatch ACK, wrong token/thread/PRE, MapID change allowed, RoleID change rejected và xác nhận AutoFight/NPC/Sell vẫn khóa.

## Runtime acceptance

Không test từng phase. Chỉ một lượt live:

`60/60 -> harmless proof PASS -> chết -> dead 2/2 -> fresh Đầu thai -> HandleClickEvent ONCE -> optional map transition/recovery -> dead=0 2/2 -> REVIVE POST PASS -> queue 0/1`

Chi tiết ở `docs/FIRST_RUNTIME_TEST.md`.

## Donor boundary

Legacy v0.9.0 (pre-NewCore) chỉ là donor tên API/UI semantics/ACK/data. Không copy RemoteExecutor, CreateRemoteThread, il2cpp_thread_attach, sleep workflow, raw RVA executor hay cached UI pointer sang NewCore.
