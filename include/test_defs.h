#pragma once

enum class TestId
{
    POWER_TEST,
    ALARM_POSITIVE_TEST,
    ALARM_NEGATIVE_TEST,
    OPEN_CIRCUIT_TEST,
    SHORT_CIRCUIT_TEST,
    FAULT_RELAY_TEST
};

enum class TestResult
{
    PASS,
    FAIL,
    ERROR,
    TIMEOUT
};

const char *testIdToString(TestId testId);
const char *testResultToString(TestResult result);
