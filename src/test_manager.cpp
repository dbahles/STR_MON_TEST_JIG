#include "test_manager.h"

#include <Arduino.h>
#include "config.h"
#include "pinmap.h"
#include "relay_control.h"
#include "serial_logger.h"

namespace
{
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
}

void TestManager::begin()
{
    RelayControl::begin();
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

    result = runSingleTest(TestId::OPEN_CIRCUIT_TEST);
    if (result != TestResult::PASS)
    {
        return result;
    }

    result = runSingleTest(TestId::SHORT_CIRCUIT_TEST);
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

TestResult TestManager::runOpenCircuitTest()
{
    return runFaultSimulationTest(
        "Starting open circuit test",
        RelayControl::setOpenCircuitTest,
        "Setting IO26 HIGH to simulate open circuit fault",
        "Setting IO26 LOW and checking relay restore");
}

TestResult TestManager::runShortCircuitTest()
{
    return runFaultSimulationTest(
        "Starting short circuit test",
        RelayControl::setShortCircuitTest,
        "Setting IO27 HIGH to simulate short circuit fault",
        "Setting IO27 LOW and checking relay restore");
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
