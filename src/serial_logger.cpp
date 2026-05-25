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
        Serial.println(" ESP32 Firmware Skeleton");
        Serial.print(" Simulation mode: ");
        Serial.println(SIMULATION_MODE ? "ON" : "OFF");
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

    void testResult(TestId testId, TestResult result)
    {
        printPrefix("TEST");
        Serial.print(testIdToString(testId));
        Serial.print(" -> ");
        Serial.println(testResultToString(result));
    }
}
