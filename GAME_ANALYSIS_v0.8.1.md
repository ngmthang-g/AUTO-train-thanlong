# Phân tích bộ file game dùng cho phát triển Auto Train

## Nhận dạng build

- `GameAssembly.dll`
  - SHA-256: `4c98c9934bc4260efa64f5492c58e0c5104c89359f0126e7cd402feb381fe3c7`
  - PE TimeDateStamp: `0x6A410C14`
  - SizeOfImage: `0x03DCB000`
- `global-metadata.dat`
  - SHA-256: `d199498dad7d3139e4c09f6742f4645bfc2a33c465e3d259196931199f6ee6a8`
  - IL2CPP metadata version: **39**

Hai file lõi trong `New folder (2).zip` trùng đúng build đã dùng để phát triển các bản 0.7/0.8 trước đó.

## API/điểm vào đang có giá trị cho tool

- `LuaSystemAPI_Game.GetFreeBagSpace()` → RVA `0x6716F0`
- `LuaSystemAPI_Game.ClickNPC(int)` → `0x66ADC0`
- `LuaSystemAPI_Game.GetNearestNPC()` → `0x673A90`
- `LuaSystemAPI_Game.get_EnableAutoF1()` → `0x67B440`
- `LuaSystemAPI_Game.AutoRemoveFlag()` → `0x66A1C0`
- `LuaSystemAPI_Game.AutoSetFlag(int RangerAuto)` → `0x66A210`
- `UIButton.HandleClickEvent()` → `0x52D140`
- `UIToggle.HandleSelectEvent(bool)` → `0x687450`
- `UIRectTransform.get_OnPointerClickHandler()` → `0x644B50`
- `UIInput.set_Text(string)` → `0x63C270`
- `UIFactory.SendDefaultChat(string)` → `0x6FAAD0`

### AUTO

`AutoSetFlag(int RangerAuto)` là hướng rất đáng nghiên cứu vì có thể bỏ hẳn việc mở menu, nhưng chưa có bằng chứng tĩnh đủ để xác định giá trị `RangerAuto` tương ứng `Đánh quái`. Không gọi đoán trong production.

### Chat

Metadata có `UIFactory.SendDefaultChat`, nhưng video cho thấy gameplay thực tế phụ thuộc trạng thái bảng/kênh Chat. Vì vậy v0.8.1 chuyển sang thao tác trên UI object thật, vẫn hoàn toàn chạy bằng call nội bộ.

### Resource/client phụ trợ

`FGClientTool_Windows.dll` SHA-256 `cab5148fa70ae231e7245d62d8448b7881c16ed64bd8c84558f68e21d6ecd9a0`, export:

- `FG_Decrypt`
- `FG_Encrypt`
- `HelloWorld`

Các bundle `.unity3d` không đồng nhất header: một số bắt đầu trực tiếp bằng `UnityFS`, một số có 4 byte khác trước chuỗi `UnityFS`. Điều này cho thấy cần xử lý riêng lớp header/transform của client trước khi kỳ vọng tool Unity bundle thông thường đọc được tất cả resource. Chưa đủ dữ liệu để kết luận chính xác thuật toán mã hóa.

`UnityPlayer.dll` SHA-256 `9feedb4527a30fae4762bff45d204f8e85b90851b2c8ae8d24f436f7d474667b`, export `UnityMain` và `UnityMain2`.

Bộ ZIP còn gồm `data.unity3d`, `Interface.unity3d`, `Config.unity3d`, `Translations.unity3d`, `Shared*.unity3d`, `LoadingResources.unity3d`, Burst, baselib và LiveKit FFI. Giữ bộ này làm corpus cho các chức năng UI/chat/shop tiếp theo.
