# STR-MON Test Jig Firmware Architecture

## Current State Machine

The firmware is built around a simple state machine in `src/main.cpp`.

Current states:

| State | Purpose |
| --- | --- |
| `IDLE` | No DUT is fitted. READY indicator slowly flashes. |
| `READY` | A DUT has been detected and the jig is waiting for the TEST pushbutton. READY indicator is solid. |
| `TEST_RUNNING` | Automated tests are running. PASS and FAIL indicators alternate. |
| `PASS` | The test sequence passed. The PASS indicator stays on until the DUT is removed. |
| `FAIL` | The test sequence failed or returned an error. The FAIL indicator stays on until the DUT is removed. |
| `ERROR_STATE` | Reserved for future unrecoverable jig errors. |

Basic flow:

```text
IDLE
  -> READY when a DUT is inserted
READY
  -> TEST_RUNNING when TEST is pressed
  -> IDLE if the DUT is removed before testing
TEST_RUNNING
  -> PASS or FAIL after runTests() completes
PASS / FAIL
  -> IDLE when the DUT is removed
```

## DUT Detection Logic

DUT presence uses the fault relay contact inputs:

| Signal | Pin |
| --- | --- |
| `PIN_FLT_NO` | GPIO22 |
| `PIN_FLT_NC` | GPIO23 |

The DUT is considered inserted when either `PIN_FLT_NO` or `PIN_FLT_NC` is HIGH.

The DUT is considered removed only when both `PIN_FLT_NO` and `PIN_FLT_NC` are LOW.

The raw detect state must remain stable for approximately 200 ms before the firmware accepts it as inserted or removed. This avoids false transitions caused by contact bounce or noisy wiring during insertion/removal.

## Operator Behaviour

On confirmed DUT insertion:

- The state changes from `IDLE` to `READY`.
- The READY indicator turns on.
- The buzzer gives a short confirmation beep.

In `READY`:

- The firmware waits for the TEST pushbutton on GPIO16.
- The TEST pushbutton uses an external pull-up and reads LOW when pressed.
- The firmware requires a fresh debounced button press event. If the button input is already LOW during reset, the test will not start until the switch is released and pressed again.

When TEST is pressed:

- The state changes to `TEST_RUNNING`.
- The buzzer gives a short start beep.
- `runTests()` is called.
- The current implementation delegates to `TestManager::runAllTests()`.
- The active automated sequence runs open-circuit, short-circuit, alarm positive, alarm negative, and fault relay checks. The power test is still available but currently disabled during hardware bring-up.

After testing:

- A passing result changes the state to `PASS`.
- Any non-pass result changes the state to `FAIL`.
- The buzzer gives a short double beep for the final result.
- The PASS or FAIL indicator remains on until the DUT is removed.
- If the DUT remains fitted, the operator can release TEST, then hold TEST for 1.5 seconds to return to `READY` and run the test again without removing the DUT.

On confirmed DUT removal:

- READY, PASS, and FAIL indicators are turned off.
- The firmware returns to `IDLE`.

## Serial Debug Interface

Serial debugging runs at 115200 baud. The boot banner prints the firmware version, simulation mode, development mode, and baud rate.

In development mode, serial commands work regardless of the physical DUT inputs. Commands are case-insensitive.

| Command | Action |
| --- | --- |
| `T` | Start the test sequence from the current state. |
| `M` | Enable manual test stepping. A DUT must already be detected. |
| `N` | Run the next manual test step. The TEST pushbutton does the same while manual mode is enabled. |
| `R` | Reset the state machine to `IDLE`. |
| `S` | Print the current system status. |
| `H` | Print the help menu. |
| `L` | List the automated test sequence in run order. |
| `P` | Force the `PASS` state for debug. |
| `F` | Force the `FAIL` state for debug. |

Serial-triggered test results and forced PASS/FAIL states are held on the indicators until `R` is sent. This allows bench testing with no DUT fitted. Physical button-triggered test results still clear when DUT removal is detected.

Manual test mode is intended for first-hardware validation. Fit the DUT, wait for `READY`, send `M`, then press the TEST pushbutton or send `N` to run one test at a time. After each passing step the jig returns to `READY` and prints the next test name. A failed, timed-out, or errored step moves the jig to `FAIL`; when all manual steps pass, the jig moves to `PASS`.

## Baseline Power Test

