                      PendingRuntimeEdges() + L" • action game vẫn KHÓA");
            }
        } else {
            qualificationStableScans_ = 0;
            if (state_ != FoundationState::ObservingPartial) {
                Enter(FoundationState::ObservingPartial,
                      L"SCANNER PARTIAL: thiếu " + missing + L"; qualification reset; action tiếp tục KHÓA");
            }
        }
        UpdateControlScaffold();
        MaybeRunReviveLifecycle(now);
        nextSnapshotDue_ = now + std::chrono::milliseconds(500);
    }

    bool Send(BridgeCommand command, const wchar_t* label, bool logEvent = true,
              std::uint64_t u64Arg0 = 0,
              std::int32_t intArg0 = 0,
              std::int32_t intArg1 = 0) {
        if (pending_ != BridgeCommand::None) {
            Fail(L"Invariant lỗi: có hơn 1 command đang chạy");
            return false;
        }
        std::wstring error;
        if (!bridge_.Begin(command, error, u64Arg0, intArg0, intArg1)) {
            Fail(std::wstring(label) + L": " + error);
            return false;
        }
        pending_ = command;
        if (logEvent) events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": gửi " + label);
        return true;
    }

    void AppendReport(const std::wstring& line) {
        if (!report_.empty()) report_ += L"\r\n";
        report_ += line;
        detail_ = line;
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + line);
    }

    void Enter(FoundationState next, const std::wstring& why) {
        state_ = next;
        detail_ = why;
        events_.push_back(L"PID " + std::to_wstring(game_.pid) + L": " + why);
    }

    void Fail(const std::wstring& why) {
        pending_ = BridgeCommand::None;
        actionQueue_.Clear();
        reviveDispatchIssued_ = false;
        reviveAwaitingPost_ = false;
        reviveActiveToken_ = 0;
        reviveAliveStableScans_ = 0;
        reviveDeadline_ = Clock::time_point{};
        g_harmlessProofTrackers.erase(game_.pid);
        bridge_.Close();
        SetControlFsmState(ControlFsmState::Faulted, L"fail-closed; queue cleared");
        Enter(FoundationState::Faulted, why);
    }

    GameProcess game_{};
    BridgeClient bridge_{};
    FoundationState state_ = FoundationState::Stopped;
    BridgeCommand pending_ = BridgeCommand::None;
    FoundationSnapshot snapshot_{};
    ActionCapabilitySnapshot actionCapabilities_{};
    GameSnapshot gameSnapshot_{};
    GameSnapshot previousGameSnapshot_{};
    bool haveGameSnapshot_ = false;
    int scannerFailures_ = 0;
    int scannerRecoveries_ = 0;
    int totalGoodSnapshots_ = 0;
    int qualificationStableScans_ = 0;
    std::uint32_t edgeCoverageMask_ = 0;
    std::uint32_t lastAutoPathProbeMask_ = 0;
    bool autoPathProbeLogged_ = false;
    bool autoPathObservedOnLogged_ = false;
    bool transitionLatched_ = false;
    bool transitionStartedByIdentity_ = false;
    bool transitionFlagJoinedLogged_ = false;
    bool recoveryCandidateValid_ = false;
    std::int32_t recoveryCandidateRoleID_ = 0;
    std::int32_t recoveryCandidateMapID_ = 0;
    int postTransitionStableScans_ = 0;
    int mapTransitionCount_ = 0;
    ControlFsmState controlState_ = ControlFsmState::SafePaused;
    ActionQueueScaffold actionQueue_{};

    // v1.2.0 first real gameplay action: Revive only.
    bool reviveDryRunIssued_ = true; // legacy dry-run suppressed; retained only for source compatibility.
    bool reviveDispatchIssued_ = false;
    bool reviveAwaitingPost_ = false;
    bool reviveBlockedThisDeath_ = false;
    bool reviveTransitionPreserveLogged_ = false;
    std::uint64_t reviveActiveToken_ = 0;
    std::uint64_t nextReviveToken_ = 1;
    std::int32_t reviveExpectedRoleID_ = 0;
    std::int32_t revivePreMapID_ = 0;
    std::uint32_t reviveExpectedHookThreadId_ = 0;
    std::int32_t reviveExpectedManagedThreadId_ = 0;
    std::int32_t reviveExpectedUnityMainThreadId_ = 0;
    int deadStableScans_ = 0;
    int reviveAliveStableScans_ = 0;
    Clock::time_point reviveDeadline_{};

    std::wstring lastGuardBlockReason_{};
    Clock::time_point nextSnapshotDue_{};
    std::wstring detail_ = L"Đã dừng";
    std::wstring report_{};
    std::deque<std::wstring> events_{};
};
HWND g_main = nullptr;
HWND g_list = nullptr;
HWND g_log = nullptr;
std::map<DWORD, std::unique_ptr<Session>> g_sessions;

void Log(const std::wstring& text) {
    if (!g_log) return;
