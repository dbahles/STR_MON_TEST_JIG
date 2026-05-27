#pragma once

namespace RelayControl
{
    void begin();
    void setAlarmTestLow(bool isOn);
    void setAlarmTestHigh(bool isOn);
    void setOpenCircuitTest(bool isOn);
    void setShortCircuitTest(bool isOn);
    void allOff();
}
