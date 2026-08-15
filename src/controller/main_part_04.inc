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
            Send(BridgeCommand::ProveUnityMainThread, L"ProveUnityMainThread");
            return;
        }
        if (completed == BridgeCommand::ProveUnityMainThread) {
            if ((snapshot_.validMask & ValidUnityMainThread) == 0) {
                Fail(L"MainThread proof không đặt ValidUnityMainThread; fail-closed");
                return;
            }
            Enter(FoundationState::ReadingSnapshot,
                  L"MAINTHREAD PROVEN; đọc snapshot đầu rồi chuyển sang continuous scanner read-only");
            Send(BridgeCommand::ReadGameSnapshot, L"ReadGameSnapshot");
            return;
        }
    }

    bool PopEvent(std::wstring& out) {
        if (events_.empty()) return false;
        out = std::move(events_.front());
        events_.pop_front();
        return true;
    }

private:
    static bool IsScannerState(FoundationState state) {
        return state == FoundationState::WaitingStableMap ||
               state == FoundationState::ObservingPartial ||
               state == FoundationState::Observing ||
               state == FoundationState::RecoveringScanner;
    }

    static bool HasBoth(const GameSnapshot& a, const GameSnapshot& b, std::uint32_t mask) {
        return (a.validMask & mask) == mask && (b.validMask & mask) == mask;
    }

    static void AddChange(std::wstring& out, const wchar_t* name, std::int32_t before, std::int32_t after) {
        if (!out.empty()) out += L" • ";
        out += name;
        out += L"=";
        out += std::to_wstring(before);
        out += L"→";
        out += std::to_wstring(after);
    }

    void EmitStateEdges(const GameSnapshot& before, const GameSnapshot& after) {
        std::wstring changes;
        if (HasBoth(before, after, ValidMapId) && before.mapID != after.mapID)
            AddChange(changes, L"map", before.mapID, after.mapID);
        if (HasBoth(before, after, ValidLifeState) && before.dead != after.dead)
            AddChange(changes, L"dead", before.dead, after.dead);
        if (HasBoth(before, after, ValidRideState) && before.riding != after.riding)
            AddChange(changes, L"riding", before.riding, after.riding);
        if (HasBoth(before, after, ValidMovingState) && before.moving != after.moving)
            AddChange(changes, L"moving", before.moving, after.moving);
        if (HasBoth(before, after, ValidAutoFightState) && before.autoFight != after.autoFight)
            AddChange(changes, L"auto", before.autoFight, after.autoFight);
        if (HasBoth(before, after, ValidAutoPathState) && before.autoPathing != after.autoPathing)
            AddChange(changes, L"autoPath", before.autoPathing, after.autoPathing);
        if (HasBoth(before, after, ValidMapReadyState) && before.mapReady != after.mapReady)
            AddChange(changes, L"mapReady", before.mapReady, after.mapReady);
        if (HasBoth(before, after, ValidMapTransitionState) && before.waitingChangeMap != after.waitingChangeMap)
            AddChange(changes, L"waitingMap", before.waitingChangeMap, after.waitingChangeMap);
        if (!changes.empty()) events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": STATE EDGE • " + changes);
    }

    static std::wstring MissingObserverFields(const GameSnapshot& game) {
        std::wstring missing;
        auto add = [&](std::uint32_t mask, const wchar_t* name) {
            if (game.validMask & mask) return;
            if (!missing.empty()) missing += L", ";
            missing += name;
        };
        add(ValidPosition, L"position");
        add(ValidMovingState, L"moving");
        add(ValidAutoPathState, L"autoPath");
        return missing;
    }

    void StoreSnapshot(const GameSnapshot& next) {
        if (haveGameSnapshot_) {
            previousGameSnapshot_ = gameSnapshot_;
            EmitStateEdges(previousGameSnapshot_, next);
        }
        gameSnapshot_ = next;
        haveGameSnapshot_ = true;
    }

    void HandleSnapshotResponse(const BridgeResponse& response, Clock::time_point now) {
        const bool firstSnapshot = state_ == FoundationState::ReadingSnapshot;
