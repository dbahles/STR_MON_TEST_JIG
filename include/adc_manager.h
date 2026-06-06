#pragma once

#include <stdint.h>

namespace AdcManager
{
    void begin();
    uint16_t readPowerSenseRaw();
    float rawToAdcVoltage(uint16_t raw);
    float adcVoltageToCurrentMilliamps(float adcVoltage);
    float readPowerSenseAdcVoltage();
    float readDutVoltage();
    float readCurrentMilliamps();
}
