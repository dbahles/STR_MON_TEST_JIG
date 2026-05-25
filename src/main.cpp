#include <Arduino.h>

#include "config.h"
#include "led_manager.h"
#include "serial_logger.h"
#include "system_state.h"
#include "test_manager.h"

namespace
{
    SystemState currentState = SystemState::BOOT;
    TestManager testManager;
    unsigned long stateEnteredAt = 0;

    void changeState(SystemState nextState)
    {
        currentState = nextState;
        stateEnteredAt = millis();

        SerialLogger::info(systemStateToString(currentState));
    }

    bool hasStateTimedOut(unsigned long timeoutMs)
    {
        return (millis() - stateEnteredAt) >= timeoutMs;
    }

    void runSelfTest()
    {
        SerialLogger::info("Running controller self-test");
        LedManager::showRunning();
        delay(250);
        SerialLogger::info("Self-test complete");
        changeState(SystemState::IDLE);
    }

    void runAutomatedTests()
    {
        LedManager::showRunning();

        const TestResult result = testManager.runAllTests();
        if (result == TestResult::PASS)
        {
            changeState(SystemState::PASS);
        }
        else
        {
            changeState(SystemState::FAIL);
        }
    }

    void handleStateMachine()
    {
        switch (currentState)
        {
        case SystemState::BOOT:
            changeState(SystemState::SELF_TEST);
            break;

        case SystemState::SELF_TEST:
            runSelfTest();
            break;

        case SystemState::IDLE:
            LedManager::showIdle();
            changeState(SystemState::WAIT_FOR_START);
            break;

        case SystemState::WAIT_FOR_START:
#if SIMULATION_MODE
            if (hasStateTimedOut(START_DELAY_MS))
            {
                SerialLogger::info("Simulation start trigger");
                changeState(SystemState::RUN_TESTS);
            }
#endif
            break;

        case SystemState::RUN_TESTS:
            runAutomatedTests();
            break;

        case SystemState::PASS:
            LedManager::showPass();
            break;

        case SystemState::FAIL:
            LedManager::showFail();
            break;

        case SystemState::ERROR_STATE:
            LedManager::showError();
            break;
        }
    }
}

void setup()
{
    SerialLogger::begin(SERIAL_BAUDRATE);
    SerialLogger::printBootBanner();
    LedManager::begin();
    testManager.begin();
    changeState(SystemState::IDLE);
}

void loop()
{
    handleStateMachine();
    delay(LOOP_DELAY_MS);
}
