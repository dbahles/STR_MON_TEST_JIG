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
    TestResult runFaultRelayTest();
    TestResult runSingleTest(TestId testId);
};
