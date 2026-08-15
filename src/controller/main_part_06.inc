        snapshot_.validMask |= response.snapshot.validMask;
        if (response.snapshot.hookThreadId) snapshot_.hookThreadId = response.snapshot.hookThreadId;
        if (response.snapshot.windowThreadId) snapshot_.windowThreadId = response.snapshot.windowThreadId;
        if (response.snapshot.resolvedExports) snapshot_.resolvedExports = response.snapshot.resolvedExports;
        if (response.snapshot.requiredExports) snapshot_.requiredExports = response.snapshot.requiredExports;
        if (response.snapshot.fgMainThreadMethodCount) snapshot_.fgMainThreadMethodCount = response.snapshot.fgMainThreadMethodCount;
        if (response.snapshot.fgMainThreadFieldCount) snapshot_.fgMainThreadFieldCount = response.snapshot.fgMainThreadFieldCount;
        if (response.snapshot.unityDispatcherMethodCount) snapshot_.unityDispatcherMethodCount = response.snapshot.unityDispatcherMethodCount;
        if (response.snapshot.unityDispatcherFieldCount) snapshot_.unityDispatcherFieldCount = response.snapshot.unityDispatcherFieldCount;
        if (response.snapshot.currentManagedThreadId) snapshot_.currentManagedThreadId = response.snapshot.currentManagedThreadId;
        if (response.snapshot.unityMainManagedThreadId) snapshot_.unityMainManagedThreadId = response.snapshot.unityMainManagedThreadId;

        if (completed == BridgeCommand::ReadGameSnapshot) {
            HandleSnapshotResponse(response, now);
            return;
        }

        AppendReport(response.detail);
        if (!response.ok) {
            Fail(L"Bridge validator lỗi " + std::to_wstring(response.errorCode) + L": " + std::wstring(response.detail));
            return;
        }

        if (completed == BridgeCommand::ValidateNative) {
            Enter(FoundationState::InspectingFgMainThread,
                  L"Native PASS; inspect metadata FGStudio.Engine.Utilities.MainThread");
            Send(BridgeCommand::InspectFgMainThread, L"InspectFgMainThread");
            return;
        }
        if (completed == BridgeCommand::InspectFgMainThread) {
            Enter(FoundationState::InspectingUnityDispatcher,
                  L"FG MainThread đã đọc metadata; inspect UnityMainThreadDispatcher");
            Send(BridgeCommand::InspectUnityDispatcher, L"InspectUnityDispatcher");
            return;
        }
        if (completed == BridgeCommand::InspectUnityDispatcher) {
            Enter(FoundationState::ProvingUnityMainThread,
                  L"Metadata PASS; chạy main-thread proof chỉ bằng managed getter read-only");
