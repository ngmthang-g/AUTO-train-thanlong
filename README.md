# Thần Long Auto - NewCore v1.1.0 Integrated Acceptance

v1.1.0 gom các phase kiểm chứng nhỏ trước đây thành **một mốc tích hợp** để giảm tối đa runtime test thủ công.

## Pipeline

`Resolver -> Scanner(read-only) -> Snapshot/State Store -> Observer -> FSM -> SafetyGuard -> ActionQueue(MAX=1) -> Hook/MainThread Dispatcher -> Internal Action Engine`

Scanner/Observer và map-transition guard kế thừa runtime evidence đã PASS từ v1.0.10/v1.0.11.

## Control plane

- FSM: SAFE_PAUSED / SCANNER_QUALIFYING / IDLE_STABLE / DEAD_DETECTED / MAP_TRANSITION / ACTION_PREPARED / AWAITING_POSTCONDITION / FAULTED.
- SafetyGuard PRE-condition matrix cho identity, scanner, observer, transition, watchdog và life-state.
- ActionQueue capacity compile-time đúng 1, mô hình queued/active.
- ScannerWatchdog fail-closed sau 3 lỗi liên tiếp.
- Postcondition framework cho infrastructure proof, Revive và combat automation.
- Gameplay DispatcherGate tách khỏi InfrastructureDispatcherGate.
- `kGameplayMutationEnabled = false` vẫn khóa compile-time.
- Protocol `0x00010102`.
- BridgeCommand `ReadGameSnapshot=5`; command `ProveHookActionEnvelope=6` là **infrastructure-only**, không phải gameplay command.

## Harmless hook/main-thread action envelope

Mốc này chứng minh đường **ActionQueue -> SafetyGuard -> hook/main-thread callback -> POST** mà không gọi gameplay method.

Sau khi scanner đủ 60/60 trên identity ổn định và nhân vật alive:

1. Controller tạo `HarmlessInfrastructureProof` với token duy nhất.
2. SafetyGuard bắt buộc bridge attached, MainThread proven, scanner qualified/healthy, observer stable, không transition và identity khớp.
3. ActionQueue nhận đúng 1 intent rồi chuyển `queued -> active`; trong lúc active không intent thứ hai được phép vào.
4. `ProveHookActionEnvelope=6` được post bằng cùng `kWakeMessage` tới `TlGetMessageHook`.
5. Bridge **re-prove Unity main thread** rồi chỉ tăng một counter nội bộ bridge; không resolve/call UI, NPC, combat, path, inventory hay shop action.
6. Bridge echo token + proof sequence + callback TID + managed current/main IDs.
7. Controller chỉ `CompleteActive()` khi POST xác minh token đúng, sequence tăng, callback TID đúng PRE hook thread và managed current/main vẫn đúng Unity main thread.
8. Sai bất kỳ điều kiện nào => fail-closed, clear queue, close bridge.

Build audit cấm trực tiếp các token gameplay `ClickNPC`, `HandleClickEvent`, `StartAutoFight`, `RequestSellItem`, `StartPath`, `StopPath` và direct `runtime_invoke` trong `harmless_action_envelope.inc`.

Đây là **proof của execution envelope**, không phải proof của gameplay mutation. Gameplay vẫn bị khóa.

## Action / Dispatcher Capability Resolver v3 — read-only

Foundation sau MainThread proof chạy metadata-only capability probe trên **current client**. Probe chỉ dùng class/method/field/type metadata, không gọi managed action, không lấy live UIButton và không mutate game.

### UI/NPC/inventory

Probe xác minh metadata cho:

- `UIObject.instances`, active/children;
- `UIButton.get_Interactable`, `get_Text`, `HandleClickEvent()`;
- `UIToggle` selected/set/select-event;
- `LuaSystemAPI_Game.ClickNPC(Int32)`, `GetNearestNPC(Int32)`;
- `GetFreeBagSpace`, `GetItemsAtSite(Int32)`, `GetItemData(Int32)`;
- `GetItemType`, `GetEquipType`, `IsItemSellable`.

### Lua executor không đoán namespace

Hai export `il2cpp_image_get_class_count` + `il2cpp_image_get_class` được resolve **optional** để duyệt Assembly-CSharp. Probe tìm class duy nhất tên `MonoBehaviourExecutor` rồi yêu cầu:

- static `get_Instance()`;
- instance `ExecuteScriptFunction(FGStudio.LuaSystem.Base.UIObject,System.String,System.Object[])`.

Không tìm thấy/không duy nhất => PARTIAL/AMBIGUOUS, action vẫn khóa.

### Dispatcher dựa trên runtime evidence thật

Runtime metadata log cũ của chính client đã từng in:

- `FGStudio.Engine.Utilities.MainThread`: static `get_Instance()` và instance `Execute(System.Action)`;
- `UnityMainThreadDispatcher`: static `get_Instance()`, instance `Enqueue(System.Action)`, static `Dispatch(System.Action)`.

Probe v3 dùng đúng các signature này làm capability gate thay vì suy đoán từ chuỗi metadata thô. Các capability chỉ chứng minh **method surface hiện diện**; NewCore hiện chưa cần tạo `System.Action` để làm harmless proof vì chính `WH_GETMESSAGE` callback đã là đường vào proven Unity main thread.

### Dynamic Lua vẫn donor-only

Legacy v0.9.0 (pre-NewCore) giữ evidence:

- `AutoFight_Main.StartAutoFight(Train/None)`;
- `NPCShop_SellItemTab.RequestSellItem(DBItemData)`.

Hai đường này không được tự nâng thành current-client runtime capability. Gameplay action thật chỉ được mở sau fresh-object/method resolution + ActionQueue + proven main-thread envelope + POST/ACK thật.

## Automated self-test

`build.cmd` compile/chạy `ControlSelfTest.exe` trước bridge/controller. Test hiện bao phủ **48 checks**: FSM, SafetyGuard, identity drift, map transition, queue MAX=1 queued/active/cancel, watchdog, gameplay dispatcher lock, infrastructure dispatcher gate, harmless token/sequence/thread POST và Revive/combat postconditions.

Build audit bắt buộc capability probe metadata-only và harmless envelope không chứa gameplay/invoke token.

## Runtime-test policy

Không test từng version nhỏ. Mốc code/CI này vẫn chưa cần người dùng test riêng; integrated runtime acceptance sau cùng sẽ xác nhận command 6 trên client thật cùng các capability/action được mở ở phase sau.

## Golden boundary

Legacy v0.9.0 chỉ là donor. Không RemoteExecutor, remote gameplay worker, gameplay CreateRemoteThread, il2cpp_thread_attach runtime path, Sleep-driven transition, cached UI pointer hay blind retry.
