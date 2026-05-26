#include "led_manager.h"

#include <Arduino.h>
#include "pinmap.h"

namespace
{
    void writeLed(uint8_t pin, bool isOn)
    {
        digitalWrite(pin, isOn ? HIGH : LOW);
    }
}

namespace LedManager
{
    void begin()
    {
        pinMode(PIN_LED_RDY, OUTPUT);
        pinMode(PIN_LED_PASS, OUTPUT);
        pinMode(PIN_LED_FAIL, OUTPUT);
        showIdle();
    }

    void setReady(bool isOn)
    {
        writeLed(PIN_LED_RDY, isOn);
    }

    void setPass(bool isOn)
    {
        writeLed(PIN_LED_PASS, isOn);
    }

    void setFail(bool isOn)
    {
        writeLed(PIN_LED_FAIL, isOn);
    }

    void setIndicators(bool readyOn, bool passOn, bool failOn)
    {
        setReady(readyOn);
        setPass(passOn);
        setFail(failOn);
    }

    void showIdle()
    {
        setIndicators(false, false, false);
    }

    void showReady()
    {
        setIndicators(true, false, false);
    }

    void showRunning()
    {
        setIndicators(false, false, false);
    }

    void showPass()
    {
        setIndicators(false, true, false);
    }

    void showFail()
    {
        setIndicators(false, false, true);
    }

    void showError()
    {
        setIndicators(true, false, true);
    }
}
