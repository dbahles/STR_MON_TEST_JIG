#include "serial_logger.h"

#include "config.h"

namespace
{
    void printPrefix(const char *level)
    {
        Serial.print('[');
        Serial.print(millis());
        Serial.print(" ms] ");
        Serial.print(level);
        Serial.print(": ");
    }
}

namespace SerialLogger
{
    void begin(uint32_t baudRate)
    {
        Serial.begin(baudRate);
        delay(100);
    }

    void printBootBanner()
    {
        Serial.println();
        Serial.println("================================");
        Serial.println(" STR-MON Automated Test Jig");
        Serial.print(" Firmware version: ");
        Serial.println(FIRMWARE_VERSION);
        Serial.print(" Simulation mode: ");
        Serial.println(SIMULATION_MODE ? "ON" : "OFF");
        Serial.print(" Development mode: ");
        Serial.println(DEVELOPMENT_MODE ? "ON" : "OFF");
        Serial.print(" Serial baud: ");
        Serial.println(SERIAL_BAUDRATE);
        Serial.println("================================");
    }

    void info(const char *message)
    {
        printPrefix("INFO");
        Serial.println(message);
    }

    void error(const char *message)
    {
        printPrefix("ERROR");
        Serial.println(message);
    }

    void stateChange(SystemState fromState, SystemState toState, const char *reason)
    {
        printPrefix("STATE");
        Serial.print(systemStateToString(fromState));
        Serial.print(" -> ");
        Serial.print(systemStateToString(toState));

        if (reason != nullptr && reason[0] != '\0')
        {
            Serial.print(" | ");
            Serial.print(reason);
        }

        Serial.println();
    }

    void testResult(TestId testId, TestResult result)
    {
        printPrefix("TEST");
        Serial.print(testIdToString(testId));
        Serial.print(" -> ");
        Serial.println(testResultToString(result));
    }
}