The baseline DUT power check is currently disabled with `ENABLE_POWER_TEST 0`.

Power test sequence:

1. Read `PIN_PWR_SENSE` / GPIO36 using averaged ADC samples.
2. Convert the ADC reading to an estimated DUT voltage.
3. Confirm the voltage is between `DUT_MIN_VOLTAGE` and `DUT_MAX_VOLTAGE`.
4. Log the measured voltage.
5. Return `PASS` only if the voltage is in range.

Current configurable values:

| Setting | Current value |
| --- | ---: |
| `DUT_MIN_VOLTAGE` | 22.0 V |
| `DUT_MAX_VOLTAGE` | 28.0 V |
| `ADC_SAMPLE_COUNT` | 32 samples |
| `ADC_REFERENCE_VOLTAGE` | 3.3 V |
| `DUT_POWER_ADC_SCALE` | 11.0 |

`DUT_POWER_ADC_SCALE` is a provisional voltage-divider scale factor. It must be checked and calibrated when final hardware is available.

## Automated Open And Short Tests

The open-circuit and short-circuit tests live in `TestManager`.

Open-circuit test:

1. Read the original fault relay input state from `PIN_FLT_NO` and `PIN_FLT_NC`.
2. Confirm the original NC/NO state is complementary.
3. Set `PIN_EOL_OC_TEST` / GPIO26 HIGH.
4. Poll the relay inputs until they reach the inverted complementary state and remain stable briefly.
5. Fail with `TIMEOUT` if the inverted state is not detected within 2 seconds.
6. Set GPIO26 LOW.
7. Poll until the relay inputs return to the original state and remain stable briefly.
8. Return `PASS` only if both the transition and restore checks succeed.

Short-circuit test:

1. Read the original fault relay input state from `PIN_FLT_NO` and `PIN_FLT_NC`.
2. Confirm the original NC/NO state is complementary.
3. Set `PIN_EOL_SC_TEST` / GPIO27 HIGH.
4. Poll the relay inputs until they reach the inverted complementary state and remain stable briefly.
5. Fail with `TIMEOUT` if the inverted state is not detected within 2 seconds.
6. Set GPIO27 LOW.
7. Poll until the relay inputs return to the original state and remain stable briefly.
8. Return `PASS` only if both the transition and restore checks succeed.

The polling loop uses short 10 ms waits between input reads. This keeps the code easy to follow while avoiding long fixed delays.

## Automated Alarm Tests

The alarm positive and alarm negative tests live in `TestManager`. The firmware now uses the INA240 current-sense path on GPIO36 instead of the old alarm-sense voltage check.

Configurable values:

| Setting | Current value |
| --- | ---: |
| `ALARM_CURRENT_SETTLE_MS` | 1000 ms |
| `ALARM_INTERTEST_DELAY_MS` | 500 ms |
| `ALARM_CURRENT_INCREASE_MIN_MA` | 25.0 mA |
| `ALARM_CURRENT_INCREASE_MAX_MA` | 70.0 mA |
| `ALARM_OC_CURRENT_DROP_MIN_MA` | 10.0 mA |
| `ALARM_OC_CURRENT_DROP_MAX_MA` | 20.0 mA |
| `ALARM_CURRENT_RESTORE_TOLERANCE_MA` | 8.0 mA |

Alarm positive test:

1. Clear the alarm output and EOL open-circuit output.
2. Wait for current to settle and read the baseline current.
3. Set `PIN_ALM_TEST_H` / GPIO33 HIGH.
4. Wait for current to settle and confirm the current increased by the configured alarm-current range.
5. Set `PIN_EOL_OC_TEST` / GPIO26 HIGH while the alarm output remains active.
6. Wait for current to settle and confirm the current drops by the configured OC-drop range.
7. Clear GPIO26 and confirm alarm current restores.
8. Clear GPIO33 and confirm current returns near baseline.

Alarm negative test:

Same current-delta flow as alarm positive, but using `PIN_ALM_TEST_L` / GPIO25.

The alarm output and EOL open-circuit output are always switched LOW before the test returns.

Example serial output:

