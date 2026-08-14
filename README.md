# Thần Long Auto - NewCore v1.0.2

Đây là nhánh clean rebuild, không phải bản vá v0.9.0.

## Nguyên tắc lõi đã áp dụng

- Không còn `remote_worker.S` / `CreateRemoteThread` để chạy action game.
- Bridge DLL được Windows nạp vào đúng thread cửa sổ game bằng `WH_GETMESSAGE`.
- Trước mọi action, bridge phải chứng minh main-thread bằng `UnitySynchronizationContext` (fail-closed; không suy đoán từ tên class/thread khác).
- IL2CPP method được resolve runtime bằng class/method metadata; không dùng RVA cứng làm action engine.
- Mỗi client có `Session` riêng, FSM riêng, `ActionQueue` riêng.
- Tối đa 1 action đang hoạt động trên mỗi client.
- FSM chờ POST condition từ snapshot thật; không dùng Sleep để đoán UI/state.
- Mất snapshot, map chưa ready, nhân vật chết, main-thread proof mất hoặc POST fail => SAFE PAUSE, không spam retry.

## Phạm vi bản nền này

Đã migrate lên lõi mới:

- Quét nhiều client.
- Main-thread bridge.
- Runtime IL2CPP resolver.
- Snapshot: main-thread proof, map ready, MapID, AUTO state, dead/riding/moving/bag khi API resolve được. X/Y chỉ hiện khi resolver tìm được accessor/field phản chiếu hợp lệ; không fallback offset cứng.
- `AUTO -> Đánh quái` / `AUTO -> Dừng` qua Lua `AutoFight_Main.StartAutoFight(Train/None)` trên main thread.
- `ClickNPC(ResID)` đã có command bridge để dùng cho state NPC tiếp theo.

Chưa bật production trong v1.0.2 nền:

- AutoPath/mount.
- Đầu thai/hồi sinh.
- Trị liệu.
- Sell Engine.
- Buff/skill engine.

Các phần này cố ý chưa bê nguyên v0.9.0 sang. Chúng phải được migrate từng state vào cùng FSM sau khi main-thread bridge chạy ổn định; donor logic/offset/API từ v0.9.0 vẫn được giữ trong tài liệu dưới `docs/`.

## Build

Cần Zig 0.15.2:

```bat
build.cmd
```

Output:

- `dist/ThanLongAutoTrain_NewCore_v1.0.2.exe`
- `dist/ThanLongNewCoreBridge.dll`

Hoặc push GitHub rồi chạy workflow `Build NewCore Windows x64`.

## Test bắt buộc trước khi migrate feature khác

1. Mở game và vào map.
2. Mở NewCore bằng quyền Admin nếu game chạy Admin.
3. Quét client, tick đúng client.
4. Bấm Bắt đầu.
5. Cột `MainThread` phải là `UnitySync`. Nếu `NO`, engine fail-closed và không action.
6. Chạy AUTO cơ bản lâu để xác minh còn diss hay không.
7. Chỉ sau khi nền này ổn định mới migrate sell/heal/revive vào FSM.
