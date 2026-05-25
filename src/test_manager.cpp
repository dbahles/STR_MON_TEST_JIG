#include "test_manager.h"

#include <Arduino.h>
#include "config.h"
#include "serial_logger.h"

void TestManager::begin()
{
    SerialLogger::info("Test manager initialized");
}

TestResult TestManager::runAllTests()
{
    SerialLogger::info("Starting automated test sequence");

    TestResult result = runSingleTest(TestId::POWER_TEST);
    if (result != TestResult::PASS)
    {
        return result;
    }

    result = runSingleTest(TestId::ALARM_TEST);
    if (result != TestResult::PASS)
    {
        return result;
    }

    result = runSingleTest(TestId::FAULT_RELAY_TEST);
    if (result != TestResult::PASS)
    {
        return result;
    }

    SerialLogger::info("Automated test sequence complete");
    return TestResult::PASS;
}

TestResult TestManager::runSingleTest(TestId testId)
{
    TestResult result = TestResult::ERROR;

    switch (testId)
    {
    case TestId::POWER_TEST:
        result = runPowerTest();
        break;

    case TestId::ALARM_TEST:
        result = runAlarmTest();
        break;

    case TestId::FAULT_RELAY_TEST:
        result = runFaultRelayTest();
        break;
    }

    SerialLogger::testResult(testId, result);
    return result;
}

TestResult TestManager::runPowerTest()
{
#if SIMULATION_MODE
    delay(250);
    return TestResult::PASS;
#else
    // Future: read DUT power using ADC manager.
    return TestResult::ERROR;
#endif
}

TestResult TestManager::runAlarmTest()
{
#if SIMULATION_MODE
    delay(250);
    return TestResult::PASS;
#else
    // Future: drive alarm outputs using relay control.
    return TestResult::ERROR;
#endif
}

TestResult TestManager::runFaultRelayTest()
{
#if SIMULATION_MODE
    delay(250);
    return TestResult::PASS;
#else
    // Future: verify fault relay NO/NC contacts.
    return TestResult::ERROR;
#endif
}
