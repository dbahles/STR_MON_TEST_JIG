# Hardware Validation Checklist

Status: ready for first hardware validation.

Use this checklist when the real test jig hardware arrives. Keep serial monitor open at 115200 baud and save any unusual output.

## Before Power-Up

- [ ] Confirm ESP32 board type matches `esp32doit-devkit-v1`.
- [ ] Confirm all wiring uses ESP32 GPIO numbers, not dev-board physical pin numbers.
- [ ] Confirm 3.3 V logic levels on all ESP32 inputs.
- [ ] Confirm alarm, EOL, LED, and buzzer outputs cannot overload ESP32 pins.
- [ ] Confirm DUT power sense divider output is below 3.3 V at maximum DUT voltage.
- [ ] Confirm alarm sense input output is below 3.3 V at maximum expected alarm voltage.

## Firmware Identity

- [ ] Build firmware with `pio run`.
- [ ] Upload firmware to ESP32.
- [ ] Open serial monitor at 115200 baud.
- [ ] Confirm boot banner shows firmware version `0.5.0`.
- [ ] Send `H` and confirm command help appears.
- [ ] Send `L` and confirm the test order:
  - Power Test
  - Alarm Positive Test
  - Alarm Negative Test
  - Open Circuit Test
  - Short Circuit Test
  - Fault Relay Test

## Basic IO Validation

- [ ] Send `S` with no DUT fitted and record input states.
- [ ] Fit DUT and confirm READY state after debounce.
- [ ] Confirm READY LED turns on.
- [ ] Confirm buzzer gives a short insertion beep.
- [ ] Press TEST button and confirm test starts only on a fresh press.
- [ ] Power-cycle with TEST held down and confirm test does not auto-start.
- [ ] Remove DUT and confirm state returns to `IDLE`.

## Power Sense Validation

- [ ] Measure DUT voltage with a calibrated multimeter.
- [ ] Run the Power Test and record logged DUT voltage.
- [ ] Compare logged voltage with multimeter reading.
- [ ] Adjust `DUT_POWER_ADC_SCALE` if required.
- [ ] Confirm valid DUT voltage passes between `DUT_MIN_VOLTAGE` and `DUT_MAX_VOLTAGE`.
- [ ] Confirm out-of-range voltage fails safely if practical to test.

## Alarm Sense Validation

- [ ] Run Alarm Positive Test.
- [ ] Confirm GPIO19 activates the expected hardware path.
- [ ] Confirm GPIO39 logged alarm sense voltage is plausible.
- [ ] Confirm fault relay inputs do not change during Alarm Positive Test.
- [ ] Run Alarm Negative Test.
- [ ] Confirm GPIO18 activates the expected hardware path.
- [ ] Confirm GPIO39 logged alarm sense voltage is plausible.
- [ ] Confirm fault relay inputs do not change during Alarm Negative Test.
- [ ] Adjust `ALARM_SENSE_MIN_V` and `ALARM_SENSE_MAX_V` if required.

## Open And Short Fault Validation

- [ ] Run Open Circuit Test.
- [ ] Confirm GPIO17 activates the open-circuit simulation.
- [ ] Confirm `PIN_FLT_NO` / `PIN_FLT_NC` transition is detected.
- [ ] Confirm relay restores after GPIO17 is turned off.
- [ ] Run Short Circuit Test.
- [ ] Confirm GPIO16 activates the short-circuit simulation.
- [ ] Confirm `PIN_FLT_NO` / `PIN_FLT_NC` transition is detected.
- [ ] Confirm relay restores after GPIO16 is turned off.

## Result Handling

- [ ] Confirm PASS LED turns on after all tests pass.
- [ ] Confirm FAIL LED turns on after a forced or real failure.
- [ ] Confirm PASS/FAIL remains displayed until DUT removal.
- [ ] Send `R` and confirm state returns to `IDLE`.
- [ ] Confirm all test outputs return LOW after pass, fail, timeout, or reset.

## Calibration Follow-Up

- [ ] Record final measured power ADC scale.
- [ ] Record final measured alarm sense voltage range.
- [ ] Decide whether calibration should be stored in firmware constants or NVS.
- [ ] Commit updated calibration constants after validation.
