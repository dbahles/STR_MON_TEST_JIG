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

    void showIdle()
    {
        setReady(true);
        setPass(false);
        setFail(false);
    }

    void showRunning()
    {
        setReady(false);
        setPass(false);
        setFail(false);
    }

    void showPass()
    {
        setReady(false);
        setPass(true);
        setFail(false);
    }

    void showFail()
    {
        setReady(false);
        setPass(false);
        setFail(true);
    }

    void showError()
    {
        setReady(true);
        setPass(false);
        setFail(true);
    }
}
