# Clean Route v1.5.0 — audit

- Giữ CleanRoute architecture, multi-account, shared spots, user-configured NPC coordinates.
- Không CreateRemoteThread / WriteProcessMemory / remote_worker / mouse_event.
- Không RequestSellItem / ProcessRemoveItem / direct inventory mutation.
- NPC preset chỉ chứa tên + MapID + NPC ID/ResID; không hard-code X/Y.
- Foreground `SendInput` vẫn dùng cho 6 fixed click points và sell macro.
- Train stable cadence phải có `kTargetRecheckMs = 180000`.
- Deviation correction phải tắt AUTO trước rồi dùng `DriveTravelWithMountFallback`.
- Mount fallback phải giữ: 2 lần mount, mỗi lần chờ 5s, sau đó chạy bộ tối đa 15s rồi lặp.
- Auto Sell route và return-to-train dùng cùng mount fallback.
- Cross-map Confirm phải giữ portal state và retry có giới hạn; không quay route ngay sau một click.
- Post-Đầu-thai Confirm phải retry có giới hạn vì manual TEST pass không đồng nghĩa automatic timing pass.
- `GetFreeBagSpace()` vẫn là read-only bag trigger/completion guard.
- `ClickNPC(npcID)` vẫn là internal action dùng để mở NPC.
- `PROJECT_KNOWLEDGE.md` và `CHANGELOG.md` là file bắt buộc trong source/release từ v1.5.0.
