            error = L"Bridge không export TlGetMessageHook";
            Close();
            return false;
        }
        hook_ = SetWindowsHookExW(WH_GETMESSAGE, proc, localBridge_, game.threadId);
        if (!hook_) {
            error = L"SetWindowsHookEx thất bại; thử chạy tool cùng quyền với game";
            Close();
            return false;
        }
        if (!PostThreadMessageW(game.threadId, kWakeMessage, 0, 0)) {
            error = L"Không đánh thức được message thread game";
            Close();
            return false;
        }
        attached_ = true;
        return true;
    }

    void Close() {
        if (hook_) UnhookWindowsHookEx(hook_);
        hook_ = nullptr;
        if (localBridge_) FreeLibrary(localBridge_);
        localBridge_ = nullptr;
        if (shared_) UnmapViewOfFile(shared_);
        shared_ = nullptr;
        if (mapping_) CloseHandle(mapping_);
        mapping_ = nullptr;
        attached_ = false;
        outstandingSeq_ = 0;
    }

    bool Attached() const { return attached_ && hook_ && shared_; }
    bool Busy() const { return outstandingSeq_ != 0; }
    bool LoadedInTarget() const { return shared_ && shared_->bridgeLoaded != 0; }

    bool Begin(BridgeCommand command, std::wstring& error,
               std::uint64_t u64Arg0 = 0,
               std::int32_t intArg0 = 0,
               std::int32_t intArg1 = 0) {
        if (!Attached()) {
            error = L"Bridge chưa attach";
            return false;
        }
        if (Busy()) {
            error = L"Đang có probe khác; không chạy song song";
            return false;
        }
        const LONG next = shared_->requestSeq + 1;
        shared_->request = {};
        shared_->request.command = static_cast<std::uint32_t>(command);
        shared_->request.u64Arg0 = u64Arg0;
        shared_->request.intArg0 = intArg0;
        shared_->request.intArg1 = intArg1;
        MemoryBarrier();
        InterlockedExchange(&shared_->requestSeq, next);
        outstandingSeq_ = next;
        sentAt_ = Clock::now();
        if (!PostThreadMessageW(game_.threadId, kWakeMessage, 0, 0)) {
            outstandingSeq_ = 0;
            error = L"PostThreadMessage thất bại";
            return false;
        }
        return true;
    }

    enum class PollResult { Idle, Pending, Complete, Timeout };

    PollResult Poll(BridgeResponse& out) {
        if (!Busy()) return PollResult::Idle;
        if (shared_->completedSeq == outstandingSeq_) {
            MemoryBarrier();
            out = shared_->response;
            outstandingSeq_ = 0;
            return PollResult::Complete;
        }
        if (Clock::now() - sentAt_ > std::chrono::seconds(5)) {
            outstandingSeq_ = 0;
            return PollResult::Timeout;
        }
        return PollResult::Pending;
    }

