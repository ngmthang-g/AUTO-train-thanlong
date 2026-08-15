# Than Long Clean Route v1.5.0 — 3 phút check tọa + Mount Retry + Confirm Retry

Bản v1.5.0 giữ nguyên nền v1.4.2: multi-account, shared train spots, user-configured NPC X/Y, BagSpace guard, ClickNPC, foreground mouse-lock click, F4/F8, 6 fixed click points và sell macro.

## Quy tắc train mới: chỉ check tọa mỗi 3 phút

Sau khi nhân vật đã tới bãi và `AUTO → Đánh quái` đã chạy:

- snapshot vẫn được đọc bình thường;
- chết/Đầu thai vẫn xử lý ngay;
- túi full vẫn xử lý ngay;
- nhưng controller **không so Map/X/Y để sửa route mỗi tick**;
- chỉ sau **180 giây** mới so lại Map/X/Y với bãi train.

Nếu vẫn đúng bãi → reset timer 180 giây tiếp theo.

Nếu lệch → tool chuyển sang correction:

1. click `DỪNG AUTO 1`;
2. click `DỪNG AUTO 2`;
3. verify `get_EnableAutoF1` nếu getter có dữ liệu;
4. quay về tọa train bằng mount retry/walk fallback;
5. tới nơi → AUTO → Đánh quái lại;
6. bắt đầu lại timer 180 giây.

## Mount retry / chạy bộ fallback

Áp dụng cho:

- correction khi lệch bãi;
- đi NPC bán khi túi full;
- quay từ NPC về bãi sau khi bán xong.

Flow:

1. Nếu đang riding → AutoPath bình thường.
2. Nếu chưa riding → gửi lên ngựa lần 1.
3. Chờ **5 giây**.
4. Nếu vẫn chưa riding → gửi lên ngựa lần 2.
5. Chờ thêm **5 giây**.
6. Nếu vẫn chưa riding → cho AutoPath chạy bộ tối đa **15 giây**.
7. Nếu chưa tới đích sau 15 giây → StopPath nếu cần rồi quay lại bước lên ngựa lần 1.
8. Nếu riding thành công ở bất kỳ thời điểm nào → bỏ fallback và tiếp tục AutoPath bằng ngựa.

## Fix XÁC NHẬN RA MAP

Runtime v1.4.2 đã xác nhận cả 6 điểm click đều lấy/test được, nhưng riêng Confirm tự động trong flow thật không đáng tin dù TEST tay OK. Vì vậy v1.5.0 sửa **timing/state**, không sửa tọa click.

### Cross-map

- phải từng thấy AutoPath chạy tới cổng;
- khi AutoPath dừng ở sai map và nhân vật đứng yên, giữ trạng thái portal;
- chờ **2,5 giây** trước click Confirm đầu tiên;
- retry mỗi **1,8 giây**, tối đa **4 lần**;
- nếu map bắt đầu chuyển thì top-level guard chặn click tiếp;
- chỉ sau khi hết toàn bộ retry window mà vẫn không đổi map mới reset portal state và cho AutoPath bắt lại cổng.

### Sau Đầu thai

- click Đầu thai như cũ;
- chờ **1,4 giây** trước Confirm đầu tiên;
- retry mỗi **1,8 giây**, tối đa **4 lần** nếu vẫn còn dead và chưa chuyển map.

## 6 điểm click riêng từng acc

1. XÁC NHẬN RA MAP
2. ĐẦU THAI
3. AUTO
4. ĐÁNH QUÁI
5. DỪNG AUTO 1
6. DỪNG AUTO 2

Gameplay click dùng foreground `SendInput`: focus cửa sổ game, đưa chuột tới điểm, giữ LEFTDOWN 140 ms, nhả chuột; không restore chuột/focus theo chế độ mouse-lock hiện tại.

## Auto bán đồ

NPC preset chỉ giữ MapID + NPC ID/ResID:

- Mã Kiêu Minh — MapID 5 — ID 373.
- Dược Đại Phu — MapID 55 — ID 279.

**Không có X/Y NPC mặc định trong code.** Người dùng tự nhập hoặc bấm `LẤY VỊ TRÍ`. X/Y được lưu dùng chung theo từng NPC.

Khi `GetFreeBagSpace() <= 0`:

`Stop AUTO → mount retry/walk fallback → NPC X/Y người dùng → ClickNPC(ID) → sell macro → FreeBagSpace > 0 stable → mount retry/walk fallback về bãi → AUTO → Đánh quái`

Nếu chưa có tọa NPC hoặc chưa cấu hình sell macro, flow fail-closed.

## Knowledge Base

Theo `AI_PROJECT_KNOWLEDGE_PROTOCOL`:

- `PROJECT_KNOWLEDGE.md` = bộ nhớ kỹ thuật dài hạn;
- `CHANGELOG.md` = thay đổi version ngắn gọn;
- build artifact/source phải mang theo cả hai file này.

## Trạng thái test

- Manual TEST 6 click: **RUNTIME PASS theo user** trên v1.4.2.
- v1.5.0 build: xem GitHub CI của commit release.
- v1.5.0 runtime cadence/mount fallback/Confirm retry: **NEEDS USER TEST**.
