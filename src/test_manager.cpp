#include "test_manager.h"

#include <Arduino.h>
#include <math.h>
#include "adc_manager.h"
#include "config.h"
#include "led_manager.h"
#include "pinmap.h"
#include "relay_control.h"
#include "serial_logger.h"

namespace
{
    const TestId TEST_SEQUENCE[] = {
#if ENABLE_POWER_TEST
        TestId::POWER_TEST,
#endif
        TestId::OPEN_CIRCUIT_TEST,
        TestId::SHORT_CIRCUIT_TEST,
#if ENABLE_ALARM_OUTPUT_TESTS
        TestId::ALARM_POSITIVE_TEST,
        TestId::ALARM_NEGATIVE_TEST,
#endif
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

    bool isComplementaryState(FaultRelayState state)
    {
        return state.ncHigh != state.noHigh;
    }

    bool isInvertedState(FaultRelayState originalState, FaultRelayState currentState)
    {
        return currentState.ncHigh == !originalState.ncHigh &&
               currentState.noHigh == !originalState.noHigh;
    }

    void logFaultRelayState(const char *label, FaultRelayState state)
    {
        Serial.print("  ");
        Serial.print(label);
        Serial.print(" NC=");
        Serial.print(state.ncHigh ? "HIGH" : "LOW");
        Serial.print(" NO=");
        Serial.println(state.noHigh ? "HIGH" : "LOW");
    }

    void serviceDelay(unsigned long durationMs)
    {
        const unsigned long startedAt = millis();

        while ((millis() - startedAt) < durationMs)
        {
            LedManager::update();
            delay(FAULT_RELAY_POLL_MS);
        }
    }

    bool waitForRelayInversion(FaultRelayState originalState, unsigned long timeoutMs)
    {
        const unsigned long startedAt = millis();
        unsigned long invertedSince = 0;
        bool loggedIntermediateState = false;

        while ((millis() - startedAt) < timeoutMs)
        {
            const FaultRelayState currentState = readFaultRelayState();
            if (isInvertedState(originalState, currentState))
            {
                if (invertedSince == 0)
                {
                    invertedSince = millis();
                    logFaultRelayState("Fault relay inverted:", currentState);
                }

                if ((millis() - invertedSince) >= FAULT_RELAY_STABLE_MS)
                {
                    return true;
                }
            }
            else
            {
                invertedSince = 0;

                if (!statesMatch(currentState, originalState) && !loggedIntermediateState)
                {
                    loggedIntermediateState = true;
                    logFaultRelayState("Fault relay intermediate state:", currentState);
                }
            }

            serviceDelay(FAULT_RELAY_POLL_MS);
        }

        return false;
    }

    bool waitForRelayRestore(FaultRelayState originalState, unsigned long timeoutMs)
    {
        const unsigned long startedAt = millis();
        unsigned long restoredSince = 0;

        while ((millis() - startedAt) < timeoutMs)
        {
            const FaultRelayState currentState = readFaultRelayState();
            if (statesMatch(currentState, originalState))
            {
                if (restoredSince == 0)
                {
                    restoredSince = millis();
                    logFaultRelayState("Fault relay restored:", currentState);
                }

                if ((millis() - restoredSince) >= FAULT_RELAY_STABLE_MS)
                {
                    return true;
                }
            }
            else
            {
                restoredSince = 0;
            }

            serviceDelay(FAULT_RELAY_POLL_MS);
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

            serviceDelay(FAULT_RELAY_POLL_MS);
        }

        return true;
    }

    bool isDutVoltageValid(float voltage)
    {
        return !isnan(voltage) &&
               voltage >= DUT_MIN_VOLTAGE &&
               voltage <= DUT_MAX_VOLTAGE;
    }

    void logDutVoltage(float voltage)
    {
        Serial.print("  DUT power voltage: ");
        Serial.print(voltage, 2);
        Serial.println(" V");
    }

    float readAndLogCurrent(const char *label)
    {
        const float currentMa = AdcManager::readCurrentMilliamps();
        Serial.print("  ");
        Serial.print(label);
        Serial.print(" current: ");
        Serial.print(currentMa, 2);
        Serial.println(" mA");
        return currentMa;
    }

    bool isValueBetween(float value, float minimum, float maximum)
    {
        return !isnan(value) &&
               value >= minimum &&
               value <= maximum;
    }

    bool isCloseTo(float value, float target, float tolerance)
    {
        return !isnan(value) &&
               fabs(value - target) <= tolerance;
    }

    void waitUntilMinimumElapsed(unsigned long startedAt, unsigned long minimumMs)
    {
        while ((millis() - startedAt) < minimumMs)
        {
            serviceDelay(FAULT_RELAY_POLL_MS);
        }
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

        if (TEST_STEP_DELAY_MS > 0 && (index + 1) < TEST_SEQUENCE_COUNT)
        {
            serviceDelay(TEST_STEP_DELAY_MS);
        }
    }

    SerialLogger::info("Automated test sequence complete");
    return TestResult::PASS;
}

uint8_t TestManager::getTestCount() const
{
    return TEST_SEQUENCE_COUNT;
}

TestId TestManager::getTestId(uint8_t index) const
{
    if (index >= TEST_SEQUENCE_COUNT)
    {
        return TEST_SEQUENCE[TEST_SEQUENCE_COUNT - 1];
    }

    return TEST_SEQUENCE[index];
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

    SerialLogger::testStart(testId);

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
    (void)testName;

    TestResult result = TestResult::PASS;

    RelayControl::setOpenCircuitTest(false);
    setAlarmOutput(false);
    serviceDelay(ALARM_CURRENT_SETTLE_MS);

    const float baselineCurrent = readAndLogCurrent("Baseline");

    SerialLogger::info(activateMessage);
    setAlarmOutput(true);
    serviceDelay(ALARM_CURRENT_SETTLE_MS);

    const float alarmCurrent = readAndLogCurrent("Alarm active");
    const float alarmIncrease = alarmCurrent - baselineCurrent;

    Serial.print("  Alarm current increase: ");
    Serial.print(alarmIncrease, 2);
    Serial.println(" mA");

    if (!isValueBetween(alarmIncrease, ALARM_CURRENT_INCREASE_MIN_MA, ALARM_CURRENT_INCREASE_MAX_MA))
    {
        SerialLogger::error("Alarm current increase out of expected range");
        result = TestResult::FAIL;
    }

    if (result == TestResult::PASS)
    {
        SerialLogger::info("Setting GPIO26 HIGH to apply open-circuit fault during alarm");
        RelayControl::setOpenCircuitTest(true);
        serviceDelay(ALARM_CURRENT_SETTLE_MS);

        const float alarmOcCurrent = readAndLogCurrent("Alarm + OC fault");
        const float ocDrop = alarmCurrent - alarmOcCurrent;

        Serial.print("  OC fault current drop: ");
        Serial.print(ocDrop, 2);
        Serial.println(" mA");

        if (!isValueBetween(ocDrop, ALARM_OC_CURRENT_DROP_MIN_MA, ALARM_OC_CURRENT_DROP_MAX_MA))
        {
            SerialLogger::error("OC fault current drop out of expected range");
            result = TestResult::FAIL;
        }
    }

    SerialLogger::info("Setting GPIO26 LOW and checking alarm current restores");
    RelayControl::setOpenCircuitTest(false);
    serviceDelay(ALARM_CURRENT_SETTLE_MS);

    if (result == TestResult::PASS)
    {
        const float alarmRestoredCurrent = readAndLogCurrent("Alarm restored");
        if (!isCloseTo(alarmRestoredCurrent, alarmCurrent, ALARM_CURRENT_RESTORE_TOLERANCE_MA))
        {
            SerialLogger::error("Alarm current did not restore after OC fault cleared");
            result = TestResult::FAIL;
        }
    }

    SerialLogger::info(restoreMessage);
    setAlarmOutput(false);
    serviceDelay(ALARM_CURRENT_SETTLE_MS);

    if (result == TestResult::PASS)
    {
        const float finalCurrent = readAndLogCurrent("Alarm off final");
        if (!isCloseTo(finalCurrent, baselineCurrent, ALARM_CURRENT_RESTORE_TOLERANCE_MA))
        {
            SerialLogger::error("Current did not return near baseline after alarm cleared");
            result = TestResult::FAIL;
        }
    }

    RelayControl::setOpenCircuitTest(false);
    setAlarmOutput(false);
    serviceDelay(ALARM_INTERTEST_DELAY_MS);

    return result;
}

TestResult TestManager::runFaultSimulationTest(
    const char *testName,
    void (*setFaultOutput)(bool),
    const char *activateMessage,
    const char *restoreMessage)
{
    (void)testName;

    const FaultRelayState originalState = readFaultRelayState();
    logFaultRelayState("Original fault relay state:", originalState);

    if (!isComplementaryState(originalState))
    {
        SerialLogger::error("Fault relay original NC and NO inputs are not complementary");
        return TestResult::FAIL;
    }

    SerialLogger::info(activateMessage);
    const unsigned long outputActivatedAt = millis();
    setFaultOutput(true);
    serviceDelay(FAULT_RELAY_SETTLE_BEFORE_READ_MS);

    if (!waitForRelayInversion(originalState, FAULT_RELAY_TIMEOUT_MS))
    {
        SerialLogger::error("Fault relay did not reach inverted complementary state before timeout");
        waitUntilMinimumElapsed(outputActivatedAt, TEST_OUTPUT_VISIBLE_ON_MS);
        setFaultOutput(false);
        serviceDelay(TEST_OUTPUT_VISIBLE_OFF_MS);
        return TestResult::TIMEOUT;
    }

    waitUntilMinimumElapsed(outputActivatedAt, TEST_OUTPUT_VISIBLE_ON_MS);

    SerialLogger::info(restoreMessage);
    setFaultOutput(false);
    serviceDelay(FAULT_RELAY_SETTLE_BEFORE_READ_MS);

    if (!waitForRelayRestore(originalState, FAULT_RELAY_TIMEOUT_MS))
    {
        SerialLogger::error("Fault relay did not restore before timeout");
        return TestResult::FAIL;
    }

    serviceDelay(TEST_OUTPUT_VISIBLE_OFF_MS);

    return TestResult::PASS;
}

TestResult TestManager::runPowerTest()
{
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
        "GPIO33 HIGH",
        "GPIO33 LOW");
}

