# Rà soát kỹ thuật - Auto Train v0.7.1

Ngày rà soát: 14-08-2026.

## Game build khóa cứng

- PE timestamp: `0x6A410C14`
- `SizeOfImage`: `0x03DCB000`
- Image base tĩnh: `0x180000000`
- Runtime kiểm tra prefix machine-code trước khi gửi lệnh IL2CPP.

## Các lỗi nhánh build dở được nhập lại

1. **ABI 5 args**: `MonoBehaviourExecutor.ExecuteScriptFunction(UIObject,string,object[])` là instance method có 3 managed args; native call cần `RCX=this`, `RDX=uiObject`, `R8=functionName`, `R9=args`, và hidden `MethodInfo*` ở stack. Remote packet/worker v0.7.1 có `arg5` và đặt tại `[rsp+0x20]`.
2. **Không thành công ảo**: AUTO chỉ trả success khi `get_EnableAutoF1()` thật sự ON; Dừng chỉ success khi OFF.
3. **UIToggle readback**: `set_Selected(true)` phải đọc lại `get_Selected()`, fallback `HandleSelectEvent(true)` nếu state chưa đổi.
4. **UIRect/Lua**: không gọi `HandlePointerClick(null)`. Đọc handler string và gọi `ExecuteScriptFunction` bằng managed object array thật.
5. **NPC ResID**: lưu ResID + RoleID. Lúc bán dùng `GetNearestNPC(ResID)` lấy RoleID instance hiện tại rồi mới `ClickNPC`.
6. **CoreParents**: dùng một call `get_CoreParents()` để giảm chuỗi getter parent; có fallback cũ.

## RVA mới v0.7.1

| Method | Token/RID | RVA |
|---|---:|---:|
| `LuaSystemAPI_Game.GetNearestNPC(int)` | `0x06000891` | `0x673AA0` |
| `LuaMapSpriteData.get_ResID()` | `0x06000294` | `0x425870` |
| `UIObject.get_CoreParents()` | `0x060003A3` | `0x52FF10` |
| `UIToggle.HandleSelectEvent(bool)` | `0x060006CA` | `0x687450` |
| `UIRectTransform.get_OnPointerClickHandler()` | `0x060005AF` | `0x644B50` |
| `MonoBehaviourExecutor.get_Instance()` | `0x0600035B` | `0x523CE0` |
| `MonoBehaviourExecutor.ExecuteScriptFunction(UIObject,string,object[])` | `0x06000372` | `0x521B20` |

Method pointer table của `Assembly-CSharp.dll` được đối chiếu với các RVA đã biết (UIButton/UIToggle) trước khi lấy các RID trên. Prefix 12-byte của từng RVA mới tiếp tục được khóa trong `Validate()`.

## UIRect callback

Disassembly của wrapper `UIRectTransform.HandlePointerClick` cho thấy game gọi `MonoBehaviourExecutor.ExecuteScriptFunction` với `object[3]`. v0.7.1 tạo `System.Object[3]` bằng `il2cpp_array_new`, root array bằng GCHandle, đặt UIObject ở index 0 và để hai pointer-only args còn lại null. Không tạo `PointerEventData` giả.

## Tự bán đồ

- `GetFreeBagSpace()` là bằng chứng tay nải thật.
- NPC file v2 lưu `Name, ResID, RoleID, MapID, X, Y`; loader vẫn nhận v1.
- Item cell có thể là UIButton hoặc UIRect có Lua handler; cả hai phải nằm trong parent path bag/inventory và không khớp vùng buy/product/NPC item.
- Tối đa 90 callback trong một chuỗi bán.
- Không tăng free space hoặc callback thất bại lặp 3 lần → dừng riêng PID.

## Kiểm tra build tĩnh

- `clang++ -std=c++17 -Wall -Wextra -Werror -fsyntax-only`: PASS.
- `clang -target x86_64-w64-windows-gnu -c src/remote_worker.S`: PASS; output là x86-64 COFF.
- Worker disassembly xác nhận arg5 được lấy từ packet offset `0x30` và ghi vào `[rsp+0x20]`; result ở packet offset `0x38`.

## Chưa xác nhận runtime

Không có client/server đang chạy trong môi trường này. Đặc biệt UIRect-Lua cần test trên UI server thật. State thật (`EnableAutoF1`, `GetFreeBagSpace`) vẫn là điều kiện xác minh sau callback; tool không coi “đã gửi call” là bằng chứng hành động thành công.
