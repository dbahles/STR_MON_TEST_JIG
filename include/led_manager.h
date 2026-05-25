#pragma once

namespace LedManager
{
    void begin();
    void setReady(bool isOn);
    void setPass(bool isOn);
    void setFail(bool isOn);
    void showIdle();
    void showRunning();
    void showPass();
    void showFail();
    void showError();
}
