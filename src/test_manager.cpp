#include "test_manager.h"

#include <Arduino.h>
#include <math.h>
#include "adc_manager.h"
#include "config.h"
#include "pinmap.h"
#include "relay_control.h"
#include "serial_logger.h"

namespace
{
    const TestId TEST_SEQUENCE[] = {
        TestId::POWER_TEST,
        TestId::ALARM_POSITIVE_TEST,
        TestId::ALARM_NEGATIVE_TEST,
        TestId::OPEN_CIRCUIT_TEST,
        TestId::SHORT_CIRCUIT_TEST,
        TestId::FAULT_RELAY_TEST};

    const uint8_t TEST_SEQUENCE_COUNT = sizeof(TEST_SEQUENCE) / sizeof(TEST_SEQUENCE[0]);

    struct FaultRelayState
    {
        bool ncHigh;
        bool noHigh;
    };

    FaultRelayState readFaultRelayState()
    {
        FaultRelayState state;
        state.ncHigh = digitalRead(PIN_FLT_NC) == HIGH;
        state.noHigh = digitalRead(PIN_FLT_NO) == HIGH;
        return state;
    }

    bool statesMatch(FaultRelayState left, FaultRelayState right)
    {
        return left.ncHigh == right.ncHigh &&
               left.noHigh == right.noHigh;
    }

    void logFaultRelayState(const char *label, FaultRelayState state)
    {
        Serial.print(label);
        Serial.print(" NC=");
        Serial.print(state.ncHigh ? "HIGH" : "LOW");
        Serial.print(" NO=");
        Serial.println(state.noHigh ? "HIGH" : "LOW");
    }

    bool waitForRelayChange(FaultRelayState originalState, unsigned long timeoutMs)
    {
        const unsigned long startedAt = millis();

        while ((millis() - startedAt) < timeoutMs)
        {
            const FaultRelayState currentState = readFaultRelayState();
            if (!statesMatch(currentState, originalState))
            {
                logFaultRelayState("Fault relay changed:", currentState);
                return true;
            }

            delay(FAULT_RELAY_POLL_MS);
        }

        return false;
    }

    bool waitForRelayRestore(FaultRelayState originalState, unsigned long timeoutMs)
    {
        const unsigned long startedAt = millis();

        while ((millis() - startedAt) < timeoutMs)
        {
            const FaultRelayState currentState = readFaultRelayState();
            if (statesMatch(currentState, originalState))
            {
                logFaultRelayState("Fault relay restored:", currentState);
                return true;
            }

            delay(FAULT_RELAY_POLL_MS);
        }

        return false;
    }

    bool isFaultRelayStateValid(FaultRelayState originalState)
    {
        const FaultRelayState currentState = readFaultRelayState();
        if (!statesMatch(currentState, originalState))
        {
            logFaultRelayState("Fault relay changed unexpectedly:", currentState);
            return false;
        }

        return true;
    }

    bool waitWithRelayStable(FaultRelayState originalState, unsigned long durationMs)
    {
        const unsigned long startedAt = millis();

        while ((millis() - startedAt) < durationMs)
        {
            if (!isFaultRelayStateValid(originalState))
            {
                return false;
            }

            delay(FAULT_RELAY_POLL_MS);
        }

        return true;
    }

    bool isAlarmVoltageValid(float voltage)
    {
        return !isnan(voltage) &&
               voltage >= ALARM_SENSE_MIN_V &&
               voltage <= ALARM_SENSE_MAX_V;
    }

    void logAlarmVoltage(float voltage)
    {
        Serial.print("Alarm Sense Voltage = ");
        Serial.print(voltage, 2);
        Serial.println(" V");
    }

    bool isDutVoltageValid(float voltage)
    {
        return !isnan(voltage) &&
               voltage >= DUT_MIN_VOLTAGE &&
               voltage <= DUT_MAX_VOLTAGE;
    }

    void logDutVoltage(float voltage)
    {
        Serial.print("DUT Power Voltage = ");
        Serial.print(voltage, 2);
        Serial.println(" V");
    }
}

void TestManager::begin()
{
    AdcManager::begin();
    RelayControl::begin();
    SerialLogger::info("Test manager initialized");
}

TestResult TestManager::runAllTests()
{
    SerialLogger::info("Starting automated test sequence");

    for (uint8_t index = 0; index < TEST_SEQUENCE_COUNT; index++)
    {
        const TestResult result = runSingleTest(TEST_SEQUENCE[index]);
        if (result != TestResult::PASS)
        {
            return result;
        }
    }

    SerialLogger::info("Automated test sequence complete");
    return TestResult::PASS;
}

void TestManager::printTestSequence()
{
    Serial.println();
    Serial.println("Test sequence:");

    for (uint8_t index = 0; index < TEST_SEQUENCE_COUNT; index++)
    {
        Serial.print("  ");
        Serial.print(index + 1);
        Serial.print(". ");
        Serial.println(testIdToString(TEST_SEQUENCE[index]));
    }

    Serial.println();
}

