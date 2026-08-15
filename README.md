# Thần Long Auto - NewCore v1.0.12 SafetyGuard + ActionQueue + FSM Dry-Run

v1.0.12 bắt đầu tầng control-plane sau khi Scanner/Observer runtime đã PASS. **Gameplay mutation vẫn khóa compile-time**.

## Pipeline

`Resolver -> Scanner(read-only) -> Snapshot/State Store -> Observer -> FSM -> SafetyGuard -> ActionQueue(MAX=1) -> Dispatcher Gate -> Internal Action Engine`

Bản này chỉ dựng và chứng minh phần `FSM -> SafetyGuard -> ActionQueue` ở chế độ DRY-RUN. Không có BridgeCommand gameplay mới và dispatcher không phát action game.

## FSM scaffold

Mỗi client chỉ có một `ControlFsmState` hiện tại:
- `SAFE_PAUSED`
- `SCANNER_QUALIFYING`
- `IDLE_STABLE`
- `DEAD_DETECTED`
- `MAP_TRANSITION`
- `FAULTED`

Map transition hoặc scanner/observer không ổn định luôn kéo control-plane về trạng thái fail-closed và clear queue.

## SafetyGuard

Candidate đầu tiên chỉ là `Revive`, dùng để chứng minh PRE condition từ snapshot đã biết. Guard yêu cầu:
- bridge còn attach;
- Unity main-thread proof còn valid;
- scanner healthy + qualified 60/60;
- observer stable mask đầy đủ;
- không có map transition;
- RoleID/MapID của candidate khớp snapshot hiện tại;
- `dead=1`.

Guard PASS vẫn **không cấp quyền mutate**. `kGameplayMutationEnabled=false` được `static_assert` và `build.cmd` kiểm tra.

## ActionQueue scaffold

`ActionQueueScaffold::kCapacity = 1` và có `static_assert` giữ MAX=1. Queue chỉ nhận một dry-run envelope sau khi guard preconditions PASS. Dispatcher gate sau đó bắt buộc block envelope, log và bỏ nó; `active=0` luôn luôn ở v1.0.12.

## Golden-rule boundary

Legacy v0.9.0 vẫn donor-only. Không RemoteExecutor, remote gameplay worker, CreateRemoteThread gameplay action, il2cpp_thread_attach runtime path, Sleep transition, cached UI pointer hay scanner-triggered mutation.

Nếu runtime v1.0.12 chứng minh FSM transitions, SafetyGuard block/pass và ActionQueue depth không vượt 1, phase tiếp theo mới được chứng minh dispatcher/action path bằng một probe an toàn riêng; chưa nhảy thẳng vào bán đồ/đầu thai/AUTO.
