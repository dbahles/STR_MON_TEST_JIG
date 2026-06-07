# Changelog

## 0.7.0 - Release Build

- Kept a single PlatformIO firmware environment: `esp32doit-devkit-v1`.
- Removed standalone bring-up sketches and helper scripts from the release tree.
- Reordered the active automated sequence:
  - Open Circuit Test
  - Short Circuit Test
  - Alarm Positive Test
  - Alarm Negative Test
  - Fault Relay Test
- Left Power Test disabled until the power-sense calibration is ready.
- Added INA240 current-based alarm validation on GPIO36 with a 0.5 ohm shunt.
- Added stricter fault relay validation:
  - original NC/NO state must be complementary,
  - fault state must be the stable inverted NC/NO state,
  - restore state must be stable before passing.
- Added operator feedback:
  - READY LED slow flashes when no DUT is present,
  - READY LED is solid when a DUT is detected,
  - PASS and FAIL LEDs alternate while testing,
  - short beep on DUT insertion,
  - short beep on test start,
  - double beep on PASS or FAIL.
- Added TEST button long-hold retest support after PASS or FAIL.
- Cleaned serial output so test sections and results are easier to read.
- Updated firmware version to `0.7.0`.

## Alpha Checkpoint

- Commit `431f90f` preserved the hardware bring-up state before final cleanup.
- Commit `60483eb` preserved the earlier INA and hardware-test bring-up milestone.