```text
================================
 STR-MON Automated Test Jig
 Firmware version: 0.7.0
 Simulation mode: OFF
 Development mode: ON
 Serial baud: 115200
================================
  Info: Test manager initialized
  Info: System ready. Send H for serial command help.
Command received: T
Starting test run: Serial command T
State: IDLE -> TEST_RUNNING | Serial command T
  Info: Starting automated test sequence

--------------------------------
Test: Open Circuit Test
  Original fault relay state: NC=LOW NO=HIGH
  Info: Setting GPIO26 HIGH to simulate open circuit fault
  Fault relay inverted: NC=HIGH NO=LOW
  Info: Setting GPIO26 LOW and checking relay restore
  Fault relay restored: NC=LOW NO=HIGH
  Result: PASS

--------------------------------
Test: Short Circuit Test
  Original fault relay state: NC=LOW NO=HIGH
  Info: Setting GPIO27 HIGH to simulate short circuit fault
  Fault relay inverted: NC=HIGH NO=LOW
  Info: Setting GPIO27 LOW and checking relay restore
  Fault relay restored: NC=LOW NO=HIGH
  Result: PASS

--------------------------------
Test: Alarm Positive Test
  Baseline current: 0.00 mA
  Info: GPIO33 HIGH
  Alarm active current: 40.00 mA
  Alarm current increase: 40.00 mA
  Info: Setting GPIO26 HIGH to apply open-circuit fault during alarm
  Alarm + OC fault current: 27.00 mA
  OC fault current drop: 13.00 mA
  Info: Setting GPIO26 LOW and checking alarm current restores
  Alarm restored current: 40.00 mA
  Info: GPIO33 LOW
  Alarm off final current: 0.00 mA
  Result: PASS

--------------------------------
Test: Alarm Negative Test
  Result: PASS

--------------------------------
Test: Fault Relay Test
  Fault relay contact state: NC=LOW NO=HIGH
  Result: PASS
  Info: Automated test sequence complete
State: TEST_RUNNING -> PASS | Automated test sequence passed
```

## Debugging Workflow

Recommended early bench workflow:

1. Open the PlatformIO serial monitor at 115200 baud.
2. Press reset on the ESP32 and confirm the boot banner appears.
3. Send `H` to confirm the command parser is active.
4. Send `L` to confirm the test order.
5. Send `S` to inspect the current state and raw input levels.
6. With real hardware fitted, send `M` to enable manual stepping.
7. Press TEST or send `N` to run one test at a time.
8. Send `T` to run the full automated sequence when manual validation looks good.
9. Send `P` or `F` to check the PASS and FAIL indicators.
10. Send `R` to return to `IDLE`.

## Pin Assignments

| Function | Pin | Direction | Notes |
| --- | --- | --- | --- |
| `PIN_FLT_NO` | GPIO22 | Input | Fault relay NO contact. HIGH means DUT present. |
| `PIN_FLT_NC` | GPIO23 | Input | Fault relay NC contact. HIGH means DUT present. |
| TEST pushbutton | GPIO16 | Input | External pull-up. LOW means pressed. |
| INA current sense | GPIO36 | Input | INA240 output, ADC input only. |
| Fault input | GPIO32 | Input | Active LOW fault input. Uses internal pull-up. |

| Alarm test low / negative | GPIO25 | Output | Alarm negative test output. |
| Alarm test high / positive | GPIO33 | Output | Alarm positive test output. |
| EOL open-circuit test | GPIO26 | Output | Driven HIGH during open-circuit test. |
| EOL short-circuit test | GPIO27 | Output | Driven HIGH during short-circuit test. |
| READY LED | GPIO21 | Output | On in `READY`. |
| PASS LED | GPIO19 | Output | On in `PASS`. |
| FAIL LED | GPIO18 | Output | On in `FAIL`. |
| Buzzer | GPIO17 | Output | Short beep on DUT insertion. |

The pin numbers in this table are ESP32 GPIO numbers, not physical dev-module header pin numbers.

## Future Expansion Strategy

The firmware is intentionally split into small modules:

- `main.cpp` owns the state machine and high-level workflow.
- `LedManager` owns LED outputs.
- `Buzzer` owns buzzer output.
- `TestManager` owns the automated test sequence.
- Future modules such as ADC, relay control, and calibration should hide hardware details behind simple functions.

When adding new tests, prefer this pattern:

1. Add the low-level hardware action or measurement to the correct hardware module.
2. Add one focused test function in `TestManager`.
3. Call that test from `TestManager::runAllTests()`.
4. Keep state transitions in `main.cpp`, not inside individual tests.

This keeps the state machine easy to read while allowing the hardware and tests to grow safely over time.
