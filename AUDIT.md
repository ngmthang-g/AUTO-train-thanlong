# Rà soát kỹ thuật - Auto Train v0.8.0

Ngày rà soát: 14-08-2026.

## 1. Hồi quy NPC v0.7.2

Nguyên nhân đã xác định:

- `LuaSystemAPI_Game.GetNearestNPC()` (0 tham số) trả `LuaMapObjectData`.
- v0.7.2 đã gọi getter ResID của lớp dẫn xuất trên object base và biến ResID thành điều kiện bắt buộc, làm NPC hợp lệ bị loại.
- Nhánh lookup 1 tham số trước đó còn dùng sai RVA `0x673AA0`; entry đúng của overload 1 tham số nằm ở `0x673A80`. v0.8.0 không còn phụ thuộc overload này để lưu/mở NPC.

Fix:

- Save NPC dùng đúng `GetNearestNPC() -> get_RoleID + get_Name` như v0.7.0.
- Tới vị trí NPC, re-scan nearest NPC và so tên trước khi dùng RoleID hiện tại để `ClickNPC()`.
- Nếu nearest NPC khác tên, không click mù.

## 2. Shop / bán trang bị

Metadata C# không có API `SellItem` trực tiếp. Những API liên quan được xác nhận gồm:

- `GetFreeBagSpace()`
- `IsItemSellable(itemID)`
- `IsItemSellToShopWithBoundMoney(itemID)`
- `GetItemsAtSite(site)` / `GetItemAtSite(site,pos)`

Do enum/site và packet bán trực tiếp không được chứng minh đủ để gọi an toàn, v0.8.0 giữ đường callback UI nội bộ:

`ClickNPC -> Mua thú cưỡi -> Bán vật phẩm -> Bán vật phẩm nhanh -> Trang bị -> item cells`.

Mỗi item tối đa 3 lần, toàn chuỗi tối đa 90 callback. Chỉ click control thuộc cây bag/inventory và hỗ trợ UIButton hoặc UIRect-Lua.

## 3. AUTO -> Đánh quái

- Navigation, map guard, confirm map, Đầu thai và mount là luồng dùng chung với SelectedSkill.
- AUTO root, Fight và Stop đều resolve đa control.
- Fight phải xác nhận AutoFight=ON; Stop phải xác nhận AutoFight=OFF.
- Không dùng direct setter để giả thành công.

## 4. Repeated death / PK crash guard

Race nguy hiểm trong luồng cũ: thao tác menu AUTO hoặc callback UI khác có thể trùng lúc death overlay đang được tạo/hủy; lặp callback Đầu thai trên object UI cũ làm nguy cơ crash tăng sau nhiều chu kỳ.

v0.8.0:

1. Death edge chỉ Stop AutoPath.
2. Chờ `IsDeath=true`, `MapReady=true`, `WaitingChangeMap=false` ổn định 2 scan.
3. Re-scan nút Đầu thai rồi gọi đúng một lần.
4. Không gửi UI/path khác khi còn dead.
5. Chỉ cho một retry muộn sau 15 giây, và retry cũng re-scan object mới.
6. `IsDeath=false` phải ổn định 2 scan trước khi tiếp tục.
7. Sau sống lại đi qua map guard; nếu bật trị liệu thì trị liệu trước khi về bãi.

## 5. Auto buff / chat / trị liệu

- Buff: dùng `GetBuffs`, `HasBuff`, `RequestUsingSkill` và LuaBuffData getters; không giả định `buffID == skillID`.
- Chat: `UIFactory.SendDefaultChat(string)` đã được xác nhận trong metadata và disassembly; đường này tạo chat data rồi gửi command nội bộ. Chưa có bằng chứng đủ để chọn channel cụ thể từ API này.
- Trị liệu: không tìm thấy API C# trực tiếp tương đương NPC treatment; dùng UI callback có nhận diện nhãn/cây control.

## 6. Kiểm tra binary/source

- PE guard: TimeDateStamp `0x6A410C14`, SizeOfImage `0x03DCB000`.
- 56/56 signature runtime hiện có trong source khớp byte với `GameAssembly.dll` đã cung cấp.
- C++17 `-Wall -Wextra -Werror`: PASS.
- Clang Static Analyzer: 0 diagnostics.
- `remote_worker.S` -> x86-64 Windows COFF: PASS; arg5 được đặt tại `[rsp+0x20]`.

## 7. Giới hạn còn lại

Không ghi “runtime OK” cho các control shop/treatment/buff/AUTO khi chưa chạy trực tiếp server game. Mọi resolver mới đều ưu tiên fail-safe: không nhận diện duy nhất thì dừng nhánh đó, không click theo tọa độ màn hình và không click mù.
