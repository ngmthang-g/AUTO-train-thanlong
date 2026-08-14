# Rà soát kỹ thuật - Auto Train v0.8.1

## Auto chat

Video người dùng thể hiện chuỗi thao tác UI thật: mở Chat, nhập nội dung ở ô chat, gửi, rồi đóng bảng. Metadata IL2CPP v39 xác nhận class `FGStudio.LuaSystem.GUI.UIInput` và method `set_Text` tại RVA `0x63C270`. v0.8.1 dùng chính `UIObject.instances` để tìm UIInput trong cây Chat và gọi setter managed; sau đó resolver gọi control `Gửi tin nhắn` và `Đóng`.

`UIFactory.SendDefaultChat(string)` vẫn tồn tại trong game nhưng không còn là đường chính vì runtime thực tế không tái hiện đúng workflow/kênh chat người dùng cần.

## Trị liệu

Chuỗi được thực hiện tuần tự bằng resolver nội bộ:

`ClickNPC(RoleID hiện tại) → Trị liệu → Xác nhận → Ta biết rồi`.

Không PostMessage/click tọa độ màn hình cho chuỗi này.

## AUTO → Đánh quái

Hồi quy v0.8.0 đến từ việc mở rộng resolver AUTO sang toàn bộ `UIRectTransform` có Lua click handler. Quét tất cả rect + cây con vừa nặng vừa tăng nguy cơ gọi nhầm handler khi UI game thay đổi.

v0.8.1 giới hạn production path:

- AUTO root: `UIButton` duy nhất, giống đường bản cũ đã mở menu ổn định.
- `Đánh quái`: `UIButton` hoặc `UIToggle`.
- `Dừng`: `UIButton` hoặc `UIToggle`.
- Không gọi `UIRectTransform` trong AUTO production path.
- Sau callback phải xác minh `LuaSystemAPI_Game.get_EnableAutoF1()`.
- Retry thất bại: 3 giây.

Metadata còn có `LuaSystemAPI_Game.AutoSetFlag(int)` và `AutoRemoveFlag()`, nhưng ý nghĩa giá trị `RangerAuto` chưa được chứng minh đủ. Bản này **không đoán flag** để tránh gây trạng thái sai hoặc crash. Đây là hướng nghiên cứu kế tiếp nếu cần bypass hoàn toàn menu AUTO.

## NPC tách dữ liệu

Hai nút lưu dùng hai vector, hai file và hai lookup độc lập. Không còn trường hợp lưu NPC trị liệu ghi đè/chèn chung vào danh sách bán đồ.

## Buff

Mỗi skill buff được thử một lần mỗi chu kỳ. `TryApplyAndVerifyBuff()` vẫn kiểm tra buff trước/sau cast; nếu không thấy effect thì chuyển ngay sang skill tiếp theo và chỉ thử lại ở chu kỳ 5 phút sau.

## Kiểm tra source

- `main.cpp`: C++17 syntax check với `-Wall -Wextra -Werror` qua Win32 compatibility stubs: PASS.
- `remote_worker.S`: assemble target Windows x64 COFF: PASS.
- Worker 5 tham số giữ nguyên ABI: arg5 được đặt ở `[rsp+0x20]`.
- Không thay đổi đường `SelectedSkill` đang hoạt động ổn ngoài việc dùng chung state machine hiện hữu.
