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
