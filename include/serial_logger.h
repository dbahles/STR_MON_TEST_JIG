#pragma once

#include <Arduino.h>
#include "test_defs.h"

namespace SerialLogger
{
    void begin(uint32_t baudRate);
    void printBootBanner();
    void info(const char *message);
    void error(const char *message);
    void testResult(TestId testId, TestResult result);
}
