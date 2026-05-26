# Changelog

## Unreleased

- Added initial DUT detection state machine architecture.
- Added debounced DUT insertion/removal detection using the `PIN_FLT_NO` and `PIN_FLT_NC` inputs.
- Added TEST pushbutton handling on D16.
- Updated TEST pushbutton handling for GPIO16 with external pull-up and active-low activation.
- Added firmware version reporting to the serial boot banner.
- Added serial debug commands for starting tests, resetting to IDLE, printing status/help, and forcing PASS/FAIL.
- Added detailed state transition logging.
- Added READY, TEST_RUNNING, PASS, and FAIL workflow.
- Added buzzer helper and short insertion confirmation beep.
- Updated indicator handling so IDLE turns all status LEDs off.
- Documented state behaviour, DUT detection logic, expansion strategy, and current pin assignments.
