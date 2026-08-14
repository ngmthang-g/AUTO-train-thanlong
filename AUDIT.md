# Rà soát kỹ thuật - Auto Train v0.8.2

## 1. Nguyên nhân popup “Không tìm thấy NPC tương ứng!”

Đã đối chiếu trực tiếp `GameAssembly.dll` của build được bảo vệ bởi tool.

Predicate mà `LuaSystemAPI_Game.ClickNPC(int npcID)` dùng để tìm NPC chứa logic tương đương:

```asm
mov eax, dword ptr [closure + 0x10]   ; npcID truyền vào
cmp dword ptr [candidate + 0xC0], eax
sete al
```

`GNPC.get_ResID()` của đúng build là:

```asm
mov eax, dword ptr [rcx + 0xC0]
ret
```

Kết luận: tham số `ClickNPC(int)` là **ResID**, không phải RoleID. v0.8.1 truyền RoleID nên game tự tìm không thấy GNPC tương ứng và hiển thị chính popup người dùng chụp lại.

### Đường resolver mới

- `GetNearestNPC()` → `LuaMapObjectData.RoleID + Name`.
- `SessionData.get_NPCs()` / `get_MovingNPCs()` → enumerate value object.
- Match `GSprite.RoleID` tại `+0x30`.
- Read `GNPC.ResID` tại `+0xC0`, sau đó xác minh lại bằng method thật `GNPC.get_ResID()` RVA `0x68EE70`.
- Trước `ClickNPC`, gọi `GetNearestNPC(resID)` RVA `0x673A80` để chắc NPC resource đó đang trong phạm vi tương tác.
- Cuối cùng `ClickNPC(resID)`.

Không dựa vào screen coordinate để mở NPC.

## 2. Vì sao AUTO mode làm đi map/ngựa chậm và dễ diss

v0.8.1 có nhánh phục hồi lên ngựa gọi `TriggerAutoMenu()` trong pha 10 giây dọn quái. Cùng lúc state machine còn quản AutoPath, cổng và ngựa. Đây là hai nhóm lệnh cùng tác động gameplay/UI nhưng không có ownership rõ ràng; UI AUTO có thể bị quét/callback trong lúc game đang dựng map hoặc xử lý mount.

v0.8.2 áp dụng **command ownership**:

- `NAVIGATION`: chỉ AutoPath, cổng, ngựa và skill cleanup đã chọn.
- `AUTO_COMBAT`: chỉ được kích hoạt khi đã đúng bãi, map ready, đứng yên và xuống ngựa.
- Nếu cần rời bãi mà AutoFight còn ON: stop path → `AUTO → Dừng` → chờ scan mới xác nhận OFF → mới giao quyền cho navigation.
- Khi map/UI đang phục hồi, `StopPathOnly()` được ưu tiên; không phát động một UI scan AUTO chỉ vì đọc state thất bại.

Điều này đưa đường di chuyển của AutoMenu về cùng kiến trúc với SelectedSkill thay vì chạy logic combat song song.

Riêng lúc vừa tới bãi mà vẫn còn trên ngựa, v0.8.2 không gọi `AUTO → Dừng` trước nữa. Hai activation mode đều chạy cùng thứ tự: `Stop AutoPath → xuống ngựa → chờ 4 giây → scan mới xác nhận đã xuống`, sau đó AutoMenu mới được quyền mở AUTO. Đây là khác biệt runtime trực tiếp so với nhánh cũ làm down-mount của AUTO chậm hơn.

## 3. AUTO → Đánh quái

Production path cố ý bảo thủ:

1. Tìm và gọi **UIButton AUTO** — phần duy nhất các version cũ đã chứng minh mở được.
2. Sau khi root callback chạy, chờ submenu được tạo.
3. Chỉ nhận `Đánh quái` dưới dạng `UIButton` hoặc `UIToggle`.
4. Với Toggle: setter/select handler chỉ là cơ chế gọi control; thành công cuối cùng vẫn phải có `get_EnableAutoF1() == ON`.
5. Không quét `UIRectTransform` toàn cục trong AUTO vì v0.8.0/v0.8.1 cho thấy đường này nặng và có nguy cơ chạm stale/recycled object.
6. Không báo “ĐANG TRAIN” chỉ vì cờ cũ đang ON; session phải xác minh combat của chính lần kích hoạt tại bãi.

## 4. `AutoSetFlag` không phải chế độ Đánh quái

RVA `0x66A210` được disassemble. Đường gọi cuối chuyển `RangerAuto` thành:

- vertices = 64
- outerRadius = RangerAuto
- innerRadius = RangerAuto - 5
- current position

và gọi `MapRenderer.DrawCicleAutoFight(...)` RVA `0x588D70`.

Do đó không dùng `AutoSetFlag()` làm shortcut bật Đánh quái.

## 5. Các RVA NPC bổ sung

- `LuaSystemAPI_Game.GetNearestNPC()` = `0x673A90`
- `LuaSystemAPI_Game.GetNearestNPC(int npcResID)` = `0x673A80`
- `LuaSystemAPI_Game.ClickNPC(int)` = `0x66ADC0`
- `SessionData.get_NPCs()` = `0x6F3DD0`
- `SessionData.get_MovingNPCs()` = `0x6F3D80`
- `GNPC.get_ResID()` = `0x68EE70`

Các RVA mới nằm trong bảng signature guard của source để tool từ chối chạy nếu client game không khớp build.

## 6. Những gì vẫn cần runtime

Static analysis xác định được identifier và xung đột state machine, nhưng không thể chứng minh class thực tế của mục `Đánh quái` trên UI server đang chạy. Nếu v0.8.2 mở được AUTO nhưng không tìm được submenu, bước đúng tiếp theo là thêm diagnostic **chỉ đọc** để dump Button/Toggle/Rect vừa xuất hiện trước/sau khi người dùng tự bấm `Đánh quái` một lần, rồi gọi đúng handler đó. Không quay lại “quét Rect rồi thử callback hàng loạt”.
