# Thần Long Auto - NewCore v1.0.11 Unified Map Transition Guard

v1.0.11 vẫn **READ-ONLY**. Gameplay mutation tiếp tục **LOCKED**.

## Runtime evidence dẫn tới bản này

v1.0.10 đã chứng minh AutoPath resolver/semantics (`5/5`, value `0→1→0`) và scanner đạt `60/60`. Log thực tế cũng cho thấy nhiều lần MapID đổi trong khi `MapReady/WaitingChangeMap` không bật. Vì vậy transition flags không thể là nguồn duy nhất để định nghĩa đổi map.

## v1.0.11

Map transition có hai nguồn quan sát hợp lệ:

- `source=flags`: `MapReady=0` hoặc `WaitingChangeMap=1`; bridge giữ donor guard và bỏ qua Leader/AutoPath khi scene rebuild.
- `source=identity`: RoleID/MapID đổi trong snapshot stable nhưng flags chưa bật hoặc đến muộn.

Cả hai nguồn đều:

1. reset scanner qualification;
2. đánh dấu runtime coverage `mapTransition`;
3. khóa action;
4. đi vào cùng recovery gate;
5. chỉ PASS sau 2 snapshot stable liên tiếp có cùng RoleID + MapID.

Nếu identity tiếp tục đổi giữa recovery, candidate reset về `1/2`. Nếu flags xuất hiện muộn sau transition khởi phát bởi identity, flags chỉ join session đang có, không tạo transition thứ hai.

`60/60` vẫn có nghĩa 60 snapshot stable liên tiếp trên cùng identity (~30 giây ở 500 ms). Đổi map trong lúc qualification thì reset là đúng.

## Golden-rule boundary

Legacy v0.9.0 vẫn chỉ là donor. Không RemoteExecutor, remote gameplay worker, CreateRemoteThread gameplay action, il2cpp_thread_attach runtime path, Sleep transition, cached UI pointer hay scanner-triggered action.

Sau khi v1.0.11 runtime PASS mới được đóng Scanner/Observer gate để dựng SafetyGuard + ActionQueue(MAX=1) + FSM scaffold; mutation vẫn khóa ở bước scaffold đầu.
