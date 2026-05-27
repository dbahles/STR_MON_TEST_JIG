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
