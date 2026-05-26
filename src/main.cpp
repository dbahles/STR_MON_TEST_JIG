#include <Arduino.h>
#include <ctype.h>

#include "buzzer.h"
#include "config.h"
#include "led_manager.h"
#include "pinmap.h"
#include "serial_logger.h"
#include "system_state.h"
#include "test_manager.h"

namespace
{
    enum class DutPresence
    {
        REMOVED,
        INSERTED
    };

    SystemState currentState = SystemState::IDLE;
    TestManager testManager;
    unsigned long stateEnteredAt = 0;

    DutPresence stableDutPresence = DutPresence::REMOVED;
    bool lastRawDutPresent = false;
    bool holdResultForSerialDebug = false;
    unsigned long dutInputChangedAt = 0;

    void changeState(SystemState nextState, const char *reason)
    {
        if (currentState == nextState)
        {
            return;
        }

        const SystemState previousState = currentState;
        currentState = nextState;
        stateEnteredAt = millis();
        SerialLogger::stateChange(previousState, currentState, reason);
    }

    void changeState(SystemState nextState)
    {
        changeState(nextState, "");
    }

    void setupInputs()
    {
        pinMode(PIN_FLT_NC, INPUT_PULLDOWN);
        pinMode(PIN_FLT_NO, INPUT_PULLDOWN);
        pinMode(PIN_FLT_IN, INPUT_PULLUP);
        pinMode(PIN_TEST_SW, INPUT);
    }

    bool readRawDutPresent()
    {
        // The DUT is present if either detection input is HIGH.
        return digitalRead(PIN_FLT_NC) == HIGH ||
               digitalRead(PIN_FLT_NO) == HIGH;
    }

    bool isTestButtonPressed()
    {
        // The TEST switch has an external pull-up, so pressed reads LOW.
        return digitalRead(PIN_TEST_SW) == LOW;
    }

    void updateDutPresence()
    {
        const bool rawDutPresent = readRawDutPresent();

        if (rawDutPresent != lastRawDutPresent)
        {
            lastRawDutPresent = rawDutPresent;
            dutInputChangedAt = millis();
            return;
        }

        if ((millis() - dutInputChangedAt) < DUT_DEBOUNCE_MS)
        {
            return;
        }

        const DutPresence newPresence = rawDutPresent ? DutPresence::INSERTED : DutPresence::REMOVED;
        if (newPresence != stableDutPresence)
        {
            stableDutPresence = newPresence;
            SerialLogger::info(rawDutPresent ? "DUT detected" : "DUT removed");
        }
    }

    bool isDutInserted()
    {
        return stableDutPresence == DutPresence::INSERTED;
    }

    bool isDutRemoved()
    {
        return stableDutPresence == DutPresence::REMOVED;
    }

