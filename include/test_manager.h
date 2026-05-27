#pragma once

#include "test_defs.h"

class TestManager
{
public:
    void begin();
    TestResult runAllTests();

private:
    TestResult runPowerTest();
    TestResult runAlarmTest();
    TestResult runOpenCircuitTest();
    TestResult runShortCircuitTest();
    TestResult runFaultRelayTest();
    TestResult runSingleTest(TestId testId);
    TestResult runFaultSimulationTest(
        const char *testName,
        void (*setFaultOutput)(bool),
        const char *activateMessage,
        const char *restoreMessage);
};
