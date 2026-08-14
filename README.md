# Thần Long Mobile - Auto Train v0.8.4

Bản này sửa trực tiếp ba lỗi runtime được xác nhận từ v0.8.3 bằng dữ liệu lấy từ chính asset client đã giải mã: NPC ID thật, action Lua thật của AUTO và action mở ChatBox thật.

## 1. NPC bán đồ / trị liệu: bỏ suy RoleID -> ResID đối với hai NPC chuẩn

Từ `Config.unity3d` của chính client:

- `Mã Kiêu Minh`: NPC/ResID **373**, `NPCData MapID=5`.
- `Đỗ Thanh Đằng`: NPC/ResID **339**, `NPCData MapID=5`.

RoleID runtime như `1000000378` không phải `1,000,000,000 + ResID`; phép suy cũ là sai. v0.8.4 có database built-in theo tên và tự sửa ResID của entry cũ khi load. Khi bấm Lưu NPC, tool vẫn lấy Map/X/Y realtime của nhân vật đang đứng sát NPC, nhưng ResID lấy từ config client thay vì quét Dictionary mong manh.

Khi mở NPC, tool đọc NPC gần nhất và bắt buộc tên phải đúng với entry đã chọn rồi mới `ClickNPC(373/339)`. Không dùng RoleID làm tham số `ClickNPC`.

## 2. AUTO -> Đánh quái: gọi đúng Lua action, không quét menu con

`Interface.unity3d` cho thấy chính xác:

- `TopIcon:AutoTrainClick()` -> `AutoFight_Main:StartAutoFight(C_AutoModel.Train)`.
- `TopIcon:AutoStopClick()` -> `AutoFight_Main:StartAutoFight(C_AutoModel.None)`.
- `C_AutoModel.Train = 1`, `None = 0`.

Vì vậy v0.8.4 không còn phụ thuộc mở popup AUTO rồi dò label `Đánh quái`. Tool gọi trực tiếp action Lua của `TopIcon` qua `MonoBehaviourExecutor.ExecuteScriptFunction` với `object[0]` thật.

Một lỗi logic quan trọng cũng được sửa: Lua gốc **set `Game.EnableAutoF1=false` khi bắt đầu Train**, còn `StopAllCurrentTask()` khôi phục **true** khi dừng. Các version cũ kiểm tra ngược nên có thể báo “AUTO đã bật” giả khi nhân vật vẫn đứng im. `LiveState.autoFight` và verify Start/Stop đã đảo lại đúng semantics này.

Navigation vẫn có ưu tiên tuyệt đối: action AUTO chỉ được gửi khi đã tới bãi, map ổn định và xuống ngựa xong.

## 3. Auto Chat

Asset client cho thấy `BottomIcon:ButtonOpenChatBoxClicked()` tự gọi `GUI.CallUI("ChatBox")` nếu bảng chat chưa tồn tại. v0.8.4 dùng action này để mở chat, set `UIInput.Text`, rồi gọi trực tiếp đúng hai callback Lua `ChatBox:ButtonSendMessageClicked()` và `ChatBox:ButtonCloseClicked()`. Không giữ UIButton Chat cũ qua các bước.

Auto Chat chỉ chạy khi nhân vật đã ở đúng bãi, không AutoPath, không đang di chuyển và không cưỡi ngựa. Sau chat có **1 giây action barrier**: không gửi AutoPath/ngựa/AUTO/NPC trong cùng lượt worker. Sau barrier state machine train tiếp tục bình thường.

## 4. Train Liên Server

Từ MapID 10000:

- Thanh Liên Trại `10005`: cổng `15600,8250`.
- Khô Vinh Đạo `10007`: cổng `8195,1190`.
- Phàm Liên Trại `10004`: cổng `1215,8475`.

Nhận diện cả bằng MapID lẫn tên bãi. Sau xác nhận cổng phải chờ map transition + MapReady ổn định rồi mới đi tọa độ train.

## 5. Ổn định / chống lệnh chồng

- Không Auto Chat trong lúc navigation.
- Sau Chat khóa action 1 giây.
- Khi rời bãi, nếu tool vừa gửi `AUTO -> Dừng`, khóa AutoPath/ngựa/xác nhận cổng thêm 700 ms để Lua/UI hoàn tất trước lệnh navigation tiếp theo.
- AUTO combat không quét menu transient nữa.
- NPC built-in không quét `SessionData.NPCs/MovingNPCs` để suy ID.
- Map transition vẫn bắt buộc hai lần đọc ổn định.
- Remote worker vẫn chống ghi đè khi packet cũ chưa xong.

## Build

Chạy `build.cmd` trên Windows hoặc workflow GitHub.

Output: `dist\ThanLongAutoTrain_v0.8.4.exe`

## Test ưu tiên

1. Đứng cạnh Mã Kiêu Minh -> `LƯU NPC BÁN ĐỒ GẦN NHẤT`: status phải hiện ResID 373.
2. Đứng cạnh Đỗ Thanh Đằng -> `LƯU NPC TRỊ LIỆU GẦN NHẤT`: status phải hiện ResID 339.
3. Test AUTO mode tại bãi: không cần popup AUTO mở; log phải nói `TopIcon.AutoTrainClick` và `EnableAutoF1=OFF`.
4. Test Auto Chat khi đang ở bãi; gửi xong phải quay lại state train sau action barrier.
5. Test KVD 10007 và PLT 10004 từ Map 10000.
