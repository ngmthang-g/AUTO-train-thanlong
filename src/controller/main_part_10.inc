                Enter(FoundationState::WaitingStableMap,
                      L"MAP TRANSITION: chỉ scan MapReady/WaitingChangeMap; bỏ qua Leader/AutoPath theo donor guard");
            }
            nextSnapshotDue_ = now + std::chrono::milliseconds(500);
            return;
        }

        if ((gameSnapshot_.validMask & kRequiredGameCoreMask) != kRequiredGameCoreMask) {
            Fail(L"Snapshot stable không đủ core valid mask; fail-closed");
            return;
        }

        ObserveAutoPathProbe(gameSnapshot_);

        if (!transitionLatched_ && identityChanged) {
            qualificationStableScans_ = 0;
            const std::uint32_t coverageBefore = edgeCoverageMask_;
            edgeCoverageMask_ |= EdgeMapTransition;
            ++mapTransitionCount_;
            transitionLatched_ = true;
            transitionStartedByIdentity_ = true;
            transitionFlagJoinedLogged_ = false;
            recoveryCandidateValid_ = true;
            recoveryCandidateRoleID_ = gameSnapshot_.roleID;
            recoveryCandidateMapID_ = gameSnapshot_.mapID;
            postTransitionStableScans_ = 1;
            state_ = FoundationState::WaitingStableMap;
            actionQueue_.Clear();
            lastGuardBlockReason_.clear();
            SetControlFsmState(ControlFsmState::MapTransition,
                               L"source=identity; queue cleared; fail-closed");
            EmitCoverageIfChanged(coverageBefore);
            events_.push_back(
                L"PID " + std::to_wstring(game_.pid) +
                L": MAP TRANSITION • source=identity • flags missed/delayed • role=" +
                std::to_wstring(previousGameSnapshot_.roleID) + L"/map=" +
                std::to_wstring(previousGameSnapshot_.mapID) + L" → role=" +
                std::to_wstring(gameSnapshot_.roleID) + L"/map=" +
                std::to_wstring(gameSnapshot_.mapID) + L" • action LOCKED");
            detail_ = L"MAP RECOVERY 1/2 • source=identity • candidate role=" +
                      std::to_wstring(recoveryCandidateRoleID_) +
                      L" map=" + std::to_wstring(recoveryCandidateMapID_) +
                      L"; cần snapshot thứ hai cùng identity";
            events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
            nextSnapshotDue_ = now + std::chrono::milliseconds(500);
            return;
        }

        if (transitionLatched_) {
            if (!recoveryCandidateValid_) {
                recoveryCandidateValid_ = true;
                recoveryCandidateRoleID_ = gameSnapshot_.roleID;
                recoveryCandidateMapID_ = gameSnapshot_.mapID;
                postTransitionStableScans_ = 1;
                state_ = FoundationState::WaitingStableMap;
                detail_ = L"MAP RECOVERY 1/2 • source=" +
                          std::wstring(transitionStartedByIdentity_ ? L"identity" : L"flags") +
                          L" • candidate role=" + std::to_wstring(recoveryCandidateRoleID_) +
                          L" map=" + std::to_wstring(recoveryCandidateMapID_) +
                          L"; cần snapshot thứ hai cùng identity";
                events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
                nextSnapshotDue_ = now + std::chrono::milliseconds(500);
                return;
            }

            if (gameSnapshot_.roleID != recoveryCandidateRoleID_ ||
                gameSnapshot_.mapID != recoveryCandidateMapID_) {
                events_.push_back(
                    L"PID " + std::to_wstring(game_.pid) +
                    L": MAP RECOVERY RESET • source=" +
                    std::wstring(transitionStartedByIdentity_ ? L"identity" : L"flags") +
                    L" • identity đổi role=" +
                    std::to_wstring(recoveryCandidateRoleID_) + L"/map=" +
                    std::to_wstring(recoveryCandidateMapID_) + L" → role=" +
                    std::to_wstring(gameSnapshot_.roleID) + L"/map=" +
                    std::to_wstring(gameSnapshot_.mapID) +
                    L"; bắt đầu lại 1/2, action vẫn KHÓA");
                recoveryCandidateRoleID_ = gameSnapshot_.roleID;
                recoveryCandidateMapID_ = gameSnapshot_.mapID;
                postTransitionStableScans_ = 1;
                qualificationStableScans_ = 0;
                state_ = FoundationState::WaitingStableMap;
                nextSnapshotDue_ = now + std::chrono::milliseconds(500);
                return;
            }

            postTransitionStableScans_ = 2;
            const std::wstring recoveredSource = transitionStartedByIdentity_ ? L"identity" : L"flags";
            transitionLatched_ = false;
            transitionStartedByIdentity_ = false;
            transitionFlagJoinedLogged_ = false;
            recoveryCandidateValid_ = false;
            postTransitionStableScans_ = 0;
            events_.push_back(
                L"PID " + std::to_wstring(game_.pid) +
                L": MAP RECOVERY PASS 2/2 • source=" + recoveredSource +
                L" • same identity role=" + std::to_wstring(gameSnapshot_.roleID) +
                L" map=" + std::to_wstring(gameSnapshot_.mapID) +
                L" • state ổn định trở lại");
        }

        const std::wstring missing = MissingObserverFields(gameSnapshot_);
        if ((gameSnapshot_.validMask & kRequiredObserverStableMask) == kRequiredObserverStableMask) {
            if (qualificationStableScans_ < kQualificationStableScans) ++qualificationStableScans_;
            if (qualificationStableScans_ < kQualificationStableScans) {
                state_ = FoundationState::QualifyingScanner;
                detail_ = L"SCANNER QUALIFYING " + std::to_wstring(qualificationStableScans_) +
                          L"/" + std::to_wstring(kQualificationStableScans) +
                          L" stable • good=" + std::to_wstring(totalGoodSnapshots_) +
                          L" recovery=" + std::to_wstring(scannerRecoveries_) +
                          L" mapTransitions=" + std::to_wstring(mapTransitionCount_) +
                          L" • action LOCKED";
                if (qualificationStableScans_ == 1 || qualificationStableScans_ == 10 ||
                    qualificationStableScans_ == 30) {
                    events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + detail_);
                }
            } else if (state_ != FoundationState::Observing) {
                Enter(FoundationState::Observing,
                      L"SCANNER CORE QUALIFIED 60/60 (~30s) • runtime edge pending=" +
