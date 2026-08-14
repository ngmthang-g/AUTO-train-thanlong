# Thần Long Mobile - Auto Train v0.8.7

## v0.8.7 - bán duy nhất ô Trang bị số 2

- Sau khi vào Bán vật phẩm nhanh và tab Trang bị, tool bỏ ô hiển thị số 1.
- Tool resolve lại đúng ItemBox đang hiển thị ở vị trí số 2 rồi gọi `ButtonItemClicked` mỗi 1 giây, tổng 90 nhịp.
- Không giữ pointer ô cũ: BagItemsGrid Clear + RebuildLayout sau khi server bán thành công nên món tiếp theo dịch vào vị trí hiển thị số 2 bằng một ItemBox khác.
- Không còn quét/click toàn bộ 33/100 ItemBox và không còn coi `GetFreeBagSpace` phải tăng là điều kiện bán thành công. Sau 90 nhịp tool đóng shop/tay nải và trả quyền cho Auto Train.


## v0.8.7 - NPC exclusive action guard
- Trị liệu và bán đồ chạy trong miền thao tác độc quyền theo từng PID: trong chuỗi NPC không phát AutoPath/ngựa/AUTO/map-check khác.
- Sau khi đóng UI NPC/shop, client được giữ yên 900 ms rồi phải qua 2 lần RefreshLive ổn định mới tiếp tục navigation. Điều này tránh lỗi `Mất phản hồi trạng thái chuyển map` ngay sau callback Lua/server.
- Xóa các cờ chuyển-map còn treo khi bắt đầu chuỗi NPC để timer cổng/map cũ không chạy chen vào shop/trị liệu.
- Thêm preset ResID `Long Phá Thiên = 463`; khi lưu ở Lạc Dương Liên Server, MapID realtime 10000 + X/Y realtime được ghi riêng vào danh sách NPC trị liệu. Config asset gốc bind cùng template 463 ở Lạc Dương thường (Map 3), vì vậy tọa độ Liên Server không được bịa/hardcode.

# Thần Long Mobile - Auto Train v0.8.7

Bản này tập trung vào ba lỗi runtime đã test được ở v0.8.4: chuỗi trị liệu/shop quá chậm, shop đã tới tab Trang bị nhưng chưa bán, và AUTO đã tới bãi nhưng `AutoTrainClick` gọi trực tiếp không tạo trạng thái đánh quái.

## 1. Trị liệu nhanh hơn nhưng vẫn tuần tự

Không gửi các action song song. Chuỗi vẫn là:

`ClickNPC -> Trị liệu -> Xác nhận -> Ta biết rồi -> đóng UI`

Điểm thay đổi là polling UI từ 140–150 ms xuống 35 ms và bỏ các khoảng chờ cố định 180–260 ms không cần thiết. Mỗi bước chỉ được gọi sau khi control của bước đó thực sự xuất hiện. Vì vậy phản hồi bình thường nhanh khoảng 4 lần nhưng không đổi thành macro spam.

## 2. Bán đồ: dùng đúng ItemBox của BagItemsGrid

Từ `Interface.unity3d`:

- `NPCShop_SellItemTab` đặt `CustomClickOnItemBox` cho `BagItemsGrid`.
- Khi `ToggleQuickSell.Selected=true`, click một ItemBox gọi `RequestSellItem(dbItemData)`.
- `ItemBox_Layout` dùng `UIButton ClickHandler="ButtonItemClicked"`.
- Danh sách mua lại bên trái dùng các handler khác (`ButtonItemIconClicked`, `ButtonBuyBackItemClicked`).

v0.8.7 đọc thêm `UIButton.get_ClickHandler()` (RVA `0x52DF50`) và chỉ nhận ItemBox có handler `ButtonItemClicked` nằm dưới `SellItemTab`. Không còn phụ thuộc tên slot/item do runtime sinh ra.

Sau khi vào `Trang bị`, tool bỏ ItemBox đang hiển thị đầu tiên. Trong 90 nhịp liên tiếp, mỗi nhịp cách nhau 1 giây, tool resolve lại ItemBox đang hiển thị thứ 2 trong `ItemsGrid` rồi gọi đúng `ButtonItemClicked` của nó. Sau khi server bán món đó, `BagItemsGrid:RemoveItem()` Clear ô cũ và `DoFilter()`/`RebuildLayout()` làm món tiếp theo dịch vào đúng vị trí hiển thị số 2; tool vì vậy không giữ pointer cũ. Hết 90 nhịp thì đóng shop/tay nải và quay lại state train, không phụ thuộc `GetFreeBagSpace` có tăng hay không.

## 3. AUTO -> Đánh quái: replay đúng hai UIButton thật

Asset client cho thấy:

- nút gốc `ButAutoFight`, handler `AutoFightClick`, chỉ mở/đóng `AutoFightGroup`;
- nút con `Đánh quái`, handler `AutoTrainClick`;
- `AutoTrainClick` mới gọi `AutoFight_Main:StartAutoFight(C_AutoModel.Train)`.

v0.8.4 gọi `TopIcon.AutoTrainClick` trực tiếp bằng executor; runtime test cho thấy callback được gửi nhưng `EnableAutoF1` không đổi. v0.8.7 bỏ đường đó khỏi production AUTO và replay đúng event chain của game:

`UIButton(AutoFightClick) -> chờ AutoFightGroup active -> UIButton(AutoTrainClick) -> verify EnableAutoF1=OFF`.

`AUTO -> Dừng` tương tự dùng `AutoFightClick -> AutoStopClick -> verify EnableAutoF1=ON`.

Tool đọc ClickHandler thật của UIButton nên không còn phải đoán nút AUTO/Đánh quái chỉ từ chữ hiển thị. Text matching chỉ là fallback nếu handler chưa đọc được.

Navigation vẫn có ưu tiên tuyệt đối: AUTO combat chỉ chạy sau khi đúng map, đúng tọa độ và đã xuống ngựa; không chạy song song với AutoPath/ngựa/cổng.

## 4. NPC và Liên Server giữ nguyên từ v0.8.4

Built-in client config:

- Mã Kiêu Minh: ResID 373.
- Đỗ Thanh Đằng: ResID 339.

Liên Server từ Map 10000:

- Thanh Liên Trại 10005: `15600,8250`.
- Khô Vinh Đạo 10007: `8195,1190`.
- Phàm Liên Trại 10004: `1215,8475`.

## Build

Chạy `build.cmd` trên Windows hoặc workflow GitHub.

Output: `dist\ThanLongAutoTrain_v0.8.7.exe`

## Test ưu tiên

1. Trị liệu: so tốc độ từng bước với v0.8.4.
2. Full túi: vào Trang bị, bỏ ô 1; status cuối phải ghi hoàn thành 90 nhịp ô số 2 rồi đóng shop/tay nải.
3. AUTO tại bãi: phải thấy popup AUTO được mở rồi mục Đánh quái được gọi; thành công chỉ khi `EnableAutoF1=OFF`.
