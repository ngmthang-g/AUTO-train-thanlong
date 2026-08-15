# Thần Long Auto - NewCore v1.0.9 Scanner Qualification

v1.0.9 **không mở gameplay action**. Bản này siết Phase Scanner/Observer để một snapshot đúng không còn bị hiểu nhầm là scanner đã ổn định.

## Pipeline vẫn khóa cứng

`Resolver -> Read-only Scanner -> GameSnapshot/State Store -> Observer -> State Machine -> Safety Guard -> ActionQueue(MAX=1) -> MainThread Dispatcher -> Internal Action Engine`

Hiện chỉ các tầng Resolver/Scanner/Snapshot/Observer và main-thread proof được chạy. Action Queue / Internal Action Engine chưa được phép mutate game.

## Điểm mới so với v1.0.8

- `SCANNER PASS` tức thời bị bỏ.
- Cần **60 snapshot stable liên tiếp** ở chu kỳ 500 ms (~30 giây) mới đạt `SCANNER CORE QUALIFIED`.
- Bất kỳ scanner query failure nào sẽ reset qualification về 0 và vào recovery read-only; 3 lỗi liên tiếp vẫn fail-closed.
- Map transition reset qualification, giữ donor guard: đọc `MapReady/WaitingChangeMap` trước, không đụng Leader/AutoPath khi scene đang chuyển, rồi bắt buộc `MAP RECOVERY 1/2 -> PASS 2/2`.
- Observer ghi coverage cho các runtime edge cần test thủ công: `moving`, `riding`, `autoPath`, `dead`, `mapTransition`.
- Qualification PASS **không đồng nghĩa edge coverage PASS**. Log sẽ ghi rõ `runtime edge pending=...`.

## Legacy v0.9.0 donor boundary

**Legacy v0.9.0 = bản auto cũ pre-NewCore, không phải NewCore v0.9.0.** Chỉ dùng làm donor cho names/data/state semantics/RVA đối chiếu/NPC-map-item-UI-Lua-ACK knowledge. Không copy `RemoteExecutor`, remote gameplay worker, `CreateRemoteThread`, Sleep transition, long-lived UI pointer hay Worker cũ.

## Runtime test mong đợi

Sau foundation/main-thread proof và snapshot đầu:

- `SCANNER QUALIFYING 1/60`
- mốc `10/60`
- mốc `30/60`
- sau khoảng 30 giây stable: `SCANNER CORE QUALIFIED 60/60 (~30s)`

Sau đó test thủ công di chuyển, riding, AutoPath, chết/hồi sinh và đổi map. Mỗi edge hợp lệ phải giảm danh sách `runtime edge pending`.

## Fail-closed

- Scanner chỉ đọc.
- Không action từ scanner thread.
- Không quá 1 bridge request/client.
- Unknown/partial state không click thử; qualification reset và action vẫn LOCKED.
- 3 scanner failures liên tiếp -> session faulted.
- Map transition -> chỉ scan transition state cho tới recovery 2/2.

## Build

Yêu cầu Zig 0.15.2 trên Windows.

1. chạy `build.cmd`;
2. Architecture audit phải PASS;
3. Bridge LoadLibrary self-test phải PASS;
4. chạy `dist\ThanLongAutoTrain_NewCore_v1.0.9.exe`;
5. tick client -> **Kiểm tra nền + Scanner**;
6. làm checklist trong `docs/FIRST_RUNTIME_TEST.md`.

Các bất biến bắt buộc vẫn ở `docs/GOLDEN_RULES.md`.
