# Development Notes

## Current Release State

- Firmware version: `0.7.0`.
- PlatformIO environment: `esp32doit-devkit-v1`.
- Serial baud: `115200`.
- Simulation mode is off.
- Development serial commands remain enabled for bench support.

## Active Test Sequence

With `ENABLE_POWER_TEST 0`, the active sequence is:

1. Open Circuit Test
2. Short Circuit Test
3. Alarm Positive Test
4. Alarm Negative Test
5. Fault Relay Test

The Power Test code remains available but disabled until power-sense calibration is ready.

## Hardware Notes

- Fault relay inputs are GPIO22 (`PIN_FLT_NO`) and GPIO23 (`PIN_FLT_NC`).
- Relay contact pulldowns were changed from 10K to 100K after bench testing showed the stronger pulldowns could prevent reliable ESP32 HIGH readings.
- INA240 current sense is on GPIO36.
- Do not wire analog signals to ESP32 `EN`; driving `EN` can reset the module.
- INA240 uses a 0.5 ohm shunt and provisional firmware calibration constants.

## Operator Feedback

- No DUT: READY LED slow flashes.
- DUT detected: READY LED solid and one short beep.
- Test running: PASS and FAIL LEDs alternate.
- Test started: one short beep.
- PASS or FAIL: result LED solid and short double beep.
- After PASS or FAIL, release TEST, then hold TEST for 1.5 seconds to return to `READY` without removing the DUT.

## Cleanup Notes

- Standalone hardware-test and INA-test sketches were removed from the release tree.
- The alpha checkpoint commit keeps the bring-up code available in Git history if needed.
