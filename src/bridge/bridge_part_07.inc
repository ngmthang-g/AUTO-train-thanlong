    game.dead = dead ? 1 : 0; game.validMask |= ValidLifeState;
    if (!ReadScalarGetter(leaderClass, "get_IsRiding", leader, riding, detail, cap)) return false;
    game.riding = riding ? 1 : 0; game.validMask |= ValidRideState;

    const MethodInfo* getName = FindMethodInHierarchy(leaderClass, "get_Name", 0);
    if (getName) {
        Il2CppObject* nameObject = nullptr;
        if (InvokeObject(getName, leader, nameObject, detail, cap) && nameObject &&
            CopyManagedString(reinterpret_cast<Il2CppString*>(nameObject), game.characterName, _countof(game.characterName))) {
            game.validMask |= ValidCharacterName;
        }
    }

    // Position: metadata/getter only. Donor +0x50/+0x54/+0x58 remains documentation, never fallback.
    std::int32_t x = 0, y = 0;
    bool positionOk = ReadScalarGetter(leaderClass, "get_PosX", leader, x, detail, cap) &&
                      ReadScalarGetter(leaderClass, "get_PosY", leader, y, detail, cap);
    if (!positionOk) {
        FieldInfo* roleField = FindFieldInHierarchy(leaderClass, "roleData");
        Il2CppObject* roleBacking = nullptr;
        if (roleField) {
            const Il2CppType* roleType = g_api.field_get_type(roleField);
            Il2CppClass* roleFieldClass = roleType ? g_api.class_from_type(roleType) : nullptr;
            if (roleFieldClass && !g_api.class_is_valuetype(roleFieldClass)) {
                g_api.field_get_value(leader, roleField, &roleBacking);
            }
        }
        if (roleBacking) {
            Il2CppClass* backingClass = g_api.object_get_class(roleBacking);
            if (backingClass) {
                positionOk = ReadScalarGetter(backingClass, "get_PosX", roleBacking, x, detail, cap) &&
                             ReadScalarGetter(backingClass, "get_PosY", roleBacking, y, detail, cap);
            }
        }
    }
    if (positionOk) {
        game.x = x; game.y = y; game.validMask |= ValidPosition;
    }

    std::int32_t enableAutoF1 = 0, freeBag = -1, moving = 0;
    if (!ReadStaticScalar(gameApi, "get_EnableAutoF1", enableAutoF1, detail, cap)) return false;
    game.autoFight = enableAutoF1 ? 0 : 1; game.validMask |= ValidAutoFightState;
    if (!ReadStaticScalar(gameApi, "GetFreeBagSpace", freeBag, detail, cap) || freeBag < 0 || freeBag > 10000) return false;
    game.freeBagSpace = freeBag; game.validMask |= ValidBagSpace;
    if (ReadStaticScalar(gameApi, "IsMoving", moving, detail, cap)) {
        game.moving = moving ? 1 : 0; game.validMask |= ValidMovingState;
    }

    // v0.9.0 read AutoPathManager.IsRunning. NewCore resolves the equivalent metadata property
    // FGStudio.Engine.Logic.AutoPathManager.get_IsAutoPathing on its singleton instance.
    // v1.0.10 also records a read-only resolver/probe mask so runtime logs can distinguish
    // "getter resolved but value stayed 0" from "AutoPath resolution failed".
    Il2CppClass* autoPathClass = g_api.class_from_name(image, "FGStudio.Engine.Logic", "AutoPathManager");
    if (autoPathClass) {
        game.autoPathProbeMask |= AutoPathClassResolved;
        const MethodInfo* getInstance = FindMethodInHierarchy(autoPathClass, "get_Instance", 0);
        if (getInstance && IsStaticMethod(getInstance)) {
            game.autoPathProbeMask |= AutoPathInstanceGetterResolved;
            Il2CppObject* autoPath = nullptr;
            if (InvokeObject(getInstance, nullptr, autoPath, detail, cap) && autoPath) {
                game.autoPathProbeMask |= AutoPathInstanceResolved;
                Il2CppClass* actualAutoPathClass = g_api.object_get_class(autoPath);
                if (actualAutoPathClass) {
                    const MethodInfo* getIsAutoPathing =
                        FindMethodInHierarchy(actualAutoPathClass, "get_IsAutoPathing", 0);
                    if (getIsAutoPathing && !IsStaticMethod(getIsAutoPathing)) {
                        game.autoPathProbeMask |= AutoPathStateGetterResolved;
                        std::int32_t pathing = 0;
                        if (ReadScalarGetter(actualAutoPathClass, "get_IsAutoPathing",
                                             autoPath, pathing, detail, cap)) {
                            game.autoPathProbeMask |= AutoPathValueRead;
                            game.autoPathing = pathing ? 1 : 0;
                            game.validMask |= ValidAutoPathState;
                        }
                    }
                }
            }
        }
    }

    if ((game.validMask & kRequiredGameCoreMask) != kRequiredGameCoreMask) {
        CopyText(detail, cap, L"Snapshot thiếu core field bắt buộc; fail-closed");
        return false;
    }

    ClearText(detail, cap);
    AppendText(detail, cap, L"SNAPSHOT PASS seq="); AppendUInt(detail, cap, game.sequence);
    AppendText(detail, cap, L" role="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.roleID));
    AppendText(detail, cap, L" map="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.mapID));
    if (game.validMask & ValidPosition) {
        AppendText(detail, cap, L" pos="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.x));
        AppendText(detail, cap, L","); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.y));
    } else {
        AppendText(detail, cap, L" pos=?");
    }
    AppendText(detail, cap, L" HP="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.hp));
    AppendText(detail, cap, L"/"); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.maxHP));
    AppendText(detail, cap, L" bag="); AppendUInt(detail, cap, static_cast<std::uint32_t>(game.freeBagSpace));
