                      PendingRuntimeEdges() + L" • action game vẫn KHÓA");
            }
        } else {
            qualificationStableScans_ = 0;
            if (state_ != FoundationState::ObservingPartial) {
                Enter(FoundationState::ObservingPartial,
                      L"SCANNER PARTIAL: thiếu " + missing + L"; qualification reset; action tiếp tục KHÓA");
            }
        }
        nextSnapshotDue_ = now + std::chrono::milliseconds(500);
    }

    bool Send(BridgeCommand command, const wchar_t* label, bool logEvent = true) {
        if (pending_ != BridgeCommand::None) {
            Fail(L"Invariant lỗi: có hơn 1 command đang chạy");
            return false;
        }
        std::wstring error;
        if (!bridge_.Begin(command, error)) {
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
        bridge_.Close();
        Enter(FoundationState::Faulted, why);
    }

    GameProcess game_{};
    BridgeClient bridge_{};
    FoundationState state_ = FoundationState::Stopped;
    BridgeCommand pending_ = BridgeCommand::None;
    FoundationSnapshot snapshot_{};
    GameSnapshot gameSnapshot_{};
    GameSnapshot previousGameSnapshot_{};
    bool haveGameSnapshot_ = false;
    int scannerFailures_ = 0;
    int scannerRecoveries_ = 0;
    int totalGoodSnapshots_ = 0;
    int qualificationStableScans_ = 0;
    std::uint32_t edgeCoverageMask_ = 0;
    bool transitionLatched_ = false;
    int postTransitionStableScans_ = 0;
    int mapTransitionCount_ = 0;
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
