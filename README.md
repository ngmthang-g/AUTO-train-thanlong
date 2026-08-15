# Thần Long Auto - NewCore v1.0.10 AutoPath + Identity Guard

v1.0.10 vẫn **READ-ONLY** và gameplay action tiếp tục **LOCKED**.

## Pipeline khóa cứng

`Resolver -> Read-only Scanner -> GameSnapshot/State Store -> Observer -> State Machine -> Safety Guard -> ActionQueue(MAX=1) -> MainThread Dispatcher -> Internal Action Engine`

Hiện chỉ Resolver/Scanner/Snapshot/Observer và main-thread proof được chạy.

## Điểm mới sau runtime v1.0.9

Runtime v1.0.9 đã chứng minh:
- scanner 60/60 stable nhiều lần;
- Moving / Riding / Dead / MapTransition edge hoạt động;
- map transition donor guard không đụng Leader/AutoPath trong scene rebuild;
- recovery 2/2 hoạt động.

Hai lỗ cần đóng trước phase action:
1. `AutoPath` chưa từng quan sát giá trị ON.
2. Recovery cũ chỉ đếm 2 snapshot stable, chưa bắt buộc cùng `RoleID + MapID`; runtime đã thấy map identity có thể đổi muộn sau recovery.

## v1.0.10

- `GameSnapshot.autoPathProbeMask` ghi 5 bước read-only:
  class -> static instance getter -> singleton instance -> state getter -> value read.
- Log `AUTOPATH PROBE 5/5 ... resolver PASS` tách lỗi resolver khỏi trường hợp getter hợp lệ nhưng value vẫn 0.
- Nếu AutoPath được quan sát ON dù scanner bắt đầu giữa phiên, coverage AutoPath được coi là proven.
- Map recovery chỉ PASS khi **2 snapshot liên tiếp có cùng RoleID và MapID**.
- Nếu identity đổi trong recovery: `MAP RECOVERY RESET` và bắt đầu lại 1/2.
- Nếu RoleID/MapID đổi ngoài transition flags: `IDENTITY REQUALIFY` và scanner phải chạy lại 60/60.
- 3 scanner failures liên tiếp vẫn fail-closed.
- Không thêm action game.

## Legacy v0.9.0 donor boundary

Legacy v0.9.0 vẫn chỉ là donor dữ liệu/semantics đã xác minh. Không copy RemoteExecutor, remote gameplay worker, CreateRemoteThread action path, Sleep transition, long-lived UI pointer hay Worker cũ.

## Build

Windows + Zig 0.15.2:

1. `build.cmd`
2. architecture audit `.cpp/.h/.inc` phải PASS
3. Bridge LoadLibrary self-test PASS
4. chạy `dist\ThanLongAutoTrain_NewCore_v1.0.10.exe`
5. test theo `docs/FIRST_RUNTIME_TEST.md`

Các bất biến ở `docs/GOLDEN_RULES.md`.
