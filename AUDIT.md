# Audit v0.8.3

## NPC

- `ClickNPC(int)` dùng ResID, không dùng RoleID.
- RoleID tĩnh namespace 1,000,000,000 được chuyển thành candidate ResID rồi xác minh bằng `GetNearestNPC(resID)` + RoleID/Name trước khi dùng.
- `SessionData.NPCs/MovingNPCs` chỉ còn fallback cho NPC đặc biệt/di động; không còn là đường bắt buộc khiến NPC tĩnh fail.
- Saved ResID được xác minh lại trước `ClickNPC`.
- NPC bán đồ và NPC trị liệu lưu ở hai file riêng.

## Navigation / AUTO

- Navigation không bị khóa bởi raw `AutoFight=ON`.
- Không loop `AUTO → Dừng` khi chưa tới bãi.
- AUTO UI chỉ được activation ở đúng bãi + map stable + đã xuống ngựa.
- AUTO activation phải xác minh `EnableAutoF1=ON` mới coi là thành công.
- Không dùng `AutoSetFlag(int)` như mode selector; native analysis cho thấy nó liên quan auto-fight range.
- Không gọi UIRect pointer callback với event giả/null cho AUTO combat.

## Cross-server

- Map 10000 → TLT: 15600,8250; target known 10005.
- Map 10000 → KVD: 8195,1190; target MapID chưa có nên match tên KVD.
- Map 10000 → PLT: 1215,8475; target MapID chưa có nên match tên PLT.
- Sau confirm portal phải chờ map transition + MapReady ổn định rồi mới đi target.

## Safety

- Death branch không gọi AUTO UI ở death edge.
- Remote worker packet hỗ trợ 5 native args theo Win64 ABI.
- C++17 syntax check `-Wall -Wextra -Werror`: PASS trong audit environment.
- Clang Static Analyzer: 0 diagnostics.
- `remote_worker.S` assemble Windows x64 COFF: PASS.
