#include "system_state.h"

const char *systemStateToString(SystemState state)
{
    switch (state)
    {
    case SystemState::BOOT:
        return "BOOT";
    case SystemState::SELF_TEST:
        return "SELF_TEST";
    case SystemState::IDLE:
        return "IDLE";
    case SystemState::WAIT_FOR_START:
        return "WAIT_FOR_START";
    case SystemState::RUN_TESTS:
        return "RUN_TESTS";
    case SystemState::PASS:
        return "PASS";
    case SystemState::FAIL:
        return "FAIL";
    case SystemState::ERROR_STATE:
        return "ERROR_STATE";
    }

    return "UNKNOWN";
}
