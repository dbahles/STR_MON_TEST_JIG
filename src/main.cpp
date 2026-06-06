#include <Arduino.h>
#include <ctype.h>

#include "buzzer.h"
#include "adc_manager.h"
#include "config.h"
#include "led_manager.h"
#include "pinmap.h"
#include "relay_control.h"
#include "serial_logger.h"
#include "system_state.h"
#include "test_manager.h"

#define SIMPLE_HELLO_WORLD_DIAGNOSTIC 0

#if SIMPLE_HELLO_WORLD_DIAGNOSTIC

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println();
    Serial.println("Hello world from STR-MON test jig ESP32");
    Serial.println("Diagnostic firmware: no jig GPIOs are configured");
}

void loop()
{
    static unsigned long lastPrintAt = 0;

    if ((millis() - lastPrintAt) >= 1000)
    {
        lastPrintAt = millis();
        Serial.print("Alive ms=");
        Serial.println(millis());
    }
}

#else

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
    bool manualTestMode = false;
    bool manualAlarmPositiveOn = false;
    bool manualAlarmNegativeOn = false;
    uint8_t manualTestIndex = 0;
    unsigned long dutInputChangedAt = 0;
    unsigned long lastTestCompletedAt = 0;

    bool stableTestButtonPressed = false;
    bool lastRawTestButtonPressed = false;
    bool testButtonArmed = false;
    bool testButtonPressEvent = false;
    unsigned long testButtonInputChangedAt = 0;

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

    bool readRawTestButtonPressed()
    {
        // The TEST switch has an external pull-up, so pressed reads LOW.
        return digitalRead(PIN_TEST_SW) == LOW;
    }

    void updateTestButton()
    {
        testButtonPressEvent = false;

        const bool rawPressed = readRawTestButtonPressed();
        if (rawPressed != lastRawTestButtonPressed)
        {
            lastRawTestButtonPressed = rawPressed;
            testButtonInputChangedAt = millis();
            return;
        }

        if ((millis() - testButtonInputChangedAt) < TEST_BUTTON_DEBOUNCE_MS)
        {
            return;
        }

        if (rawPressed == stableTestButtonPressed)
        {
            if (!stableTestButtonPressed && !testButtonArmed)
            {
                testButtonArmed = true;
            }

            return;
        }

        stableTestButtonPressed = rawPressed;

        if (!stableTestButtonPressed)
        {
            // A release arms the next press. This prevents an input already LOW
            // during reset from immediately starting a test.
            testButtonArmed = true;
            return;
        }

        if (testButtonArmed)
        {
            testButtonPressEvent = true;
            testButtonArmed = false;
        }
    }

    bool consumeTestButtonPress()
    {
        if (!testButtonPressEvent)
        {
            return false;
        }

        testButtonPressEvent = false;
        return true;
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

    void setManualAlarmOutputs(bool positiveOn, bool negativeOn, const char *reason)
    {
        const bool changed = manualAlarmPositiveOn != positiveOn ||
                             manualAlarmNegativeOn != negativeOn;

        manualAlarmPositiveOn = positiveOn;
        manualAlarmNegativeOn = negativeOn;

        RelayControl::setAlarmTestHigh(manualAlarmPositiveOn);
        RelayControl::setAlarmTestLow(manualAlarmNegativeOn);

        if (changed)
        {
            Serial.print("Manual alarm output: ");
            Serial.print(reason);
            Serial.print(" | GPIO33=");
            Serial.print(manualAlarmPositiveOn ? "HIGH" : "LOW");
            Serial.print(" GPIO25=");
            Serial.println(manualAlarmNegativeOn ? "HIGH" : "LOW");
        }
    }

    TestResult runTests()
    {
        // Future automated tests will continue to live behind this call.
        return testManager.runAllTests();
    }

    void completeTestRun(TestResult result)
    {
        manualTestMode = false;
        lastTestCompletedAt = millis();

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

    bool isTestRestartLockedOut()
    {
        return lastTestCompletedAt != 0 &&
               (millis() - lastTestCompletedAt) < TEST_RESTART_LOCKOUT_MS;
    }

    void returnToIdle(const char *reason)
    {
        holdResultForSerialDebug = false;
        manualTestMode = false;
        manualTestIndex = 0;
        setManualAlarmOutputs(false, false, "OFF");
        changeState(SystemState::IDLE, reason);
        setIndicators(currentState);
    }

    void startTestRun(const char *reason, bool holdResult)
    {
        if (isTestRestartLockedOut())
        {
            SerialLogger::error("Ignoring test start during restart lockout");
            return;
        }

        manualTestMode = false;
        manualTestIndex = 0;
        setManualAlarmOutputs(false, false, "OFF");
        holdResultForSerialDebug = holdResult;
        Serial.print("Starting test run: ");
        Serial.println(reason);
        changeState(SystemState::TEST_RUNNING, reason);
        setIndicators(currentState);
    }

    void printManualPrompt()
    {
        Serial.print("Manual test mode: next test ");
        Serial.print(manualTestIndex + 1);
        Serial.print(" of ");
        Serial.print(testManager.getTestCount());
        Serial.print(" is ");
        Serial.println(testIdToString(testManager.getTestId(manualTestIndex)));
        Serial.println("Press TEST or send N to run the next test. Send R to exit.");
    }

    void startManualTestMode(const char *reason)
    {
        if (!isDutInserted())
        {
            SerialLogger::error("Fit DUT before enabling manual test mode.");
            return;
        }

        manualTestMode = true;
        manualTestIndex = 0;
        holdResultForSerialDebug = false;
        changeState(SystemState::READY, reason);
        setIndicators(currentState);
        SerialLogger::info("Manual test mode enabled");
        printManualPrompt();
    }

    void runNextManualTest(const char *reason)
    {
        if (!manualTestMode)
        {
            SerialLogger::error("Manual test mode is not enabled. Send M first.");
            return;
        }

        if (manualTestIndex >= testManager.getTestCount())
        {
            manualTestMode = false;
            changeState(SystemState::PASS, "Manual test sequence complete");
            setIndicators(currentState);
            return;
        }

        const TestId testId = testManager.getTestId(manualTestIndex);

        changeState(SystemState::TEST_RUNNING, reason);
        setIndicators(currentState);

        Serial.print("Manual step ");
        Serial.print(manualTestIndex + 1);
        Serial.print(" of ");
        Serial.print(testManager.getTestCount());
        Serial.print(": ");
        Serial.println(testIdToString(testId));

        const TestResult result = testManager.runSingleTest(testId);
        if (result != TestResult::PASS)
        {
            manualTestMode = false;
            changeState(SystemState::FAIL, "Manual test step failed");
            setIndicators(currentState);
            return;
        }

        manualTestIndex++;
        if (manualTestIndex >= testManager.getTestCount())
        {
            manualTestMode = false;
            changeState(SystemState::PASS, "Manual test sequence complete");
            setIndicators(currentState);
            return;
        }

        changeState(SystemState::READY, "Manual test step passed");
        setIndicators(currentState);
        printManualPrompt();
    }

    void printHelpMenu()
    {
        Serial.println();
        Serial.println("Serial debug commands:");
        Serial.println("  T = Start full automated test sequence");
        Serial.println("  M = Enable manual test stepping");
        Serial.println("  N = Run next manual test step");
        Serial.println("  R = Reset system to IDLE");
        Serial.println("  S = Print current system status");
        Serial.println("  H = Print this help menu");
        Serial.println("  L = List test sequence");
        Serial.println("  P = Force PASS state");
        Serial.println("  F = Force FAIL state");
        Serial.println("  A = Manual alarm positive ON (GPIO33 HIGH)");
        Serial.println("  B = Manual alarm negative ON (GPIO25 HIGH)");
        Serial.println("  O = Manual alarm outputs OFF");
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
        const uint16_t powerSenseRaw = AdcManager::readPowerSenseRaw();
        const float powerSenseAdcVoltage = AdcManager::rawToAdcVoltage(powerSenseRaw);
        const float currentMilliamps = AdcManager::adcVoltageToCurrentMilliamps(powerSenseAdcVoltage);
        Serial.print("  PWR_SENSE raw ADC: ");
        Serial.println(powerSenseRaw);
        Serial.print("  PWR_SENSE ADC voltage: ");
        Serial.print(powerSenseAdcVoltage, 3);
        Serial.println(" V");
        Serial.print("  INA240 current estimate: ");
        Serial.print(currentMilliamps, 1);
        Serial.println(" mA");
        Serial.print("  TEST switch: ");
        Serial.println(stableTestButtonPressed ? "PRESSED" : "RELEASED");
        Serial.print("  Raw PIN_TEST_SW: ");
        Serial.println(digitalRead(PIN_TEST_SW) == HIGH ? "HIGH" : "LOW");
        Serial.print("  TEST switch armed: ");
        Serial.println(testButtonArmed ? "YES" : "NO");
        Serial.print("  Test restart lockout: ");
        Serial.println(isTestRestartLockedOut() ? "ACTIVE" : "OFF");
        Serial.print("  Development mode: ");
        Serial.println(DEVELOPMENT_MODE ? "ON" : "OFF");
        Serial.print("  Serial result hold: ");
        Serial.println(holdResultForSerialDebug ? "ON" : "OFF");
        Serial.print("  Manual test mode: ");
        Serial.println(manualTestMode ? "ON" : "OFF");
        Serial.print("  Manual alarm positive GPIO33: ");
        Serial.println(manualAlarmPositiveOn ? "ON" : "OFF");
        Serial.print("  Manual alarm negative GPIO25: ");
        Serial.println(manualAlarmNegativeOn ? "ON" : "OFF");
        if (manualTestMode)
        {
            Serial.print("  Manual next step: ");
            Serial.print(manualTestIndex + 1);
            Serial.print(" of ");
            Serial.print(testManager.getTestCount());
            Serial.print(" - ");
            Serial.println(testIdToString(testManager.getTestId(manualTestIndex)));
        }
        Serial.println();
    }

    void handleSerialCommand(char command)
    {
        command = static_cast<char>(toupper(command));

        if (command == '\r' || command == '\n')
        {
            return;
        }

        Serial.print("Command received: ");
        Serial.println(command);

        switch (command)
        {
        case 'T':
            startTestRun("Serial command T", true);
            break;

        case 'M':
            startManualTestMode("Serial command M");
            break;

        case 'N':
            runNextManualTest("Serial command N");
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

        case 'L':
            testManager.printTestSequence();
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

        case 'A':
            setManualAlarmOutputs(true, false, "POSITIVE ON");
            break;

        case 'B':
            setManualAlarmOutputs(false, true, "NEGATIVE ON");
            break;

        case 'O':
            setManualAlarmOutputs(false, false, "OFF");
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
        updateTestButton();

#if !ENABLE_AUTO_DUT_STATE_MACHINE
        if (currentState == SystemState::TEST_RUNNING)
        {
            completeTestRun(runTests());
        }
        return;
#endif

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
            else if (consumeTestButtonPress())
            {
                if (manualTestMode)
                {
                    runNextManualTest("TEST pushbutton manual step");
                }
                else
                {
                    startTestRun("TEST pushbutton pressed", false);
                }
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
    Serial.println("BOOT: setup start");
    Serial.flush();
    delay(100);

    SerialLogger::printBootBanner();
    Serial.println("BOOT: setup inputs");
    Serial.flush();
    setupInputs();

    Serial.println("BOOT: setup LEDs");
    Serial.flush();
    LedManager::begin();

    Serial.println("BOOT: setup buzzer");
    Serial.flush();
    Buzzer::begin();

    Serial.println("BOOT: setup test manager");
    Serial.flush();
    testManager.begin();

    Serial.println("BOOT: setup indicators");
    Serial.flush();
    setIndicators(currentState);

    SerialLogger::info("System ready. Send H for serial command help.");
}

void loop()
{
    handleSerialCommands();
    handleStateMachine();
    delay(LOOP_DELAY_MS);
}

#endif
