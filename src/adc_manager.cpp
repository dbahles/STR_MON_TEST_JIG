#include "adc_manager.h"

#include <Arduino.h>
#include "config.h"
#include "pinmap.h"

namespace AdcManager
{
    uint16_t readPowerSenseRaw()
    {
#if SIMULATION_MODE
        return 0;
#else
        uint32_t rawTotal = 0;

        for (uint8_t sample = 0; sample < ADC_SAMPLE_COUNT; sample++)
        {
            rawTotal += analogRead(PIN_PWR_SENSE);
            delay(2);
        }

        return static_cast<uint16_t>(rawTotal / ADC_SAMPLE_COUNT);
#endif
    }

    void begin()
    {
        pinMode(PIN_PWR_SENSE, INPUT);
        analogSetPinAttenuation(PIN_PWR_SENSE, ADC_11db);
    }

    float rawToAdcVoltage(uint16_t raw)
    {
        return static_cast<float>(raw) * (ADC_REFERENCE_VOLTAGE / ADC_MAX_READING);
    }

    float adcVoltageToCurrentMilliamps(float adcVoltage)
    {
        const float senseVoltage = adcVoltage - INA240_ZERO_CURRENT_V;
        const float currentAmps = senseVoltage / (INA240_GAIN * INA240_SHUNT_OHMS);
        return currentAmps * 1000.0f;
    }

    float readPowerSenseAdcVoltage()
    {
#if SIMULATION_MODE
        return 0.0f;
#else
        return rawToAdcVoltage(readPowerSenseRaw());
#endif
    }

    float readDutVoltage()
    {
#if SIMULATION_MODE
        return 24.0f;
#else
        // TODO: Replace DUT_POWER_ADC_SCALE after measuring the final voltage divider.
        return readPowerSenseAdcVoltage() * DUT_POWER_ADC_SCALE;
#endif
    }

    float readCurrentMilliamps()
    {
#if SIMULATION_MODE
        return 0.0f;
#else
        return adcVoltageToCurrentMilliamps(readPowerSenseAdcVoltage());
#endif
    }

}
