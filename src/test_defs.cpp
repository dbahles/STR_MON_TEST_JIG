#include "test_defs.h"

const char *testIdToString(TestId testId)
{
    switch (testId)
    {
    case TestId::POWER_TEST:
        return "Power Test";
    case TestId::ALARM_TEST:
        return "Alarm Test";
    case TestId::OPEN_CIRCUIT_TEST:
        return "Open Circuit Test";
    case TestId::SHORT_CIRCUIT_TEST:
        return "Short Circuit Test";
    case TestId::FAULT_RELAY_TEST:
        return "Fault Relay Test";
    }

    return "Unknown Test";
}

const char *testResultToString(TestResult result)
{
    switch (result)
    {
    case TestResult::PASS:
        return "PASS";
    case TestResult::FAIL:
        return "FAIL";
    case TestResult::ERROR:
        return "ERROR";
    case TestResult::TIMEOUT:
        return "TIMEOUT";
    }

    return "UNKNOWN";
}
