#pragma once

namespace LedManager
{
    void begin();
    void setReady(bool isOn);
    void setPass(bool isOn);
    void setFail(bool isOn);
    void setIndicators(bool readyOn, bool passOn, bool failOn);
    void showIdle();
    void showReady();
    void showRunning();
    void showPass();
    void showFail();
    void showError();
}
