#pragma once
#include <cstdint>

namespace tlcontrol {

inline constexpr bool kReviveMutationEnabled = true;
static_assert(kReviveMutationEnabled,
              "v1.2.0 intentionally enables only the Revive gameplay mutation gate");

class ReviveDispatcherGate {
public:
    static DispatchDecision Evaluate(bool metadataCapabilityResolved,
                                     bool harmlessHookProofPassed) {
        if (!kReviveMutationEnabled) return {false, L"revive-mutation-disabled"};
        if (!harmlessHookProofPassed) return {false, L"hook-envelope-unproven"};
        if (!metadataCapabilityResolved) return {false, L"revive-capability-unresolved"};
        return {true, L"revive-dispatch-allowed"};
    }
};

struct ReviveDispatchObserved {
    std::uint64_t token = 0;
    std::uint32_t callbackThreadId = 0;
    std::int32_t currentManagedThreadId = 0;
    std::int32_t unityMainManagedThreadId = 0;
    std::int32_t preRoleID = 0;
    std::int32_t preMapID = 0;
    bool preDead = false;
    bool invoked = false;
};

inline bool ReviveDispatchAckSatisfied(std::uint64_t expectedToken,
                                       std::uint32_t expectedHookThreadId,
                                       std::int32_t expectedManagedThreadId,
                                       std::int32_t expectedUnityMainThreadId,
                                       std::int32_t expectedRoleID,
                                       std::int32_t expectedMapID,
                                       const ReviveDispatchObserved& observed) {
    return expectedToken != 0 && observed.invoked && observed.preDead &&
           observed.token == expectedToken &&
           observed.callbackThreadId == expectedHookThreadId &&
           observed.currentManagedThreadId == expectedManagedThreadId &&
           observed.unityMainManagedThreadId == expectedUnityMainThreadId &&
           observed.currentManagedThreadId == observed.unityMainManagedThreadId &&
           observed.preRoleID == expectedRoleID &&
           observed.preMapID == expectedMapID;
}

struct RevivePostState {
    std::int32_t roleID = 0;
    std::int32_t mapID = 0;
    bool dead = false;
    bool mapReady = false;
    bool waitingChangeMap = false;
};

// Revive is allowed to change MapID. Role identity may not change.
// The controller additionally requires two consecutive stable alive snapshots.
inline bool RevivePostconditionSatisfied(const RevivePostState& before,
                                         const RevivePostState& after) {
    return before.roleID > 0 && before.roleID == after.roleID &&
           before.dead && !after.dead &&
           after.mapReady && !after.waitingChangeMap;
}

struct ReviveSelfTestResult {
    int total = 0;
    int passed = 0;
    int failed = 0;
    bool Pass() const { return failed == 0 && total == passed; }
};

inline void ReviveSelfTestCheck(ReviveSelfTestResult& result, bool condition) {
    ++result.total;
    if (condition) ++result.passed;
    else ++result.failed;
}

inline ReviveSelfTestResult RunReviveControlSelfTest() {
    ReviveSelfTestResult result{};
    ReviveSelfTestCheck(result, kReviveMutationEnabled);
    ReviveSelfTestCheck(result, ReviveDispatcherGate::Evaluate(true, true).allowed);
    ReviveSelfTestCheck(result, !ReviveDispatcherGate::Evaluate(false, true).allowed);
    ReviveSelfTestCheck(result, !ReviveDispatcherGate::Evaluate(true, false).allowed);

    ReviveDispatchObserved dispatch{};
    dispatch.token = 42;
    dispatch.callbackThreadId = 1234;
    dispatch.currentManagedThreadId = 1;
    dispatch.unityMainManagedThreadId = 1;
    dispatch.preRoleID = 100;
    dispatch.preMapID = 24;
    dispatch.preDead = true;
    dispatch.invoked = true;
    ReviveSelfTestCheck(result,
        ReviveDispatchAckSatisfied(42, 1234, 1, 1, 100, 24, dispatch));
    dispatch.token = 43;
    ReviveSelfTestCheck(result,
        !ReviveDispatchAckSatisfied(42, 1234, 1, 1, 100, 24, dispatch));
    dispatch.token = 42;
    dispatch.callbackThreadId = 1235;
    ReviveSelfTestCheck(result,
        !ReviveDispatchAckSatisfied(42, 1234, 1, 1, 100, 24, dispatch));
    dispatch.callbackThreadId = 1234;
    dispatch.preDead = false;
    ReviveSelfTestCheck(result,
        !ReviveDispatchAckSatisfied(42, 1234, 1, 1, 100, 24, dispatch));

    RevivePostState before{};
    before.roleID = 100;
    before.mapID = 24;
    before.dead = true;
    before.mapReady = true;
    RevivePostState after = before;
    after.mapID = 87; // map change after revive is valid
    after.dead = false;
    after.mapReady = true;
    after.waitingChangeMap = false;
    ReviveSelfTestCheck(result, RevivePostconditionSatisfied(before, after));
    after.roleID = 101;
    ReviveSelfTestCheck(result, !RevivePostconditionSatisfied(before, after));
    after.roleID = 100;
    after.mapReady = false;
    ReviveSelfTestCheck(result, !RevivePostconditionSatisfied(before, after));
    after.mapReady = true;
    after.waitingChangeMap = true;
    ReviveSelfTestCheck(result, !RevivePostconditionSatisfied(before, after));

    // Global gameplay gate from control_scaffold stays false; Revive is exclusive.
    ReviveSelfTestCheck(result, !kGameplayMutationEnabled);
    ReviveSelfTestCheck(result, !DispatcherGate::Evaluate(ActionIntentKind::EnableCombatAutomation).allowed);
    ReviveSelfTestCheck(result, !DispatcherGate::Evaluate(ActionIntentKind::NpcInteraction).allowed);
    ReviveSelfTestCheck(result, !DispatcherGate::Evaluate(ActionIntentKind::Sell).allowed);
    return result;
}

} // namespace tlcontrol
