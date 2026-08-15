# v1.1.0 - Integrated Acceptance

- Gộp FSM + SafetyGuard + ActionQueue(MAX=1) + Watchdog + postcondition + DispatcherGate vào control-plane framework.
- `ControlSelfTest.exe` chạy trong CI trước bridge/controller; controller tự chạy lại lúc khởi động.
- Gameplay mutation vẫn compile-time FALSE.
- Protocol bump nội bộ lên `0x00010102` để thêm `InfrastructureProofSnapshot`.
- `ReadGameSnapshot=5` giữ nguyên; thêm `ProveHookActionEnvelope=6` **infrastructure-only**, không phải gameplay command.
- Harmless proof đi qua SafetyGuard -> ActionQueue queued/active -> proven WH_GETMESSAGE hook/main-thread callback -> token/sequence/thread POST -> CompleteActive.
- Bridge proof chỉ tăng counter nội bộ và echo evidence; không gọi UI/NPC/combat/path/inventory/shop method.
- Build audit cấm `ClickNPC`, `HandleClickEvent`, `StartAutoFight`, `RequestSellItem`, `StartPath`, `StopPath` và direct `runtime_invoke` trong harmless envelope.
- Integrated control self-test tăng từ 39 lên **48 checks**, thêm infrastructure gate và negative POST tests.
- Current-client Action Capability Resolver metadata-only xác minh UI/NPC/inventory/classification surface.
- Probe v2 thêm optional Assembly-CSharp class enumeration để tìm `MonoBehaviourExecutor` không đoán namespace.
- Probe v3 mã hóa đúng dispatcher signature từng runtime-observe:
  - `FGStudio.Engine.Utilities.MainThread.get_Instance()` + `Execute(System.Action)`;
  - `UnityMainThreadDispatcher.get_Instance()` + `Enqueue(System.Action)` + static `Dispatch(System.Action)`.
- Harmless proof không cần tạo `System.Action`; chính hook callback đã được foundation chứng minh là Unity main thread.
- `AutoFight_Main.StartAutoFight` và `NPCShop_SellItemTab.RequestSellItem` tiếp tục là Legacy v0.9.0 donor evidence, chưa được invoke.
- Runtime policy vẫn giữ một integrated acceptance tổng thay vì test từng phase nhỏ.