TestResult TestManager::runSingleTest(TestId testId)
{
    TestResult result = TestResult::ERROR;

    switch (testId)
    {
    case TestId::POWER_TEST:
        result = runPowerTest();
        break;

    case TestId::ALARM_POSITIVE_TEST:
        result = runAlarmPositiveTest();
        break;

    case TestId::ALARM_NEGATIVE_TEST:
        result = runAlarmNegativeTest();
        break;

    case TestId::OPEN_CIRCUIT_TEST:
        result = runOpenCircuitTest();
        break;

    case TestId::SHORT_CIRCUIT_TEST:
        result = runShortCircuitTest();
        break;

    case TestId::FAULT_RELAY_TEST:
        result = runFaultRelayTest();
        break;
    }

    SerialLogger::testResult(testId, result);
    return result;
}

TestResult TestManager::runAlarmOutputTest(
    const char *testName,
    void (*setAlarmOutput)(bool),
    const char *activateMessage,
    const char *restoreMessage)
{
    SerialLogger::info(testName);

    const FaultRelayState originalState = readFaultRelayState();
    logFaultRelayState("Initial fault relay state:", originalState);

    SerialLogger::info(activateMessage);
    setAlarmOutput(true);

    TestResult result = TestResult::PASS;

    if (!waitWithRelayStable(originalState, ALARM_SETTLE_DELAY_MS))
    {
        SerialLogger::error("Fault relay state changed during alarm settle delay");
        result = TestResult::FAIL;
    }

    if (result == TestResult::PASS)
    {
        const float alarmVoltage = AdcManager::readAlarmSenseVoltage();
        logAlarmVoltage(alarmVoltage);

        if (!isAlarmVoltageValid(alarmVoltage))
        {
            SerialLogger::error("Alarm sense voltage out of range");
            result = TestResult::FAIL;
        }
    }

    if (result == TestResult::PASS && !isFaultRelayStateValid(originalState))
    {
        SerialLogger::error("Fault relay state changed during alarm test");
        result = TestResult::FAIL;
    }

    if (result == TestResult::PASS)
    {
        SerialLogger::info("Holding alarm output active");
        if (!waitWithRelayStable(originalState, ALARM_TEST_DURATION_MS))
        {
            SerialLogger::error("Fault relay state changed while alarm output was active");
            result = TestResult::FAIL;
        }
    }

    SerialLogger::info(restoreMessage);
    setAlarmOutput(false);
    delay(ALARM_INTERTEST_DELAY_MS);

    return result;
}

TestResult TestManager::runFaultSimulationTest(
    const char *testName,
    void (*setFaultOutput)(bool),
    const char *activateMessage,
    const char *restoreMessage)
{
    SerialLogger::info(testName);

    const FaultRelayState originalState = readFaultRelayState();
    logFaultRelayState("Original fault relay state:", originalState);

    SerialLogger::info(activateMessage);
    setFaultOutput(true);

    if (!waitForRelayChange(originalState, FAULT_RELAY_TIMEOUT_MS))
    {
        SerialLogger::error("Fault relay did not change before timeout");
        setFaultOutput(false);
        return TestResult::TIMEOUT;
    }

    SerialLogger::info(restoreMessage);
    setFaultOutput(false);

    if (!waitForRelayRestore(originalState, FAULT_RELAY_TIMEOUT_MS))
    {
        SerialLogger::error("Fault relay did not restore before timeout");
        return TestResult::FAIL;
    }

    return TestResult::PASS;
}

TestResult TestManager::runPowerTest()
{
    SerialLogger::info("Power baseline test started");

    const float dutVoltage = AdcManager::readDutVoltage();
    logDutVoltage(dutVoltage);

    if (!isDutVoltageValid(dutVoltage))
    {
        SerialLogger::error("DUT power voltage out of range");
        return TestResult::FAIL;
    }

    return TestResult::PASS;
}

TestResult TestManager::runAlarmPositiveTest()
{
    return runAlarmOutputTest(
        "Alarm Positive Test Started",
        RelayControl::setAlarmTestHigh,
        "GPIO19 HIGH",
        "GPIO19 LOW");
}

TestResult TestManager::runAlarmNegativeTest()
{
    return runAlarmOutputTest(
        "Alarm Negative Test Started",
        RelayControl::setAlarmTestLow,
        "GPIO18 HIGH",
        "GPIO18 LOW");
}

TestResult TestManager::runOpenCircuitTest()
{
    return runFaultSimulationTest(
        "Starting open circuit test",
        RelayControl::setOpenCircuitTest,
        "Setting IO17 HIGH to simulate open circuit fault",
        "Setting IO17 LOW and checking relay restore");
}

TestResult TestManager::runShortCircuitTest()
{
    return runFaultSimulationTest(
        "Starting short circuit test",
        RelayControl::setShortCircuitTest,
        "Setting IO16 HIGH to simulate short circuit fault",
        "Setting IO16 LOW and checking relay restore");
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
