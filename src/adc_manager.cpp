#include "adc_manager.h"

#include <Arduino.h>
#include "config.h"
#include "pinmap.h"

namespace AdcManager
{
    void begin()
    {
        pinMode(PIN_PWR_SENSE, INPUT);
        pinMode(PIN_ALM_SENSE, INPUT);
        analogSetPinAttenuation(PIN_PWR_SENSE, ADC_11db);
        analogSetPinAttenuation(PIN_ALM_SENSE, ADC_11db);
    }

    float readDutVoltage()
    {
#if SIMULATION_MODE
        return 24.0f;
#else
        uint32_t rawTotal = 0;

        for (uint8_t sample = 0; sample < ADC_SAMPLE_COUNT; sample++)
        {
            rawTotal += analogRead(PIN_PWR_SENSE);
            delay(2);
        }

        const float averageRaw = static_cast<float>(rawTotal) / ADC_SAMPLE_COUNT;
        const float adcPinVoltage = averageRaw * (ADC_REFERENCE_VOLTAGE / ADC_MAX_READING);

        // TODO: Replace DUT_POWER_ADC_SCALE after measuring the final voltage divider.
        return adcPinVoltage * DUT_POWER_ADC_SCALE;
#endif
    }

    float readAlarmSenseVoltage()
    {
#if SIMULATION_MODE
        return 3.0f;
#else
        uint32_t rawTotal = 0;

        for (uint8_t sample = 0; sample < ALARM_ADC_SAMPLES; sample++)
        {
            rawTotal += analogRead(PIN_ALM_SENSE);
            delay(2);
        }

        const float averageRaw = static_cast<float>(rawTotal) / ALARM_ADC_SAMPLES;

        // TODO: Replace this simple 3.3 V scale with calibrated hardware scaling.
        return averageRaw * (ADC_REFERENCE_VOLTAGE / ADC_MAX_READING);
#endif
    }
}
