# Changelog

## Unreleased

- Added initial DUT detection state machine architecture.
- Added debounced DUT insertion/removal detection using the `PIN_FLT_NO` and `PIN_FLT_NC` inputs.
- Added TEST pushbutton handling on GPIO32.
- Updated TEST pushbutton handling for GPIO32 with external pull-up and active-low activation.
- Added firmware version reporting to the serial boot banner.
- Added serial debug commands for starting tests, resetting to IDLE, printing status/help, and forcing PASS/FAIL.
- Added detailed state transition logging.
- Added automated open-circuit and short-circuit tests using GPIO17/GPIO16 and fault relay input transition/restore checks.
- Noted that open/short relay behavior still requires validation on final hardware.
- Added alarm positive and alarm negative automated tests with averaged GPIO39 ADC voltage checks and fault relay stability checks.
- Added serial `L` command to list the automated test sequence in run order.
- Changed TEST pushbutton handling so tests start only on a fresh debounced press, preventing auto-start after reset if the input is already LOW.
- Added baseline DUT power test using averaged GPIO36 ADC readings and provisional voltage scaling.
- Added hardware validation checklist and marked firmware ready for first hardware validation.
- Added READY, TEST_RUNNING, PASS, and FAIL workflow.
- Added buzzer helper and short insertion confirmation beep.
- Updated indicator handling so IDLE turns all status LEDs off.
- Documented state behaviour, DUT detection logic, expansion strategy, and current pin assignments.
