# AUDIT v0.9.0

## Combat separation

`TrainActivationMode` hiện chỉ có:

- `CtrlTabSkill = 1`
- `AutoFight = 2`

Mode Ctrl+Tab không còn gọi `LuaSelectTarget`/`GetNearbyEnemyIDs`; khi cần target, nó gửi đúng Ctrl+Tab rồi dùng selected skill. Mode AUTO không gọi Ctrl+Tab.

AUTO chỉ đánh giá lệch X/Y sau mỗi 5 phút kể từ lúc `StartAutoFight(Train)` được xác minh. Ctrl+Tab mode vẫn đánh giá tọa độ mỗi worker cycle. MapID vẫn guard liên tục ở cả hai.

## Mount state-machine

Đã kiểm tra thứ tự branch: `live.riding` được xử lý trước mọi branch dọn quái. Sau khi riding=true không có call-site Ctrl+Tab/skill/AutoTrain trong pha mount recovery; nếu raw `live.autoFight` vẫn ON thì Dừng được thử ngay trên scan đầu tiên thấy riding=true.

Chu kỳ: mount → 4 s → fail #1 → combat cleanup 10 s → mount → fail #2 → walk 1 phút → reset. Nếu AUTO cleanup không dừng được sau retry, tool không thử mount và chuyển walking fallback.

Mount request bị reject cũng vẫn được tính là một attempt sau đủ 4 s, không spam mỗi worker tick. AUTO cleanup chỉ thử StartAutoFight tối đa 2 lần trong cửa sổ 10 s.

## Sell stability

- Không latch selling trip khi persistent AUTO còn chạy và chưa dừng được.
- `TrySellAtNpc()` kiểm tra AUTO lần nữa trước `OpenNpc`.
- `ClickNPC` quiet 1.2 s.
- GameDialog/NPCShop stable twice.
- Shop control lookup root-scoped; global toggle scanner đã xóa.
- server ACK polling 125 ms, tối đa ~3 s; sau ACK quiet 900 ms trước scan tiếp.

## Static checks

- `clang++ -std=c++17 -Wall -Wextra -Werror -fsyntax-only` với Win32 declaration stubs: PASS.
- Clang Static Analyzer: 0 diagnostics.
- `src/remote_worker.S` assemble `x86_64-pc-windows-msvc`: PASS, COFF x86-64.
- 59/59 byte signatures trong `Validate()` khớp `GameAssembly.dll` SHA-256 `4c98c9934bc4260efa64f5492c58e0c5104c89359f0126e7cd402feb381fe3c7`.
- rva/off constexpr không dùng: 0.

## Runtime caveat

Lua callbacks vẫn đi qua remote worker, nên runtime test trên client là bắt buộc. Nếu vẫn crash đúng tại Lua invoke thì vấn đề còn lại là thread affinity/main-thread hoặc cần chuyển Sell sang packet đã trace đủ context.
