#pragma once

namespace Buzzer
{
    void begin();
    void beep(unsigned long durationMs);
    void doubleBeep(unsigned long durationMs, unsigned long gapMs);
}
