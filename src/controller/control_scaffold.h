#pragma once
#include <cstddef>
#include <cstdint>

namespace tlcontrol {

inline constexpr bool kGameplayMutationEnabled = false;
inline constexpr bool kHarmlessInfrastructureProofEnabled = true;
static_assert(!kGameplayMutationEnabled,
              "v1.1.0 integrated acceptance keeps gameplay mutation compile-time disabled");
static_assert(kHarmlessInfrastructureProofEnabled,
              "harmless infrastructure proof must stay explicitly enabled for acceptance");

enum class ControlFsmState : std::uint32_t {
    SafePaused = 0,
    ScannerQualifying,
    IdleStable,
    DeadDetected,
    MapTransition,
    ActionPrepared,
    AwaitingPostcondition,
    Faulted,
};

inline const wchar_t* ControlFsmStateName(ControlFsmState state) {
    switch (state) {
        case ControlFsmState::SafePaused: return L"SAFE_PAUSED";
        case ControlFsmState::ScannerQualifying: return L"SCANNER_QUALIFYING";
        case ControlFsmState::IdleStable: return L"IDLE_STABLE";
        case ControlFsmState::DeadDetected: return L"DEAD_DETECTED";
        case ControlFsmState::MapTransition: return L"MAP_TRANSITION";
        case ControlFsmState::ActionPrepared: return L"ACTION_PREPARED";
        case ControlFsmState::AwaitingPostcondition: return L"AWAITING_POSTCONDITION";
        case ControlFsmState::Faulted: return L"FAULTED";
    }
    return L"?";
}

enum class ActionIntentKind : std::uint32_t {
    None = 0,
    HarmlessInfrastructureProof,
    Revive,
    EnableCombatAutomation,
    DisableCombatAutomation,
    MoveTo,
    InventoryCheck,
    NpcInteraction,
    Treatment,
    Sell,
    Buff,
};

inline const wchar_t* ActionIntentName(ActionIntentKind kind) {
    switch (kind) {
        case ActionIntentKind::None: return L"None";
        case ActionIntentKind::HarmlessInfrastructureProof: return L"HarmlessInfrastructureProof";
        case ActionIntentKind::Revive: return L"Revive";
        case ActionIntentKind::EnableCombatAutomation: return L"EnableCombatAutomation";
        case ActionIntentKind::DisableCombatAutomation: return L"DisableCombatAutomation";
        case ActionIntentKind::MoveTo: return L"MoveTo";
        case ActionIntentKind::InventoryCheck: return L"InventoryCheck";
        case ActionIntentKind::NpcInteraction: return L"NpcInteraction";
        case ActionIntentKind::Treatment: return L"Treatment";
        case ActionIntentKind::Sell: return L"Sell";
        case ActionIntentKind::Buff: return L"Buff";
    }
    return L"?";
}

struct ActionIntent {
    ActionIntentKind kind = ActionIntentKind::None;
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    std::uint32_t snapshotSequence = 0;
    std::uint64_t token = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct SafetyGuardContext {
    bool bridgeAttached = false;
    bool mainThreadProven = false;
    bool scannerQualified = false;
    bool observerStable = false;
    bool transitionActive = false;
    bool scannerHealthy = false;
    bool watchdogHealthy = true;
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
        if (!ctx.watchdogHealthy) return {false, L"watchdog-tripped"};
        if (ctx.transitionActive) return {false, L"map-transition-active"};
        if (!ctx.observerStable) return {false, L"observer-partial"};
        if (!ctx.scannerQualified) return {false, L"scanner-unqualified"};
        if (ctx.roleID != intent.roleID || ctx.mapID != intent.mapID)
            return {false, L"identity-drift"};
        if (intent.kind == ActionIntentKind::Revive && !ctx.dead)
            return {false, L"precondition-dead-false"};
        if (intent.kind != ActionIntentKind::Revive && ctx.dead)
            return {false, L"precondition-alive-false"};
        return {true, L"preconditions-pass"};
    }
};

class ActionQueueScaffold {
public:
    static constexpr std::size_t kCapacity = 1;

    bool TryEnqueue(const ActionIntent& intent) {
        if (queued_ || active_) return false;
        queuedIntent_ = intent;
        queued_ = true;
        return true;
    }

