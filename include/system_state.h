#pragma once

enum class SystemState
{
    BOOT,
    SELF_TEST,
    IDLE,
    WAIT_FOR_START,
    RUN_TESTS,
    PASS,
    FAIL,
    ERROR_STATE
};

const char *systemStateToString(SystemState state);
