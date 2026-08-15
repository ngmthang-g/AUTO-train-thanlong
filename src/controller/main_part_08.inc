    static void AddChange(std::wstring& out, const wchar_t* name, std::int32_t before, std::int32_t after) {
        if (!out.empty()) out += L" • ";
        out += name;
        out += L"=";
        out += std::to_wstring(before);
        out += L"→";
        out += std::to_wstring(after);
    }

    void EmitStateEdges(const GameSnapshot& before, const GameSnapshot& after) {
        const std::uint32_t coverageBefore = edgeCoverageMask_;
        std::wstring changes;
        if (HasBoth(before, after, ValidMapId) && before.mapID != after.mapID)
            AddChange(changes, L"map", before.mapID, after.mapID);
        if (HasBoth(before, after, ValidLifeState) && before.dead != after.dead) {
            edgeCoverageMask_ |= EdgeDead;
            AddChange(changes, L"dead", before.dead, after.dead);
        }
        if (HasBoth(before, after, ValidRideState) && before.riding != after.riding) {
            edgeCoverageMask_ |= EdgeRiding;
            AddChange(changes, L"riding", before.riding, after.riding);
        }
        if (HasBoth(before, after, ValidMovingState) && before.moving != after.moving) {
            edgeCoverageMask_ |= EdgeMoving;
            AddChange(changes, L"moving", before.moving, after.moving);
        }
        if (HasBoth(before, after, ValidAutoFightState) && before.autoFight != after.autoFight)
            AddChange(changes, L"auto", before.autoFight, after.autoFight);
        if (HasBoth(before, after, ValidAutoPathState) && before.autoPathing != after.autoPathing) {
            edgeCoverageMask_ |= EdgeAutoPath;
            AddChange(changes, L"autoPath", before.autoPathing, after.autoPathing);
        }
        if (HasBoth(before, after, ValidMapReadyState) && before.mapReady != after.mapReady)
            AddChange(changes, L"mapReady", before.mapReady, after.mapReady);
        if (HasBoth(before, after, ValidMapTransitionState) && before.waitingChangeMap != after.waitingChangeMap)
            AddChange(changes, L"waitingMap", before.waitingChangeMap, after.waitingChangeMap);
        if (!changes.empty()) events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": STATE EDGE • " + changes);
        EmitCoverageIfChanged(coverageBefore);
    }

    static std::wstring MissingObserverFields(const GameSnapshot& game) {
        std::wstring missing;
        auto add = [&](std::uint32_t mask, const wchar_t* name) {
            if (game.validMask & mask) return;
            if (!missing.empty()) missing += L", ";
            missing += name;
        };
        add(ValidPosition, L"position");
