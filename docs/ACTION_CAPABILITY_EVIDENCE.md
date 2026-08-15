# Action capability evidence - NewCore v1.1.0

## Evidence levels

NewCore tách ba mức bằng chứng:

1. **Donor evidence** — Legacy v0.9.0 (pre-NewCore), asset hoặc native analysis cũ chứng minh tên/semantic từng đúng trên build đã biết.
2. **Current-client metadata evidence** — client đang chạy có đúng class/method/field/signature.
3. **Runtime action proof** — fresh runtime object + proven main-thread dispatch + POST/ACK thật.

Chỉ mức 3 cùng SafetyGuard + ActionQueue mới đủ điều kiện mở gameplay action.

## Donor evidence giữ lại

- `FGStudio.LuaSystem.Base.UIObject.instances`.
- `FGStudio.LuaSystem.GUI.UIButton.HandleClickEvent()` là instance method; stale/null `this` không an toàn.
- `FGStudio.LuaSystem.GUI.UIToggle` selected/select-event semantics.
- `FGStudio.LuaSystem.API.LuaSystemAPI_Game.ClickNPC(Int32)`, với tham số ổn định là NPC ResID.
- Inventory: `GetFreeBagSpace`, `GetItemsAtSite`, `GetItemData`, `GetItemType`, `GetEquipType`, `IsItemSellable`.
- `MonoBehaviourExecutor.get_Instance()` donor token `0x0600035B`.
- `MonoBehaviourExecutor.ExecuteScriptFunction(UIObject,string,object[])` donor token `0x06000372`.
- Lua combat: `AutoFight_Main.StartAutoFight(Train/None)`, Train=1 / None=0.
- Lua sell: `NPCShop_SellItemTab.RequestSellItem(DBItemData)`; ACK donor là `GetItemData(DBID)==null` sau server removal.

## Current-client metadata probe v3

`src/bridge/action_capability_probe.inc` chỉ inspect metadata. Không gọi managed action và không lấy live gameplay/UI object.

### UI / NPC / Inventory support masks

- Revive UI support: UIObject type + instances + active/children + UIButton type/interactable/text/HandleClickEvent.
- NPC support: LuaSystemAPI_Game + ClickNPC(Int32) + GetNearestNPC(Int32).
- Inventory support: LuaSystemAPI_Game + GetFreeBagSpace + GetItemsAtSite(Int32) + GetItemData(Int32).
- Sell classification support: inventory support + GetItemType + GetEquipType + IsItemSellable.

### Lua executor discovery

Optional `il2cpp_image_get_class_count/get_class` enumerate Assembly-CSharp without guessing namespace. A unique `MonoBehaviourExecutor` must expose:

- static `get_Instance()`;
- instance `ExecuteScriptFunction(FGStudio.LuaSystem.Base.UIObject,System.String,System.Object[])`.

The resulting bits are `CapMonoExecutorType`, `CapMonoExecutorInstanceGetter`, `CapMonoExecutorExecuteScript`; all three form `kLuaExecutorMetadataSupportMask`.

### Main-thread dispatcher evidence

Earlier live metadata inspection of this client observed exact callable surfaces:

`FGStudio.Engine.Utilities.MainThread`
- static `get_Instance()`
- instance `Execute(System.Action)`
- backing queue field observed as `System.Collections.Concurrent.ConcurrentQueue<System.Action> waitToBeProcess`

`UnityMainThreadDispatcher`
- static `get_Instance()`
- instance `Enqueue(System.Action)`
- static `Dispatch(System.Action)`
- queue field observed as `System.Collections.Generic.Queue<System.Action> _queue`

Probe v3 therefore resolves these exact signatures, not just raw metadata-name strings. Capability bits:

- `CapMainThreadType`, `CapMainThreadInstanceGetter`, `CapMainThreadExecuteAction` -> `kMainThreadDispatcherMetadataSupportMask`.
- `CapUnityDispatcherType`, `CapUnityDispatcherInstanceGetter`, `CapUnityDispatcherEnqueueAction`, `CapUnityDispatcherDispatchAction` -> `kUnityDispatcherMetadataSupportMask`.

This still proves only method surface presence. It does **not** prove safe construction of a managed `System.Action`, queue delivery, callback execution, or gameplay mutation.

## Next gate: harmless dispatcher proof

Before Revive/NPC/Combat/Sell, NewCore must prove one harmless managed action can be serialized through ActionQueue and executed via one resolved dispatcher path on Unity main thread. The proof must have an observable non-gameplay POST and fail closed if delegate construction, dispatcher identity or callback completion is uncertain.

## Mutation boundary

`kGameplayMutationEnabled=false` remains compile-time. BridgeCommand still ends at `ReadGameSnapshot=5`. Dynamic Lua actions remain donor-only until the later runtime action proof.
