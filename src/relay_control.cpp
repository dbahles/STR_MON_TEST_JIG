#include "relay_control.h"

#include <Arduino.h>
#include "pinmap.h"

namespace
{
    void writeOutput(uint8_t pin, bool isOn)
    {
        digitalWrite(pin, isOn ? HIGH : LOW);
    }
}

namespace RelayControl
{
    void begin()
    {
        pinMode(PIN_ALM_TEST_L, OUTPUT);
        pinMode(PIN_ALM_TEST_H, OUTPUT);
        pinMode(PIN_EOL_OC_TEST, OUTPUT);
        pinMode(PIN_EOL_SC_TEST, OUTPUT);
        allOff();
    }

    void setAlarmTestLow(bool isOn)
    {
        writeOutput(PIN_ALM_TEST_L, isOn);
    }

    void setAlarmTestHigh(bool isOn)
    {
        writeOutput(PIN_ALM_TEST_H, isOn);
    }

    void allOff()
    {
        writeOutput(PIN_ALM_TEST_L, false);
        writeOutput(PIN_ALM_TEST_H, false);
        writeOutput(PIN_EOL_OC_TEST, false);
        writeOutput(PIN_EOL_SC_TEST, false);
    }
}
