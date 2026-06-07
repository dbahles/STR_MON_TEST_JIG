#include "led_manager.h"

#include <Arduino.h>
#include "config.h"
#include "pinmap.h"

namespace
{
    enum class DisplayMode
    {
        STATIC,
        IDLE_FLASH,
        RUNNING_FLASH
    };

    DisplayMode displayMode = DisplayMode::STATIC;
    bool flashPhase = false;
    unsigned long lastFlashAt = 0;

    void writeLed(uint8_t pin, bool isOn)
    {
        digitalWrite(pin, isOn ? HIGH : LOW);
    }

    void setMode(DisplayMode mode)
    {
        displayMode = mode;
        flashPhase = false;
        lastFlashAt = millis();
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
        setMode(DisplayMode::STATIC);
        setReady(readyOn);
        setPass(passOn);
        setFail(failOn);
    }

    void showIdle()
    {
        setMode(DisplayMode::IDLE_FLASH);
        setReady(false);
        setPass(false);
        setFail(false);
    }

    void showReady()
    {
        setIndicators(true, false, false);
    }

    void showRunning()
    {
        setMode(DisplayMode::RUNNING_FLASH);
        setReady(false);
        setPass(true);
        setFail(false);
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

    void update()
    {
        const unsigned long now = millis();

        if (displayMode == DisplayMode::IDLE_FLASH)
        {
            if ((now - lastFlashAt) >= IDLE_READY_LED_FLASH_MS)
            {
                lastFlashAt = now;
                flashPhase = !flashPhase;
                setReady(flashPhase);
                setPass(false);
                setFail(false);
            }

            return;
        }

        if (displayMode == DisplayMode::RUNNING_FLASH)
        {
            if ((now - lastFlashAt) >= RUNNING_RESULT_LED_FLASH_MS)
            {
                lastFlashAt = now;
                flashPhase = !flashPhase;
                setReady(false);
                setPass(flashPhase);
                setFail(!flashPhase);
            }
        }
    }
}
