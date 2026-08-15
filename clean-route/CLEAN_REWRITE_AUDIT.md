# Clean Route v1.4.0 — audit

- CleanRoute vẫn là codebase rewrite mới, không quay về donor v0.9.0.
- Multi-account: một BridgeClient + RuntimeState + AccountProfile cho từng client; profile ưu tiên RoleID.
- Thư viện `ThanLongCleanRoute.spots.tsv` vẫn dùng chung; bãi được chọn riêng từng acc.
- Route train mutation vẫn là `SendToggleRideState` / `StartAutoPath` / `StopAutoPath`.
- v1.4 cho phép đúng **một mutation nội bộ mới** phục vụ bán đồ: `LuaSystemAPI_Game.ClickNPC(Int32)`, chạy trên callback game window thread.
- Read-only state mới: `LuaSystemAPI_Game.GetFreeBagSpace()`; không ghi/chỉnh inventory memory.
- Không gọi `RequestSellItem`, packet bán, `ProcessRemoveItem`, skill/buff/target/combat API hoặc UIButton/HandleClickEvent.
- Không CreateRemoteThread, WriteProcessMemory, remote_worker, `mouse_event`.
- Gameplay coordinate click là controller foreground `SendInput`; user chủ động chọn mouse-lock mode: click giữ 140 ms, không restore cursor/focus.
- 6 fixed coordinate slots / RoleID: Confirm, Revive, AutoMenu, Attack, StopAuto1, StopAuto2.
- Sell macro / RoleID là danh sách động description + point + delay + repeat.
- Auto Sell trigger/completion dùng `GetFreeBagSpace`; không lấy “macro chạy hết” làm bằng chứng duy nhất rằng bán đã xong.
- NPC preset client-config verified: Mã Kiêu Minh ID 373/M5; Dược Đại Phu ID 279/M55.
- F4 pause là controller-global; khi pause gửi StopPath cho acc RUN và chặn TickAccount tới khi resume.
- Route FSM gốc vẫn dùng `route_logic.h` + self-test 8 case.
