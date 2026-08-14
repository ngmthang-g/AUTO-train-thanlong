# Rà soát kỹ thuật - Auto Train v0.7.2

Ngày rà soát: 14-08-2026.

## Các thay đổi được rà

1. **Một đường AUTO duy nhất**: không còn mode phím nền hay direct setter trạng thái AUTO. Getter trạng thái game chỉ dùng để xác minh `AUTO → Đánh quái / Dừng` thật.
2. **AUTO → Dừng bắt buộc khi ngắt train**: `DisableActions()` chỉ gọi chuỗi AUTO → Dừng nếu trạng thái đánh quái đang ON (hoặc không đọc được state), sau đó mới dừng AutoPath.
3. **Lên ngựa**: sau call mount chờ 4 giây. Nếu vẫn chưa riding, chạy combat-clear 10 giây; AUTO mode giữ Đánh quái, skill mode dùng skill. Hết 10 giây, AUTO phải Dừng và xác minh OFF trước khi retry mount.
4. **Đầu thai duy nhất**: ButtonRole chỉ match `dauthai`; không có nhánh nút chết thứ hai.
5. **UI**: status row dịch trái và rộng hơn để hiển thị `Túi đồ trống: N ô`; các static description không có hành động đã bỏ.
6. **Config migration**: giữ numeric 1=SelectedSkill, 2=AutoMenu; old 0 được map sang AutoMenu.
7. **Dead-code cleanup**: wrapper test nội bộ không còn caller đã bị xóa; build nghiêm ngặt PASS.

## Hệ thống IL2CPP giữ nguyên

- Worker 5 native args theo Win64 ABI.
- UIButton / UIToggle / UIRect-Lua resolver.
- `GetFreeBagSpace()` cho tay nải.
- NPC ResID → RoleID hiện tại → `ClickNPC`.
- Multi-PID độc lập và guard map/hung/remote busy.

## Kiểm tra

- `clang++ -std=c++17 -Wall -Wextra -Werror -fsyntax-only`: PASS với Win32 stub kiểm tra source.
- `remote_worker.S` assemble x86-64 Windows COFF: PASS.
- Grep source chính: không còn mode phím train nền/direct setter/nhánh nút chết thứ hai.

Phần callback UI vẫn cần client/server thật để kiểm chứng runtime.
