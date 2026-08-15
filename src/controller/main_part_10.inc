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

        if (transitionLatched_) {
            ++postTransitionStableScans_;
            if (postTransitionStableScans_ < 2) {
                state_ = FoundationState::WaitingStableMap;
                detail_ = L"MAP RECOVERY 1/2: đã thấy map stable một lần; chờ scan stable thứ hai theo donor guard";
                nextSnapshotDue_ = now + std::chrono::milliseconds(500);
                return;
            }
            transitionLatched_ = false;
            postTransitionStableScans_ = 0;
            events_.push_back(L"PID " + std::to_wstring(game_.pid) +
                              L": MAP RECOVERY PASS 2/2 • state ổn định trở lại");
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
