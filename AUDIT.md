# Rà soát kỹ thuật - Auto Train v0.7.0

Ngày rà soát: 14-08-2026.

## Bản game khóa cứng

- PE timestamp: `0x6A410C14`
- `SizeOfImage`: `0x03DCB000`
- Image base tĩnh: `0x180000000`
- Tool kiểm tra chữ ký machine-code trước khi gửi lệnh IL2CPP.

## API IL2CPP mới

| Method | RVA | Mục đích |
|---|---:|---|
| `GetFreeBagSpace()` | `0x6716F0` | số ô trống |
| `GetNearestNPC()` | `0x673A90` | NPC gần nhất |
| `ClickNPC(int)` | `0x66ADC0` | mở NPC bằng ID thật |
| `LuaMapObjectData.get_RoleID()` | `0x41F000` | NPC ID |
| `LuaMapObjectData.get_Name()` | `0x41F3F0` | NPC name |
| `UIObject.get_Parent()` | `0x530270` | xác minh cây bag |
| `UIToggle.get_Interactable()` | `0x688580` | resolver toggle |
| `UIToggle.get_Selected()` | `0x6885D0` | tránh bật/tắt lặp |
| `UIToggle.get_Text()` | `0x688710` | nhận diện nhãn |
| `UIToggle.set_Selected(bool)` | `0x6888E0` | kích hoạt toggle thật |

Prefix machine-code của các RVA trên đã được đối chiếu trực tiếp với `GameAssembly(1).dll`.

## AUTO → Đánh quái

v0.7.0 dùng `FindAction()` thống nhất: UIButton trước, UIToggle sau; chỉ chọn match có điểm duy nhất. UIButton gọi `HandleClickEvent()`, UIToggle đọc `Selected` rồi gọi `set_Selected(true)`. `get_EnableAutoF1()` vẫn là state xác nhận, không phải action thay thế.

`UIRectTransform.HandlePointerClick` không được gọi với event null/fake vì đường này dereference PointerEventData. Đây là guard chủ động chống crash/disconnect.

## Tự bán đồ

- `LiveState.freeBagSpace` giữ kết quả probe gần nhất từ `GetFreeBagSpace()`; probe đầu tiên chạy khi Start và lần sau theo đúng chu kỳ cấu hình, không spam mỗi vòng worker.
- Probe tay nải bị khóa khi game treo/chết/đang chuyển map.
- `SellNpc` lưu name/roleID/mapID/x/y.
- `ProbeNearestNpc()` lấy NPC ID + tên thật, không hardcode.
- Setting `AutoSell`, `BagCheckMinutes`, `SellNpcName` lưu theo profile.
- Full túi: `DisableActions()` → đi NPC → xuống ngựa → `ClickNPC(id)` → `Bán vật phẩm` → `Bán vật phẩm nhanh`.
- Item cell chỉ được gọi nếu là UIButton và parent path chứng minh thuộc bag/inventory/package/itempack.
- Nếu cây item không chứng minh được hoặc nhận dạng là UIRectTransform, state dừng bán an toàn.
- Tối đa 90 lần gọi item cell; số ô trống được theo dõi xuyên suốt, không dừng ngay sau món đầu tiên. Sau khi đã bán được, tool thử đóng đúng UIButton Close thuộc cây shop rồi quay lại bãi.

## Multi-window / crash guard

Mỗi PID giữ session/worker riêng; spots/NPC dùng chung; setting riêng RoleID. Logic map-ready, WaitingChangeMap, IsDeath, hung-window, remote packet busy và sai tọa độ của v0.5/v0.6 vẫn giữ nguyên.

## UI

- Bỏ hai nút test nội bộ.
- Thêm `Tự bán đồ`, phút kiểm tra, NPC combo, `LƯU NPC GẦN NHẤT`.
- Hiển thị `Túi trống` theo lần probe gần nhất.
- Header/about/resource/build target đổi sang `0.7.0`.

## Kiểm tra source

- `main.cpp`: `clang++ -std=c++17 -fsyntax-only` PASS bằng stub Win32 để kiểm tra C++ độc lập SDK.
- `-Wall -Wextra -Werror` PASS trong cùng vòng kiểm tra (bỏ warning tham số của stub).
- Không có byte NUL lạc trong source.
- Không có RVA constexpr mới bị bỏ không dùng.

## Chưa xác nhận runtime

Không có client/server game đang chạy trong môi trường hiện tại. Layout shop Lua động, đặc biệt kiểu control của item cell, phải test trên client thật. Nếu item cell là UIRectTransform, v0.7 sẽ dừng an toàn thay vì tạo PointerEventData giả.
