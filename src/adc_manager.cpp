#include "adc_manager.h"

#include "config.h"

namespace AdcManager
{
    void begin()
    {
        // Future: configure ADC channels and averaging.
    }

    float readDutVoltage()
    {
#if SIMULATION_MODE
        return 24.0f;
#else
        return 0.0f;
#endif
    }
}
