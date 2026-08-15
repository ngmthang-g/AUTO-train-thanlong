#include <cstdio>
#include "control_scaffold.h"
#include "revive_control_gate.h"

int main() {
    const tlcontrol::ReviveSelfTestResult result = tlcontrol::RunReviveControlSelfTest();
    std::printf("REVIVE CONTROL SELF TEST %s %d/%d (failed=%d)\n",
                result.Pass() ? "PASS" : "FAIL",
                result.passed, result.total, result.failed);
    return result.Pass() ? 0 : 1;
}
