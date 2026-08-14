# Thần Long Mobile - Auto Train v0.7.1

Bản v0.7.1 nhập lại các bản vá đúng từ nhánh build dở sau v0.7.0, dựa trên đúng `GameAssembly.dll` + `global-metadata.dat` đã cung cấp ngày 14-08-2026. Tool giữ mô hình nhiều PID độc lập, không sửa file game trên ổ đĩa và khóa đúng PE build trước khi gọi IL2CPP.

## v0.7.1 sửa gì so với v0.7.0

- **Remote worker 5 tham số native**: hỗ trợ đúng Win64 ABI cho method IL2CPP có `this + 3 managed args + hidden MethodInfo*`; tham số thứ 5 được đặt tại `[rsp+0x20]`.
- **AUTO → Đánh quái không còn báo thành công ảo**: callback chỉ được coi là thành công khi đọc lại `get_EnableAutoF1()` và thấy ON. Nếu vẫn OFF, state machine coi là thất bại để thử lại.
- **UIToggle chắc hơn**: `set_Selected(true)` → đọc lại `get_Selected()`; nếu chưa đổi thì fallback `HandleSelectEvent(true)`. Không spam toggle khi đã selected.
- **UIRectTransform/Lua fallback**: đọc `get_OnPointerClickHandler()` và gọi `MonoBehaviourExecutor.ExecuteScriptFunction(UIObject, string, object[])` thay vì truyền `PointerEventData=null/fake` cho `HandlePointerClick`.
- Fallback Rect tạo **managed `System.Object[3]` thật** bằng `il2cpp_array_new`, root bằng `il2cpp_gchandle_new`, truyền UIObject ở phần tử 0 và để hai tham số pointer phụ là nil. Đây giữ đúng số lượng args mà wrapper `HandlePointerClick` của client tạo nhưng không bịa pointer-event object.
- `FindAction()` hiện thử theo thứ tự **UIButton → UIToggle → UIRectTransform có Lua click handler**. Match vẫn phải duy nhất; không chọn đại.
- `UIObject.get_CoreParents()` được dùng làm fast-path để phân loại cây UI; nếu không đọc được thì fallback chuỗi `get_Parent()` cũ.
- **NPC bán đồ lưu thêm ResID ổn định**. Khi về NPC, tool gọi `GetNearestNPC(ResID)` để lấy RoleID instance hiện tại rồi mới `ClickNPC(RoleID)`, tránh tin tuyệt đối RoleID đã lưu từ phiên trước.
- File NPC nâng lên v2: `Tên<TAB>ResID<TAB>RoleID<TAB>MapID<TAB>X<TAB>Y`, vẫn đọc ngược file v1 cũ.
- Chuỗi bán nhận cả item cell **UIButton** lẫn **UIRectTransform có Lua handler** nếu control + cây parent chứng minh thuộc bag/inventory; tối đa 90 callback.
- Bán đồ thất bại 3 lần, hoặc đã hết 90 lượt mà túi vẫn full → **dừng riêng PID đó**, không kéo các cửa sổ khác theo.
- Dừng AUTO cũng đọc lại `EnableAutoF1=OFF`; không chỉ tin callback đã gửi.

## API/RVA mới xác minh trong v0.7.1

| Method | RVA |
|---|---:|
| `LuaSystemAPI_Game.GetNearestNPC(int resID)` | `0x673AA0` |
| `LuaMapSpriteData.get_ResID()` | `0x425870` |
| `UIObject.get_CoreParents()` | `0x52FF10` |
| `UIToggle.HandleSelectEvent(bool)` | `0x687450` |
| `UIRectTransform.get_OnPointerClickHandler()` | `0x644B50` |
| `MonoBehaviourExecutor.get_Instance()` | `0x523CE0` |
| `MonoBehaviourExecutor.ExecuteScriptFunction(UIObject,string,object[])` | `0x521B20` |

Các RVA v0.7.0 vẫn giữ: `GetFreeBagSpace=0x6716F0`, `GetNearestNPC()=0x673A90`, `ClickNPC=0x66ADC0`, `LuaMapObjectData.get_RoleID=0x41F000`, `get_Name=0x41F3F0`, `UIToggle.get_Selected=0x6885D0`, `set_Selected=0x6888E0`…

## Logic tự bán đồ

