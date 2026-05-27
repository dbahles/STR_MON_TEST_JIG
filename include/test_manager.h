#pragma once

#include "test_defs.h"

class TestManager
{
public:
    void begin();
    TestResult runAllTests();
    void printTestSequence();

private:
    TestResult runPowerTest();
    TestResult runAlarmPositiveTest();
    TestResult runAlarmNegativeTest();
    TestResult runOpenCircuitTest();
    TestResult runShortCircuitTest();
    TestResult runFaultRelayTest();
    TestResult runSingleTest(TestId testId);
    TestResult runAlarmOutputTest(
        const char *testName,
        void (*setAlarmOutput)(bool),
        const char *activateMessage,
        const char *restoreMessage);
    TestResult runFaultSimulationTest(
        const char *testName,
        void (*setFaultOutput)(bool),
        const char *activateMessage,
        const char *restoreMessage);
};
