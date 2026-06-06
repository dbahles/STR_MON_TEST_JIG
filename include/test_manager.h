#pragma once

#include <Arduino.h>

#include "test_defs.h"

class TestManager
{
public:
    void begin();
    TestResult runAllTests();
    TestResult runSingleTest(TestId testId);
    uint8_t getTestCount() const;
    TestId getTestId(uint8_t index) const;
    void printTestSequence();

private:
    TestResult runPowerTest();
    TestResult runAlarmPositiveTest();
    TestResult runAlarmNegativeTest();
    TestResult runOpenCircuitTest();
    TestResult runShortCircuitTest();
    TestResult runFaultRelayTest();
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
