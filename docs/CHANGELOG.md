# Changelog

## Unreleased

- Reordered the active automated sequence so open-circuit and short-circuit tests run before alarm positive and alarm negative tests.
- Added operator feedback patterns: slow READY LED flash when no DUT is present, solid READY when detected, alternating PASS/FAIL LEDs while testing, start beep, and result double beep.
- Added TEST button long-hold retest support: after PASS or FAIL, release then hold TEST for 1.5 seconds to return to READY without removing the DUT.
- Tightened open/short fault relay validation so tests require a stable inverted NC/NO state instead of passing on any contact change such as temporary `LOW/LOW`.
- Milestone: hardware bring-up firmware now has normal jig, GPIO hardware-test, and INA-test PlatformIO environments.
- Moved the current-sense ADC input to GPIO36 after finding the INA output had accidentally been wired to the ESP32 EN pin.
- Added INA240 current reading support using a 0.5 ohm shunt and provisional gain/zero calibration constants.
- Reworked alarm positive and alarm negative tests to use current deltas:
  - record baseline current,
  - enable alarm output and check for about 40 mA increase,
  - enable EOL open-circuit output and check for about 12-15 mA drop,
  - verify current restores when outputs are cleared.
- Re-enabled automatic DUT scanning and TEST switch starting in the main firmware.
- Added manual serial controls for alarm outputs during bench testing.
- Added standalone hardware-test firmware for direct GPIO output/input checks from serial commands.
- Added standalone INA-test firmware for ADC/current calibration and manual alarm/OC output toggling.
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
