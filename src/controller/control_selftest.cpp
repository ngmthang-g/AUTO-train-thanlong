#include <windows.h>
#include <cstdio>
#include "control_scaffold.h"

int wmain() {
    const tlcontrol::IntegratedSelfTestResult result =
        tlcontrol::RunIntegratedControlSelfTest();
    std::printf("CONTROL SELF TEST %s %d/%d (failed=%d)\n",
                result.Pass() ? "PASS" : "FAIL",
                 result.passed, result.total, result.failed);
    return result.Pass() ? 0 : 2;
}
