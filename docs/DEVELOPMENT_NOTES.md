# Development Notes

## 2026-05-25

### Current Status

- Created initial documentation placeholders in `docs/`.
- Added source placeholders in `src/`.
- Added header placeholders in `include/`.
- Generated the first working firmware skeleton for the ESP32-based STR-MON production test jig.
- Confirmed the project builds successfully with PlatformIO for `esp32doit-devkit-v1`.

### Firmware Skeleton Added

- Central state machine in `src/main.cpp`.
- State enum in `include/system_state.h`.
- Test result and test ID enums in `include/test_defs.h`.
- Simulation mode enabled in `include/config.h`.
- Fake test sequence implemented in `src/test_manager.cpp`:
  - Power Test
  - Alarm Test
  - Fault Relay Test
- Serial logging module added:
  - `include/serial_logger.h`
  - `src/serial_logger.cpp`
- LED manager module added:
  - `include/led_manager.h`
  - `src/led_manager.cpp`
- Placeholder modules added for future expansion:
  - ADC manager
  - Relay control
  - Calibration system

### Startup Behavior

- Serial starts at `115200`.
- Boot banner is printed.
- LEDs are initialized.
- Firmware enters `IDLE`, then `WAIT_FOR_START`.
- In simulation mode, tests start automatically after `START_DELAY_MS`.
- Passing simulation tests set the system to `PASS`.

### Build Verification

Command run:

```text
pio run
```

Result:

```text
[SUCCESS] Took 10.51 seconds
```

### Notes For Next Session

- Replace simulation test logic with real hardware checks when the jig hardware is available.
- Decide whether the start trigger will come from `PIN_TEST_SW`, serial command, or both.
- Flesh out ADC voltage measurement and calibration storage.
- Flesh out relay output control and fault relay contact verification.
- Add serial command handling once the basic hardware workflow is confirmed.

## 2026-05-26

### Added Serial Debug Interface

- Added firmware version reporting in the boot banner.
- Added detailed state transition logging.
- Added case-insensitive serial commands:
  - `T` starts the test sequence.
  - `R` resets to `IDLE`.
  - `S` prints system status.
  - `H` prints help.
  - `P` forces `PASS`.
  - `F` forces `FAIL`.
- Serial-triggered PASS/FAIL results are held until `R` so testing can be done without hardware fitted.
- Confirmed serial command support is intended for development mode.

### Added Open And Short Tests

- Added open-circuit test using GPIO17.
- Added short-circuit test using GPIO16.
- Both tests monitor `PIN_FLT_NO` and `PIN_FLT_NC`.
- Each test checks for relay transition from the original state, then checks restore after the output is turned off.
- Transition and restore checks use 2 second timeouts with short polling intervals.
- Build and upload were confirmed on the ESP32 dev board.
- Full relay behavior validation is pending until the real jig hardware is available.

### Added Alarm Positive And Negative Tests

- Added alarm positive test using GPIO19.
- Added alarm negative test using GPIO18.
- Added averaged alarm sense voltage reading on GPIO39.
- Configured GPIO39 ADC attenuation with `ADC_11db`.
- Alarm voltage must be between `ALARM_SENSE_MIN_V` and `ALARM_SENSE_MAX_V`.
- Alarm tests verify `PIN_FLT_NO` and `PIN_FLT_NC` do not change during the active alarm output period.
- Alarm outputs are returned LOW before each test exits.
- Added serial `L` command to print the current automated test sequence.

### TEST Button Start Guard

- Updated TEST pushbutton handling to require a fresh debounced press event.
- A button/input already LOW during reset will not start a test until it is released and pressed again.
- Serial command `T` still starts the test sequence immediately in development mode.

## 2026-05-27

### Ready For Hardware Validation

- Added real baseline power test using `PIN_PWR_SENSE` / GPIO36.
- Power test averages ADC samples and applies a provisional voltage-divider scale factor.
- Simulation mode returns 24.0 V for the power test.
- Added `docs/HARDWARE_VALIDATION_CHECKLIST.md` for first hardware bring-up.
- Current firmware is ready for hardware validation, with ADC scale and thresholds expected to need calibration.

## 2026-06-01

### Alarm Sense Follow-Up

- `ALM_SENSE` testing has been removed from the active alarm positive and alarm negative tests for now.
- Revisit `ALM_SENSE` once the rest of the jig is working reliably on hardware.
- Keep space for later ideas around how to validate alarm feedback without blocking the current bring-up work.

## 2026-06-06

### Hardware Bring-Up Milestone

- Created standalone firmware environments for bench testing:
  - `hardware-test` for direct serial control of jig GPIO outputs and input status reads.
  - `ina-test` for isolated INA240 / ADC current measurement checks.
- Found a wiring/pin-counting issue where the INA output was connected to ESP32 `EN` instead of an ADC GPIO.
- Symptom was ESP32 rebooting or appearing to lock up as soon as the INA signal was connected.
- Corrected current-sense mapping to GPIO36.
- GPIO36 is input-only and is suitable for the INA240 analog output.
- Confirmed the OC fault output causes the expected alarm current drop, roughly 13 mA on the bench.

### Current Main Firmware State

- Normal firmware is active under `esp32doit-devkit-v1`.
- Automatic DUT detection is enabled again with `ENABLE_AUTO_DUT_STATE_MACHINE`.
- The physical TEST switch can start the automated test sequence.
- Manual test mode is available from serial with `M`, then each TEST switch press or `N` runs one step.
- Power test remains temporarily disabled with `ENABLE_POWER_TEST 0`.
- Alarm positive and alarm negative tests are enabled and now use INA current deltas instead of the old alarm-sense voltage check.
- INA calibration constants are still provisional and should be revisited once final current measurements are recorded.
