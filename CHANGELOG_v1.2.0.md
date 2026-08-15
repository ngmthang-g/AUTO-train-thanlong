# v1.2.0 - First Real Action: Revive / Đầu thai

- Kế thừa toàn bộ scanner/observer/control-plane/harmless-hook proof đã live PASS ở v1.1.0.
- Protocol `0x00010200`; giữ `ReadGameSnapshot=5`, `ProveHookActionEnvelope=6`, thêm `InvokeReviveButton=7`.
- Global `kGameplayMutationEnabled=false` vẫn giữ; thêm exclusive `kReviveMutationEnabled=true` cho đúng một action.
- AutoFight/NPC/Sell/Path/Treatment/Buff vẫn khóa.
- Bridge thêm `ReviveActionSnapshot` cho token/thread/PRE identity/button evidence.
- Revive engine resolve fresh `UIObject.instances` mỗi attempt, duyệt UIButton/subclass active+interactable, đọc Text/Name và ưu tiên chính xác **Đầu thai**.
- Nhiều candidate đồng điểm hoặc không đọc chắc chắn được control => block fail-closed, không click mù.
- Re-read snapshot + revalidate UIButton ngay trước callback.
- CI audit buộc revive engine có đúng **một** direct `runtime_invoke(handleClick, ...)` và cấm mọi gameplay action khác trong file.
- Controller yêu cầu dead ổn định 2/2, current-session harmless hook proof PASS, metadata Revive READY, SafetyGuard PRE và ActionQueue MAX=1 trước dispatch.
- Queue giữ `active=1` sau callback cho tới POST thật; không retry callback.
- Revive được phép làm MapID đổi. RoleID không được đổi.
- Active Revive được preserve qua map transition chỉ để chờ `dead=0` POST; mọi action mới vẫn khóa.
- POST cần hai snapshot sống ổn định cùng RoleID, MapReady=1, WaitingChangeMap=0 rồi mới `CompleteActive()`.
- Scanner failure trong lúc chờ POST, RoleID drift, dispatch evidence mismatch hoặc POST timeout đều fail closed.
- Integrated control self-test vẫn 48/48; thêm Revive control self-test 16/16.
- Runtime acceptance chỉ cần một death/revive episode; không test AutoFight/NPC/Sell ở mốc này.
