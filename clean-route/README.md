# Thần Long Clean Route v1.5.6 — Death Session Cold Restart

## Vì sao đổi cơ chế

v1.5.5 vẫn reset theo từng nhóm state khi bắt đầu một lần chết. Cách đó có rủi ro sót state của đời trước như map qualification, fight/recovery/travel/sell/watchdog. v1.5.6 đổi sang **session boundary reset**: dùng chính `ResetRuntime()` giống nút `BẮT ĐẦU`, thay vì tiếp tục thêm danh sách cờ cần reset.

## Cơ chế mới

1. Authoritative `dead=1` lần đầu của một death session → `ResetRuntime()` toàn bộ runtime của đúng acc.
2. `deathSessionLatched` nằm ngoài `RuntimeState`, vì vậy hard reset không làm mất dấu death session và không tạo vòng reset vô hạn.
3. Tool thực hiện Đầu thai như cũ.
4. Khi authoritative `dead=0` trở lại trên client/map ổn định → `ResetRuntime()` **lần hai**.
5. Tick kế tiếp bắt đầu lại như vừa bấm `BẮT ĐẦU`: qualify map từ đầu → M87 AutoFight guard → mount/AutoPath → semantic MessageBox Confirm → train.

**Không mất setting:** bãi train, tọa độ 6 click, Auto bán đồ, NPC, macro và các tùy chọn nằm trong `AccountProfile`, không nằm trong `RuntimeState`. Bridge attachment cũng được giữ nguyên.

Log mới:
```text
NEW DEATH SESSION: HARD ResetRuntime toàn bộ • coi như AUTO vừa được bật lại từ đầu.
POST-REVIVE COLD START: ResetRuntime toàn bộ • giữ nguyên setting/bãi/click • phiên auto mới.
```

## Địa Phủ

Địa Phủ vẫn là **MapID 87** và guard AutoFight OFF của v1.5.5 được giữ nguyên.

## Confirm / Transition Freeze

Không thay tọa độ hay click engine Confirm đã runtime PASS. v1.5.3 semantic `MessageBox` Confirm và v1.5.4 Transition Freeze vẫn giữ nguyên.

## Runtime test cần làm

Test ít nhất 3 lần chết liên tiếp. Mỗi lần phải thấy đúng một `NEW DEATH SESSION`, sau khi sống lại đúng một `POST-REVIVE COLD START`, rồi route/Confirm phải diễn ra như một phiên Auto mới.

---

## Vì sao có v1.5.5

v1.5.4 đã chứng minh lần đầu có thể nhận `MessageBox=1` khi AutoPath còn ON, StopPath, click Xác nhận và đóng băng action trong lúc chuyển map. Tuy nhiên user test tiếp cho thấy sau **lần chết thứ hai**, nhân vật chạy ra lại nhưng automatic Confirm không còn xuất hiện như vòng đầu.

Không coi đây là lỗi click: saved Confirm đã runtime PASS. v1.5.5 sửa hai source gap có giá trị cao mà không thay cơ chế Confirm đang chạy được.

## Địa Phủ chính xác là Map 87

Từ canonical client DATA `database/MAPS.csv`:

`87,Địa Phủ,siwang,0,Wild,1,siwang`

Không đoán MapID.

## 1. NEW DEATH CYCLE

Mỗi lần bắt đầu một death cycle mới, tool reset sạch:
- cross-map route ownership;
- AutoPath/movement evidence;
- stall timing;
- Confirm retry/debounce/last-click timing;
- Confirm StopPath timing;
- route suppression;
- Địa Phủ stop state.

Log:
`NEW DEATH CYCLE: reset route/Confirm ownership trước Đầu thai.`

Mục tiêu: lần chết 2, 3, 4... đều bắt đầu như một route mới, không kế thừa state từ đời trước.

## 2. ĐỊA PHỦ M87 — AutoFight phải OFF trước route

M87 có ownership guard ưu tiên cao hơn route/Confirm/sell/recovery.

Nếu AutoFight getter chưa authoritative:
- chờ;
- không click mù.

Nếu AutoFight ON:
1. click saved `DỪNG AUTO 1`;
2. chờ 700 ms;
3. click saved `DỪNG AUTO 2`;
4. đọc lại getter;
5. OFF → cho route chạy;
6. vẫn ON sau 1200 ms → retry toàn bộ chuỗi; tối đa 3 lần.

Log mong đợi:
```text
ĐỊA PHỦ M87: AutoFight=ON → DỪNG AUTO 1.
ĐỊA PHỦ M87: DỪNG AUTO 2 → verify AutoFight OFF.
ĐỊA PHỦ M87: AutoFight OFF đã xác nhận → cho phép tiếp tục route về bãi.
```

## 3. Transition Freeze giữ nguyên

Khi map/client/Bridge không ổn hoặc ngay sau Confirm:
- FREEZE toàn bộ mutable action;
- không ghi đè timed-out request;
- chỉ resume sau fresh ReadState + MapReady + !WaitingChangeMap + responsive window ổn liên tục 2 giây.

## 4. Cross-map Confirm giữ nguyên

`tool-owned route → MessageBox ON + moving OFF → StopPath nếu cần → fresh snapshot → saved Confirm click → Transition Freeze`.

Không đổi saved coordinate/click engine đã runtime PASS.

## Validation
- Map 87 identity: **VERIFIED**.
- v1.5.4 first Confirm/freeze path: **RUNTIME PARTIAL PASS**.
- second-death missing automatic Confirm: **RUNTIME FAIL/PARTIAL observed**.
- exact causal link from AutoFight to the missed Confirm: **LIKELY**, not yet proven.
- v1.5.5 Windows code-validation CI: **PASS** — run `31934654526`, job `95134583587`; runtime: **NEEDS USER TEST**.
