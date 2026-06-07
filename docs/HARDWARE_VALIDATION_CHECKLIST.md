# Hardware Validation Checklist

Status: ready for first hardware validation.

Use this checklist when the real test jig hardware arrives. Keep serial monitor open at 115200 baud and save any unusual output.

## Before Power-Up

- [ ] Confirm ESP32 board type matches `esp32doit-devkit-v1`.
- [ ] Confirm all wiring uses ESP32 GPIO numbers, not dev-board physical pin numbers.
- [ ] Confirm 3.3 V logic levels on all ESP32 inputs.
- [ ] Confirm alarm, EOL, LED, and buzzer outputs cannot overload ESP32 pins.
- [ ] Confirm DUT power sense divider output is below 3.3 V at maximum DUT voltage.

## Firmware Identity

- [ ] Build firmware with `pio run`.
- [ ] Upload firmware to ESP32.
- [ ] Open serial monitor at 115200 baud.
- [ ] Confirm boot banner shows firmware version `0.6.0`.
- [ ] Confirm boot banner shows simulation mode `OFF`.
- [ ] Send `H` and confirm command help appears.
- [ ] Send `L` and confirm the test order:
  - Open Circuit Test
  - Short Circuit Test
  - Alarm Positive Test
  - Alarm Negative Test
  - Fault Relay Test
  - Power Test, if `ENABLE_POWER_TEST` is enabled.

## Basic IO Validation

- [ ] Send `S` with no DUT fitted and record input states.
- [ ] Confirm READY LED slowly flashes with no DUT fitted.
- [ ] Fit DUT and confirm READY state after debounce.
- [ ] Confirm READY LED turns solid on.
- [ ] Confirm buzzer gives a short insertion beep.
- [ ] Confirm TEST switch reads released when idle and pressed when held. The firmware expects GPIO16 LOW when pressed.
- [ ] Press TEST button and confirm test starts only on a fresh press.
- [ ] Confirm buzzer gives a short start beep.
- [ ] Confirm PASS and FAIL LEDs alternate while tests are running.
- [ ] Power-cycle with TEST held down and confirm test does not auto-start.
- [ ] Remove DUT and confirm state returns to `IDLE`.

## Manual Step Validation

- [ ] Fit DUT and wait for `READY`.
- [ ] Send `M` and confirm manual test mode prints the next test name.
- [ ] Press TEST and confirm only the Power Test runs.
- [ ] Press TEST again or send `N` and confirm only the next test runs.
- [ ] Continue stepping until `PASS`, or record the first failed step and serial output.
- [ ] Send `R` and confirm manual mode exits and the state returns to `IDLE`.

## Power Sense Validation

- [ ] Measure DUT voltage with a calibrated multimeter.
- [ ] Run the Power Test and record logged DUT voltage.
- [ ] Compare logged voltage with multimeter reading.
- [ ] Adjust `DUT_POWER_ADC_SCALE` if required.
- [ ] Confirm valid DUT voltage passes between `DUT_MIN_VOLTAGE` and `DUT_MAX_VOLTAGE`.
- [ ] Confirm out-of-range voltage fails safely if practical to test.

## Alarm Output Validation

- [ ] Confirm INA240 output is wired to ESP32 GPIO36, not `EN`.
- [ ] Confirm `S` status prints a plausible INA current estimate before running alarm tests.
- [ ] Run Alarm Positive Test.
- [ ] Confirm GPIO33 activates the expected hardware path.
- [ ] Confirm alarm activation increases current by roughly 40 mA.
- [ ] Confirm enabling EOL open-circuit drops current by roughly 12-15 mA.
- [ ] Confirm current returns near the alarm-current reading after EOL open-circuit is cleared.
- [ ] Confirm current returns near baseline after alarm positive is cleared.
- [ ] Confirm fault relay inputs do not change during Alarm Positive Test.
- [ ] Run Alarm Negative Test.
- [ ] Confirm GPIO25 activates the expected hardware path.
- [ ] Repeat the current increase, OC drop, restore, and baseline checks for alarm negative.
- [ ] Confirm fault relay inputs do not change during Alarm Negative Test.

## Open And Short Fault Validation

- [ ] Run Open Circuit Test.
- [ ] Confirm GPIO26 activates the open-circuit simulation.
- [ ] Confirm `PIN_FLT_NO` / `PIN_FLT_NC` transition is detected.
- [ ] Confirm relay restores after GPIO26 is turned off.
- [ ] Run Short Circuit Test.
- [ ] Confirm GPIO27 activates the short-circuit simulation.
- [ ] Confirm `PIN_FLT_NO` / `PIN_FLT_NC` transition is detected.
- [ ] Confirm relay restores after GPIO27 is turned off.

## Result Handling

- [ ] Confirm PASS LED turns on after all tests pass.
- [ ] Confirm FAIL LED turns on after a forced or real failure.
- [ ] Confirm PASS or FAIL result gives a short double beep.
- [ ] Confirm PASS/FAIL remains displayed until DUT removal.
- [ ] With DUT still fitted after PASS/FAIL, release TEST, then hold TEST for 1.5 seconds and confirm the jig returns to `READY`.
- [ ] Press TEST again and confirm a second run can start without removing the DUT.
- [ ] Send `R` and confirm state returns to `IDLE`.
- [ ] Confirm all test outputs return LOW after pass, fail, timeout, or reset.

## Calibration Follow-Up

- [ ] Record final INA zero-current voltage on GPIO36.
- [ ] Record final INA reading with a known load/current.
- [ ] Update `INA240_ZERO_CURRENT_V` and current thresholds if required.
- [ ] Record final measured power ADC scale.
- [ ] Decide whether calibration should be stored in firmware constants or NVS.
- [ ] Commit updated calibration constants after validation.