    void setIndicators(SystemState state)
    {
        switch (state)
        {
        case SystemState::IDLE:
            LedManager::showIdle();
            break;

        case SystemState::READY:
            LedManager::showReady();
            break;

        case SystemState::TEST_RUNNING:
            LedManager::showRunning();
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

    void buzzerBeep()
    {
        Buzzer::beep(BUZZER_CONFIRM_MS);
    }

    TestResult runTests()
    {
        // Future automated tests will continue to live behind this call.
        return testManager.runAllTests();
    }

    void completeTestRun(TestResult result)
    {
        if (result == TestResult::PASS)
        {
            changeState(SystemState::PASS, "Automated test sequence passed");
        }
        else
        {
            changeState(SystemState::FAIL, "Automated test sequence failed");
        }

        setIndicators(currentState);
    }

    void returnToIdle(const char *reason)
    {
        holdResultForSerialDebug = false;
        changeState(SystemState::IDLE, reason);
        setIndicators(currentState);
    }

    void startTestRun(const char *reason, bool holdResult)
    {
        holdResultForSerialDebug = holdResult;
        changeState(SystemState::TEST_RUNNING, reason);
        setIndicators(currentState);
    }

    void printHelpMenu()
    {
        Serial.println();
        Serial.println("Serial debug commands:");
        Serial.println("  T = Start test");
        Serial.println("  R = Reset system to IDLE");
        Serial.println("  S = Print current system status");
        Serial.println("  H = Print this help menu");
        Serial.println("  P = Force PASS state");
        Serial.println("  F = Force FAIL state");
        Serial.println();
    }

    void printSystemStatus()
    {
        Serial.println();
        Serial.println("System status:");
        Serial.print("  Firmware: ");
        Serial.println(FIRMWARE_VERSION);
        Serial.print("  State: ");
        Serial.println(systemStateToString(currentState));
        Serial.print("  Stable DUT present: ");
        Serial.println(isDutInserted() ? "YES" : "NO");
        Serial.print("  Raw PIN_FLT_NO: ");
        Serial.println(digitalRead(PIN_FLT_NO) == HIGH ? "HIGH" : "LOW");
        Serial.print("  Raw PIN_FLT_NC: ");
        Serial.println(digitalRead(PIN_FLT_NC) == HIGH ? "HIGH" : "LOW");
        Serial.print("  TEST switch: ");
        Serial.println(isTestButtonPressed() ? "PRESSED" : "RELEASED");
        Serial.print("  Development mode: ");
        Serial.println(DEVELOPMENT_MODE ? "ON" : "OFF");
        Serial.print("  Serial result hold: ");
        Serial.println(holdResultForSerialDebug ? "ON" : "OFF");
        Serial.println();
    }

    void handleSerialCommand(char command)
    {
        command = static_cast<char>(toupper(command));

        Serial.print("Command received: ");
        Serial.println(command);

        switch (command)
        {
        case 'T':
            startTestRun("Serial command T", true);
            break;

        case 'R':
            returnToIdle("Serial command R");
            break;

        case 'S':
            printSystemStatus();
            break;

        case 'H':
            printHelpMenu();
            break;

        case 'P':
            holdResultForSerialDebug = true;
            changeState(SystemState::PASS, "Serial command P");
            setIndicators(currentState);
            break;

        case 'F':
            holdResultForSerialDebug = true;
            changeState(SystemState::FAIL, "Serial command F");
            setIndicators(currentState);
            break;

        case '\r':
        case '\n':
            break;

        default:
            SerialLogger::error("Unknown serial command. Send H for help.");
            break;
        }
    }

    void handleSerialCommands()
    {
#if DEVELOPMENT_MODE
        while (Serial.available() > 0)
        {
            handleSerialCommand(static_cast<char>(Serial.read()));
        }
#endif
    }

    void handleStateMachine()
    {
        updateDutPresence();

        switch (currentState)
        {
        case SystemState::IDLE:
            if (isDutInserted())
            {
                changeState(SystemState::READY, "DUT insertion confirmed");
                holdResultForSerialDebug = false;
                setIndicators(currentState);
                buzzerBeep();
            }
            break;

        case SystemState::READY:
            if (isDutRemoved())
            {
                returnToIdle("DUT removed before test");
            }
            else if (isTestButtonPressed())
            {
                startTestRun("TEST pushbutton pressed", false);
            }
            break;

        case SystemState::TEST_RUNNING:
            completeTestRun(runTests());
            break;

        case SystemState::PASS:
        case SystemState::FAIL:
            if (!holdResultForSerialDebug && isDutRemoved())
            {
                returnToIdle("DUT removed");
            }
            break;

        case SystemState::ERROR_STATE:
            setIndicators(currentState);
            break;
        }
    }
}

void setup()
{
    SerialLogger::begin(SERIAL_BAUDRATE);
    SerialLogger::printBootBanner();
    setupInputs();
    LedManager::begin();
    Buzzer::begin();
    testManager.begin();
    setIndicators(currentState);
    SerialLogger::info("System ready. Send H for serial command help.");
}

void loop()
{
    handleSerialCommands();
    handleStateMachine();
    delay(LOOP_DELAY_MS);
}