    bool BeginNext(ActionIntent& out) {
        if (!queued_ || active_) return false;
        activeIntent_ = queuedIntent_;
        queuedIntent_ = {};
        queued_ = false;
        active_ = true;
        out = activeIntent_;
        return true;
    }

    bool Pop(ActionIntent& out) {
        if (!queued_) return false;
        out = queuedIntent_;
        queuedIntent_ = {};
        queued_ = false;
        return true;
    }

    bool CompleteActive() {
        if (!active_) return false;
        activeIntent_ = {};
        active_ = false;
        return true;
    }

    void CancelAll() {
        queuedIntent_ = {};
        activeIntent_ = {};
        queued_ = false;
        active_ = false;
    }

    void Clear() { CancelAll(); }

    std::size_t Depth() const { return queued_ ? 1u : 0u; }
    std::size_t Active() const { return active_ ? 1u : 0u; }
    std::size_t Occupied() const { return Depth() + Active(); }

private:
    ActionIntent queuedIntent_{};
    ActionIntent activeIntent_{};
    bool queued_ = false;
    bool active_ = false;
};

static_assert(ActionQueueScaffold::kCapacity == 1,
              "NewCore invariant: ActionQueue capacity must remain exactly one");

struct ControlModelInput {
    bool faulted = false;
    bool snapshotPresent = false;
    bool scannerHealthy = false;
    bool observerStable = false;
    bool scannerQualified = false;
    bool transitionActive = false;
    bool dead = false;
};

inline ControlFsmState DeriveControlFsmState(const ControlModelInput& in) {
    if (in.faulted) return ControlFsmState::Faulted;
    if (in.transitionActive) return ControlFsmState::MapTransition;
    if (!in.snapshotPresent || !in.scannerHealthy || !in.observerStable)
        return ControlFsmState::SafePaused;
    if (in.dead) return ControlFsmState::DeadDetected;
    if (!in.scannerQualified) return ControlFsmState::ScannerQualifying;
    return ControlFsmState::IdleStable;
}

inline ActionIntentKind CandidateForState(ControlFsmState state) {
    return state == ControlFsmState::DeadDetected ? ActionIntentKind::Revive
                                                  : ActionIntentKind::None;
}

class ScannerWatchdog {
public:
    bool Observe(bool scannerOk) {
        if (scannerOk) {
            consecutiveFailures_ = 0;
            tripped_ = false;
            return false;
        }
        ++consecutiveFailures_;
        if (consecutiveFailures_ >= 3) tripped_ = true;
        return tripped_;
    }

    void Reset() {
        consecutiveFailures_ = 0;
        tripped_ = false;
    }

    int ConsecutiveFailures() const { return consecutiveFailures_; }
    bool Tripped() const { return tripped_; }

private:
    int consecutiveFailures_ = 0;
    bool tripped_ = false;
};

struct DispatchDecision {
    bool allowed = false;
    const wchar_t* reason = L"unknown";
};

inline bool ActionCapabilityResolved(ActionIntentKind) {
    return false;
}

class DispatcherGate {
public:
    static DispatchDecision Evaluate(ActionIntentKind kind) {
        if (kind == ActionIntentKind::None) return {false, L"intent-none"};
        if (!kGameplayMutationEnabled) return {false, L"mutation-disabled"};
        if (!ActionCapabilityResolved(kind)) return {false, L"capability-unresolved"};
        return {true, L"dispatch-allowed"};
    }
};

class InfrastructureDispatcherGate {
public:
    static DispatchDecision Evaluate(ActionIntentKind kind) {
        if (kind != ActionIntentKind::HarmlessInfrastructureProof)
            return {false, L"not-harmless-proof"};
        if (!kHarmlessInfrastructureProofEnabled)
            return {false, L"harmless-proof-disabled"};
        return {true, L"harmless-proof-allowed"};
    }
};

struct InfrastructureProofObserved {
    std::uint64_t token = 0;
    std::uint32_t sequence = 0;
    std::uint32_t callbackThreadId = 0;
    std::int32_t currentManagedThreadId = 0;
    std::int32_t unityMainManagedThreadId = 0;
    bool completed = false;
};

inline bool InfrastructureProofPostconditionSatisfied(
    std::uint64_t expectedToken,
    std::uint32_t previousSequence,
    std::uint32_t expectedHookThreadId,
    std::int32_t expectedManagedThreadId,
    std::int32_t expectedUnityMainThreadId,
    const InfrastructureProofObserved& observed) {
    return expectedToken != 0 && observed.completed &&
           observed.token == expectedToken &&
           observed.sequence > previousSequence &&
           observed.callbackThreadId == expectedHookThreadId &&
           observed.currentManagedThreadId == expectedManagedThreadId &&
           observed.unityMainManagedThreadId == expectedUnityMainThreadId &&
           observed.currentManagedThreadId == observed.unityMainManagedThreadId;
}

struct ObservedPostState {
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    bool dead = false;
    bool autoFight = false;
};

inline bool PostconditionSatisfied(ActionIntentKind kind,
                                   const ObservedPostState& before,
                                   const ObservedPostState& after) {
    if (before.roleID != after.roleID || before.mapID != after.mapID) return false;
    switch (kind) {
        case ActionIntentKind::Revive:
            return before.dead && !after.dead;
        case ActionIntentKind::EnableCombatAutomation:
            return !before.dead && !before.autoFight && after.autoFight;
        case ActionIntentKind::DisableCombatAutomation:
            return !before.dead && before.autoFight && !after.autoFight;
        case ActionIntentKind::None:
        case ActionIntentKind::HarmlessInfrastructureProof:
        case ActionIntentKind::MoveTo:
        case ActionIntentKind::InventoryCheck:
        case ActionIntentKind::NpcInteraction:
        case ActionIntentKind::Treatment:
        case ActionIntentKind::Sell:
        case ActionIntentKind::Buff:
            return false;
    }
    return false;
}

struct IntegratedSelfTestResult {
    int total = 0;
    int passed = 0;
    int failed = 0;

