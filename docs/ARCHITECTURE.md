# NewCore architecture - v1.0.9 phase gate

> Architectural invariants: see `docs/GOLDEN_RULES.md`. `Legacy v0.9.0` always means the old pre-NewCore source, never a NewCore version.

Pipeline đích:

`Resolver -> Read-only Scanner -> Snapshot/State Store -> Observer -> FSM -> SafetyGuard -> ActionQueue(max=1) -> Proven MainThread Dispatcher -> Internal Action Engine -> Game -> ACK/Observer`

## Đã triển khai đến v1.0.9

`Window discovery -> Hook bridge -> native validation -> IL2CPP metadata -> MainThread proof -> continuous read-only Scanner -> State Store -> state-edge Observer`

Scanner cadence hiện tại: 500 ms/client, tối đa một request outstanding/client.

## Chưa triển khai

- FSM gameplay;
- SafetyGuard cho action;
- ActionQueue;
- game mutation/internal action engine.

## Transition boundary

`MapReady/WaitingChangeMap` là boundary của Scanner. Nếu scene chưa ổn định, scanner không dereference/query Leader/AutoPath state. Sau transition, cần 2 stable scans liên tiếp trước khi Scanner trở lại trạng thái ổn định. Đây là donor lesson từ Legacy v0.9.0 (pre-NewCore) được giữ ở tầng Scanner chứ không trộn vào action logic.

## Gate trước phase action

Không được mở action cho tới khi runtime test chứng minh:

1. continuous scanner ổn định qua đứng/yên/chạy/ngựa;
2. AutoPath state đúng;
3. map transition không fault;
4. dead/alive edge đúng;
5. multi-client không lẫn state;
6. không có dấu hiệu scanner gây disconnect/stutter bất thường.


## v1.0.9 qualification gate

Scanner chỉ được gọi là core-qualified sau 60 snapshot observer-valid liên tiếp (~30 giây). Scanner error, partial mask hoặc map transition reset qualification. Runtime edge coverage vẫn là test riêng và không tạo gameplay action.
