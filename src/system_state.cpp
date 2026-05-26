#include "system_state.h"

const char *systemStateToString(SystemState state)
{
    switch (state)
    {
    case SystemState::IDLE:
        return "IDLE";
    case SystemState::READY:
        return "READY";
    case SystemState::TEST_RUNNING:
        return "TEST_RUNNING";
    case SystemState::PASS:
        return "PASS";
    case SystemState::FAIL:
        return "FAIL";
    case SystemState::ERROR_STATE:
        return "ERROR_STATE";
    }

    return "UNKNOWN";
}