    bool Pass() const { return failed == 0 && total == passed; }
};

inline void SelfTestCheck(IntegratedSelfTestResult& r, bool condition) {
    ++r.total;
    if (condition) ++r.passed;
    else ++r.failed;
}

inline IntegratedSelfTestResult RunIntegratedControlSelfTest() {
    IntegratedSelfTestResult r{};

    SelfTestCheck(r, !kGameplayMutationEnabled);
    SelfTestCheck(r, kHarmlessInfrastructureProofEnabled);
    SelfTestCheck(r, ActionQueueScaffold::kCapacity == 1);

    ControlModelInput model{};
    SelfTestCheck(r, DeriveControlFsmState(model) == ControlFsmState::SafePaused);
    model.snapshotPresent = true;
    model.scannerHealthy = true;
    model.observerStable = true;
    SelfTestCheck(r, DeriveControlFsmState(model) == ControlFsmState::ScannerQualifying);
    model.scannerQualified = true;
    SelfTestCheck(r, DeriveControlFsmState(model) == ControlFsmState::IdleStable);
    model.dead = true;
    SelfTestCheck(r, DeriveControlFsmState(model) == ControlFsmState::DeadDetected);
    SelfTestCheck(r, CandidateForState(ControlFsmState::DeadDetected) == ActionIntentKind::Revive);
    model.transitionActive = true;
    SelfTestCheck(r, DeriveControlFsmState(model) == ControlFsmState::MapTransition);
    model.faulted = true;
    SelfTestCheck(r, DeriveControlFsmState(model) == ControlFsmState::Faulted);

    ActionIntent revive{};
    revive.kind = ActionIntentKind::Revive;
    revive.roleID = 100;
    revive.mapID = 24;
    revive.snapshotSequence = 77;

    SafetyGuardContext guard{};
    guard.bridgeAttached = true;
    guard.mainThreadProven = true;
    guard.scannerHealthy = true;
    guard.observerStable = true;
    guard.roleID = 100;
    guard.mapID = 24;
    guard.dead = true;
    SelfTestCheck(r, !SafetyGuard::Evaluate(revive, guard).preconditionsPass);
    guard.scannerQualified = true;
    SelfTestCheck(r, SafetyGuard::Evaluate(revive, guard).preconditionsPass);
    guard.mapID = 25;
    SelfTestCheck(r, !SafetyGuard::Evaluate(revive, guard).preconditionsPass);
    guard.mapID = 24;
    guard.transitionActive = true;
    SelfTestCheck(r, !SafetyGuard::Evaluate(revive, guard).preconditionsPass);
    guard.transitionActive = false;
    guard.watchdogHealthy = false;
    SelfTestCheck(r, !SafetyGuard::Evaluate(revive, guard).preconditionsPass);

    ActionQueueScaffold queue{};
    SelfTestCheck(r, queue.TryEnqueue(revive));
    SelfTestCheck(r, !queue.TryEnqueue(revive));
    SelfTestCheck(r, queue.Depth() == 1 && queue.Active() == 0 && queue.Occupied() == 1);
    ActionIntent active{};
    SelfTestCheck(r, queue.BeginNext(active));
    SelfTestCheck(r, queue.Depth() == 0 && queue.Active() == 1 && queue.Occupied() == 1);
    SelfTestCheck(r, !queue.TryEnqueue(revive));
    SelfTestCheck(r, queue.CompleteActive());
    SelfTestCheck(r, queue.Occupied() == 0);
    SelfTestCheck(r, queue.TryEnqueue(revive));
    queue.CancelAll();
    SelfTestCheck(r, queue.Occupied() == 0);

    ScannerWatchdog watchdog{};
    SelfTestCheck(r, !watchdog.Observe(false));
    SelfTestCheck(r, !watchdog.Observe(false));
    SelfTestCheck(r, watchdog.Observe(false));
    SelfTestCheck(r, watchdog.Tripped() && watchdog.ConsecutiveFailures() == 3);
    SelfTestCheck(r, !watchdog.Observe(true) && !watchdog.Tripped());

    SelfTestCheck(r, !DispatcherGate::Evaluate(ActionIntentKind::Revive).allowed);
    SelfTestCheck(r, !ActionCapabilityResolved(ActionIntentKind::Revive));
    SelfTestCheck(r, !ActionCapabilityResolved(ActionIntentKind::EnableCombatAutomation));
    SelfTestCheck(r, !ActionCapabilityResolved(ActionIntentKind::NpcInteraction));
    SelfTestCheck(r, !ActionCapabilityResolved(ActionIntentKind::Sell));

    SelfTestCheck(r, InfrastructureDispatcherGate::Evaluate(
                         ActionIntentKind::HarmlessInfrastructureProof).allowed);
    SelfTestCheck(r, !InfrastructureDispatcherGate::Evaluate(ActionIntentKind::Revive).allowed);
    SelfTestCheck(r, !DispatcherGate::Evaluate(
                          ActionIntentKind::HarmlessInfrastructureProof).allowed);

    InfrastructureProofObserved proof{};
    proof.token = 0x1234;
    proof.sequence = 8;
    proof.callbackThreadId = 44;
    proof.currentManagedThreadId = 1;
    proof.unityMainManagedThreadId = 1;
    proof.completed = true;
    SelfTestCheck(r, InfrastructureProofPostconditionSatisfied(
                         0x1234, 7, 44, 1, 1, proof));
    SelfTestCheck(r, !InfrastructureProofPostconditionSatisfied(
                          0x9999, 7, 44, 1, 1, proof));
    SelfTestCheck(r, !InfrastructureProofPostconditionSatisfied(
                          0x1234, 8, 44, 1, 1, proof));
    proof.callbackThreadId = 45;
    SelfTestCheck(r, !InfrastructureProofPostconditionSatisfied(
                          0x1234, 7, 44, 1, 1, proof));
    proof.callbackThreadId = 44;
    proof.currentManagedThreadId = 2;
    SelfTestCheck(r, !InfrastructureProofPostconditionSatisfied(
                          0x1234, 7, 44, 1, 1, proof));

    ObservedPostState before{};
    before.roleID = 100;
    before.mapID = 24;
    before.dead = true;
    ObservedPostState after = before;
    after.dead = false;
    SelfTestCheck(r, PostconditionSatisfied(ActionIntentKind::Revive, before, after));
    after.mapID = 25;
    SelfTestCheck(r, !PostconditionSatisfied(ActionIntentKind::Revive, before, after));

    before = {};
    before.roleID = 100;
    before.mapID = 24;
    before.autoFight = false;
    after = before;
    after.autoFight = true;
    SelfTestCheck(r, PostconditionSatisfied(ActionIntentKind::EnableCombatAutomation, before, after));
    SelfTestCheck(r, !PostconditionSatisfied(ActionIntentKind::DisableCombatAutomation, before, after));
    before.autoFight = true;
    after.autoFight = false;
    SelfTestCheck(r, PostconditionSatisfied(ActionIntentKind::DisableCombatAutomation, before, after));

    return r;
}

} // namespace tlcontrol
