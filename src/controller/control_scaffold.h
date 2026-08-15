#pragma once
#include <cstddef>
#include <cstdint>

namespace tlcontrol {

inline constexpr bool kGameplayMutationEnabled = false;
static_assert(!kGameplayMutationEnabled,
              "v1.0.12 is a dry-run scaffold; gameplay mutation must stay disabled");

enum class ControlFsmState : std::uint32_t {
    SafePaused = 0,
    ScannerQualifying,
    IdleStable,
    DeadDetected,
    MapTransition,
    Faulted,
};

inline const wchar_t* ControlFsmStateName(ControlFsmState state) {
    switch (state) {
        case ControlFsmState::SafePaused: return L"SAFE_PAUSED";
        case ControlFsmState::ScannerQualifying: return L"SCANNER_QUALIFYING";
        case ControlFsmState::IdleStable: return L"IDLE_STABLE";
        case ControlFsmState::DeadDetected: return L"DEAD_DETECTED";
        case ControlFsmState::MapTransition: return L"MAP_TRANSITION";
        case ControlFsmState::Faulted: return L"FAULTED";
    }
    return L"?";
}

enum class ActionIntentKind : std::uint32_t {
    None = 0,
    Revive,
};

inline const wchar_t* ActionIntentName(ActionIntentKind kind) {
    switch (kind) {
        case ActionIntentKind::None: return L"None";
        case ActionIntentKind::Revive: return L"Revive";
    }
    return L"?";
}

struct ActionIntent {
    ActionIntentKind kind = ActionIntentKind::None;
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    std::uint32_t snapshotSequence = 0;
};

struct SafetyGuardContext {
    bool bridgeAttached = false;
    bool mainThreadProven = false;
    bool scannerQualified = false;
    bool observerStable = false;
    bool transitionActive = false;
    bool scannerHealthy = false;
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    bool dead = false;
};

struct SafetyGuardDecision {
    bool preconditionsPass = false;
    const wchar_t* reason = L"unknown";
};

class SafetyGuard {
public:
    static SafetyGuardDecision Evaluate(const ActionIntent& intent,
                                        const SafetyGuardContext& ctx) {
        if (intent.kind == ActionIntentKind::None) return {false, L"intent-none"};
        if (!ctx.bridgeAttached) return {false, L"bridge-detached"};
        if (!ctx.mainThreadProven) return {false, L"main-thread-unproven"};
        if (!ctx.scannerHealthy) return {false, L"scanner-unhealthy"};
        if (ctx.transitionActive) return {false, L"map-transition-active"};
        if (!ctx.observerStable) return {false, L"observer-partial"};
        if (!ctx.scannerQualified) return {false, L"scanner-unqualified"};
        if (ctx.roleID != intent.roleID || ctx.mapID != intent.mapID)
            return {false, L"identity-drift"};
        if (intent.kind == ActionIntentKind::Revive && !ctx.dead)
            return {false, L"precondition-dead-false"};
        return {true, L"preconditions-pass"};
    }
};

class ActionQueueScaffold {
public:
    static constexpr std::size_t kCapacity = 1;

    bool TryEnqueue(const ActionIntent& intent) {
        if (occupied_) return false;
        slot_ = intent;
        occupied_ = true;
        return true;
    }

    bool Pop(ActionIntent& out) {
        if (!occupied_) return false;
        out = slot_;
        slot_ = {};
        occupied_ = false;
        return true;
    }

    void Clear() {
        slot_ = {};
        occupied_ = false;
    }

    std::size_t Depth() const { return occupied_ ? 1u : 0u; }
    std::size_t Active() const { return 0u; }

private:
    ActionIntent slot_{};
    bool occupied_ = false;
};

static_assert(ActionQueueScaffold::kCapacity == 1,
              "NewCore invariant: ActionQueue capacity must remain exactly one");

} // namespace tlcontrol