TestResult TestManager::runAlarmNegativeTest()
{
    return runAlarmOutputTest(
        "Alarm Negative Test Started",
        RelayControl::setAlarmTestLow,
        "GPIO25 HIGH",
        "GPIO25 LOW");
}

TestResult TestManager::runOpenCircuitTest()
{
    return runFaultSimulationTest(
        "Starting open circuit test",
        RelayControl::setOpenCircuitTest,
        "Setting GPIO26 HIGH to simulate open circuit fault",
        "Setting GPIO26 LOW and checking relay restore");
}

TestResult TestManager::runShortCircuitTest()
{
    return runFaultSimulationTest(
        "Starting short circuit test",
        RelayControl::setShortCircuitTest,
        "Setting GPIO27 HIGH to simulate short circuit fault",
        "Setting GPIO27 LOW and checking relay restore");
}

TestResult TestManager::runFaultRelayTest()
{
#if SIMULATION_MODE
    serviceDelay(250);
    return TestResult::PASS;
#else
    const FaultRelayState state = readFaultRelayState();
    logFaultRelayState("Fault relay contact state:", state);

    if (state.ncHigh == state.noHigh)
    {
        SerialLogger::error("Fault relay NC and NO inputs are not complementary");
        return TestResult::FAIL;
    }

    return TestResult::PASS;
#endif
}
