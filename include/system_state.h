#pragma once

enum class SystemState
{
    IDLE,
    READY,
    TEST_RUNNING,
    PASS,
    FAIL,
    ERROR_STATE
};

const char *systemStateToString(SystemState state);