1. Bật **Tự bán đồ** → probe `GetFreeBagSpace()` ngay khi Start, sau đó đúng chu kỳ cấu hình; không probe mỗi vòng worker.
2. Full túi → đúng PID tắt train + dừng path → đi NPC đã lưu.
3. Tới NPC → xuống ngựa → nếu có ResID thì tìm đúng NPC hiện tại bằng `GetNearestNPC(ResID)` → lấy RoleID mới → `ClickNPC(RoleID)`.
4. Chọn **Bán vật phẩm** → bật **Bán vật phẩm nhanh** bằng UIButton/UIToggle/UIRect-Lua thật.
5. Quét item cell ở cây bag/inventory. UIButton dùng `HandleClickEvent()`. UIRect chỉ được gọi khi có Lua handler thật và parent path an toàn.
6. Tối đa 90 lượt; `GetFreeBagSpace()` được dùng làm bằng chứng bán thật.
7. Nếu bán thành công, thử đóng đúng nút Close thuộc cây shop rồi quay lại bãi. Nếu thất bại lặp 3 lần hoặc hết 90 lượt vẫn full, dừng riêng session/PID.

## AUTO → Đánh quái

- Mở `AUTO` bằng callback nội bộ.
- Tìm **Đánh quái** ở UIButton, UIToggle rồi UIRect-Lua.
- UIToggle: set → readback → fallback `HandleSelectEvent(true)` nếu cần.
- UIRect: gọi Lua handler bằng `MonoBehaviourExecutor.ExecuteScriptFunction`, không gọi `HandlePointerClick(null)`.
- Sau cùng bắt buộc `EnableAutoF1=ON`; nếu không, trả thất bại để state machine thử lại.

## Chức năng cũ giữ nguyên

- Multi-window: session/worker/config/skill/bãi/state độc lập theo PID; setting theo RoleID; spots/NPC file dùng chung.
- Start/Pause chỉ áp dụng cửa sổ được tick.
- Sai MapID/tọa độ: tắt train ngay → hủy path → về đúng bãi → đúng sai số mới bật train.
- Chuyển map: `WaitingChangeMap + IsMapReady`, không dùng chờ cứng 6 giây sau xác nhận/hồi sinh.
- Game treo/mất phản hồi/remote packet bận: pause an toàn, không ghi đè packet.
- Hồi sinh/Đầu thai: chỉ khi `IsDeath=true`, gọi `UIButton.HandleClickEvent()` thật.
- Không còn hai nút test trên giao diện.
- Tab GIỚI THIỆU giữ “Phần mềm xây dựng bởi Nguyễn Mạnh Thắng.Long.”

## File cấu hình

- `ThanLongAutoTrain.spots.txt`: `Tên<TAB>MapID<TAB>X<TAB>Y`.
- `ThanLongAutoTrain.npcs.txt` v2: `Tên<TAB>ResID<TAB>RoleID<TAB>MapID<TAB>X<TAB>Y`; vẫn đọc file v1 cũ.
- `ThanLongAutoTrain.ini`: profile `AutoTrain.Role<RoleID>`; `AutoSell`, `BagCheckMinutes`, `SellNpcName`.

## Kiểm tra tĩnh đã chạy

- `main.cpp`: C++17 + `-Wall -Wextra -Werror` **PASS** bằng Win32 stub dùng riêng cho syntax/semantic check.
- `remote_worker.S`: assemble **PASS** thành x86-64 Windows COFF.
- Prefix machine-code của toàn bộ RVA mới phía trên đã đối chiếu trực tiếp với `GameAssembly(1).dll`.
- Không còn chuỗi/tên hai nút test trong source UI.

## Chưa thể khẳng định nếu chưa chạy client/server thật

- Layout Lua UI do server tải có thể khác giữa màn hình/map. Resolver đã khóa “không bấm mù”, nhưng cần test runtime để xác nhận control thực tế của **Đánh quái**, **Bán vật phẩm**, **Bán nhanh** và từng item cell.
- UIRect-Lua fallback giữ đúng callback string và mảng 3 args nhưng hai thông tin pointer phụ là nil. Phần này cần test trực tiếp; nếu handler server thực sự bắt buộc pointer coordinates/pointerId, callback có thể không thực hiện hành động và tool sẽ dựa vào state thật (`EnableAutoF1`/`GetFreeBagSpace`) để coi là thất bại.

## Build Windows

Cài Zig và để `zig` trong PATH, sau đó chạy `build.cmd`.

Kết quả mong đợi: `dist\ThanLongAutoTrain_v0.7.1.exe`.
