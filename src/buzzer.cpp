#include "buzzer.h"

#include <Arduino.h>
#include "pinmap.h"

namespace Buzzer
{
    void begin()
    {
        pinMode(PIN_BUZZER, OUTPUT);
        digitalWrite(PIN_BUZZER, LOW);
    }

    void beep(unsigned long durationMs)
    {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(durationMs);
        digitalWrite(PIN_BUZZER, LOW);
    }
}
