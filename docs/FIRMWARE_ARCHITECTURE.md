# STR-MON Test Jig Firmware Architecture

## Current State Machine

The firmware is built around a simple state machine in `src/main.cpp`.

Current states:

| State | Purpose |
| --- | --- |
| `IDLE` | No DUT is fitted. READY, PASS, and FAIL indicators are off. |
| `READY` | A DUT has been detected and the jig is waiting for the TEST pushbutton. |
| `TEST_RUNNING` | Automated tests are running. Current tests are placeholder/simulation tests. |
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

- The firmware waits for the TEST pushbutton on GPIO32.
- The TEST pushbutton uses an external pull-up and reads LOW when pressed.
- The firmware requires a fresh debounced button press event. If the button input is already LOW during reset, the test will not start until the switch is released and pressed again.

When TEST is pressed:

- The state changes to `TEST_RUNNING`.
- `runTests()` is called.
- The current implementation delegates to `TestManager::runAllTests()`.
- The automated sequence now includes baseline power, alarm positive, alarm negative, open-circuit, and short-circuit checks.

After testing:

- A passing result changes the state to `PASS`.
- Any non-pass result changes the state to `FAIL`.
- The PASS or FAIL indicator remains on until the DUT is removed.

On confirmed DUT removal:

- READY, PASS, and FAIL indicators are turned off.
- The firmware returns to `IDLE`.

## Serial Debug Interface

Serial debugging runs at 115200 baud. The boot banner prints the firmware version, simulation mode, development mode, and baud rate.

In development mode, serial commands work regardless of the physical DUT inputs. Commands are case-insensitive.

| Command | Action |
| --- | --- |
| `T` | Start the test sequence from the current state. |
| `R` | Reset the state machine to `IDLE`. |
| `S` | Print the current system status. |
| `H` | Print the help menu. |
| `L` | List the automated test sequence in run order. |
| `P` | Force the `PASS` state for debug. |
| `F` | Force the `FAIL` state for debug. |

Serial-triggered test results and forced PASS/FAIL states are held on the indicators until `R` is sent. This allows bench testing with no DUT fitted. Physical button-triggered test results still clear when DUT removal is detected.

## Baseline Power Test

The first automated test is the baseline DUT power check.

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
| `ADC_SAMPLE_COUNT` | 10 samples |
| `ADC_REFERENCE_VOLTAGE` | 3.3 V |
| `DUT_POWER_ADC_SCALE` | 11.0 |

`DUT_POWER_ADC_SCALE` is a provisional voltage-divider scale factor. It must be checked and calibrated when final hardware is available.

## Automated Open And Short Tests

The open-circuit and short-circuit tests live in `TestManager`.

Open-circuit test:

1. Read the original fault relay input state from `PIN_FLT_NO` and `PIN_FLT_NC`.
2. Set `PIN_EOL_OC_TEST` / GPIO17 HIGH.
3. Poll the relay inputs until either input changes from the original state.
4. Fail with `TIMEOUT` if no relay transition is detected within 2 seconds.
5. Set GPIO17 LOW.
6. Poll until the relay inputs return to the original state.
7. Return `PASS` only if both the transition and restore checks succeed.

Short-circuit test:

1. Read the original fault relay input state from `PIN_FLT_NO` and `PIN_FLT_NC`.
2. Set `PIN_EOL_SC_TEST` / GPIO16 HIGH.
3. Poll the relay inputs until either input changes from the original state.
4. Fail with `TIMEOUT` if no relay transition is detected within 2 seconds.
5. Set GPIO16 LOW.
6. Poll until the relay inputs return to the original state.
7. Return `PASS` only if both the transition and restore checks succeed.

The polling loop uses short 10 ms waits between input reads. This keeps the code easy to follow while avoiding long fixed delays.

## Automated Alarm Tests

The alarm positive and alarm negative tests live in `TestManager` and use `AdcManager::readAlarmSenseVoltage()` for the GPIO39 alarm sense reading.

Configurable values:

| Setting | Current value |
| --- | ---: |
| `ALARM_TEST_DURATION_MS` | 1500 ms |
| `ALARM_SETTLE_DELAY_MS` | 500 ms |
| `ALARM_INTERTEST_DELAY_MS` | 500 ms |
| `ALARM_SENSE_MIN_V` | 2.5 V |
| `ALARM_SENSE_MAX_V` | 3.3 V |
| `ALARM_ADC_SAMPLES` | 10 samples |

Alarm positive test:

1. Read the original fault relay state from `PIN_FLT_NO` and `PIN_FLT_NC`.
2. Set `PIN_ALM_TEST_H` / GPIO19 HIGH.
3. Wait for the alarm settle time while confirming the fault relay state does not change.
4. Read the alarm sense voltage on GPIO39.
5. Confirm the voltage is between `ALARM_SENSE_MIN_V` and `ALARM_SENSE_MAX_V`.
6. Confirm the fault relay state is still unchanged.
7. Hold the output active for `ALARM_TEST_DURATION_MS` while monitoring that the fault relay remains unchanged.
8. Set GPIO19 LOW.
9. Wait `ALARM_INTERTEST_DELAY_MS`.
10. Return `PASS` only if all checks succeed.

Alarm negative test:

1. Read the original fault relay state from `PIN_FLT_NO` and `PIN_FLT_NC`.
2. Set `PIN_ALM_TEST_L` / GPIO18 HIGH.
3. Wait for the alarm settle time while confirming the fault relay state does not change.
4. Read the alarm sense voltage on GPIO39.
5. Confirm the voltage is between `ALARM_SENSE_MIN_V` and `ALARM_SENSE_MAX_V`.
6. Confirm the fault relay state is still unchanged.
7. Hold the output active for `ALARM_TEST_DURATION_MS` while monitoring that the fault relay remains unchanged.
8. Set GPIO18 LOW.
9. Wait `ALARM_INTERTEST_DELAY_MS`.
10. Return `PASS` only if all checks succeed.

The alarm output is always switched LOW before the test returns, including failure paths.

Example serial output:

```text
================================
 STR-MON Automated Test Jig
 Firmware version: 0.5.0
 Simulation mode: ON
 Development mode: ON
 Serial baud: 115200
================================
[120 ms] INFO: Test manager initialized
[121 ms] INFO: System ready. Send H for serial command help.
Command received: T
[8020 ms] STATE: IDLE -> TEST_RUNNING | Serial command T
[8021 ms] INFO: Starting automated test sequence
[8022 ms] INFO: Power baseline test started
DUT Power Voltage = 24.00 V
[8272 ms] TEST: Power Test -> PASS
[8523 ms] INFO: Alarm Positive Test Started
Initial fault relay state: NC=LOW NO=HIGH
[8524 ms] INFO: GPIO19 HIGH
Alarm Sense Voltage = 3.00 V
[10040 ms] INFO: Holding alarm output active
[11541 ms] INFO: GPIO19 LOW
[12042 ms] TEST: Alarm Positive Test -> PASS
[12043 ms] INFO: Alarm Negative Test Started
Initial fault relay state: NC=LOW NO=HIGH
[12044 ms] INFO: GPIO18 HIGH
Alarm Sense Voltage = 3.00 V
[13560 ms] INFO: Holding alarm output active
[15061 ms] INFO: GPIO18 LOW
[15562 ms] TEST: Alarm Negative Test -> PASS
[15563 ms] INFO: Starting open circuit test
Original fault relay state: NC=LOW NO=HIGH
[15564 ms] INFO: Setting IO17 HIGH to simulate open circuit fault
Fault relay changed: NC=HIGH NO=LOW
[15680 ms] INFO: Setting IO17 LOW and checking relay restore
Fault relay restored: NC=LOW NO=HIGH
[15690 ms] TEST: Open Circuit Test -> PASS
[15691 ms] INFO: Starting short circuit test
Original fault relay state: NC=LOW NO=HIGH
[15692 ms] INFO: Setting IO16 HIGH to simulate short circuit fault
Fault relay changed: NC=HIGH NO=LOW
[15800 ms] INFO: Setting IO16 LOW and checking relay restore
Fault relay restored: NC=LOW NO=HIGH
[15810 ms] TEST: Short Circuit Test -> PASS
[16061 ms] TEST: Fault Relay Test -> PASS
[16062 ms] INFO: Automated test sequence complete
[16063 ms] STATE: TEST_RUNNING -> PASS | Automated test sequence passed
```

## Debugging Workflow

Recommended early bench workflow:

1. Open the PlatformIO serial monitor at 115200 baud.
2. Press reset on the ESP32 and confirm the boot banner appears.
3. Send `H` to confirm the command parser is active.
4. Send `L` to confirm the test order.
5. Send `S` to inspect the current state and raw input levels.
6. Send `T` to run the simulated test sequence without a DUT.
7. Send `P` or `F` to check the PASS and FAIL indicators.
8. Send `R` to return to `IDLE`.

## Pin Assignments

| Function | Pin | Direction | Notes |
| --- | --- | --- | --- |
| `PIN_FLT_NO` | GPIO22 | Input | Fault relay NO contact. HIGH means DUT present. |
| `PIN_FLT_NC` | GPIO23 | Input | Fault relay NC contact. HIGH means DUT present. |
| TEST pushbutton | GPIO32 | Input | External pull-up. LOW means pressed. |
| Power sense | GPIO36 | Input | Future ADC power measurement. |
| Alarm sense | GPIO39 | Input | Future ADC/alarm measurement. |
| Fault input | GPIO21 | Input | Active LOW fault input. Uses internal pull-up. |

| Alarm test low / negative | GPIO18 | Output | Alarm negative test output. |
| Alarm test high / positive | GPIO19 | Output | Alarm positive test output. |
| EOL open-circuit test | GPIO17 | Output | Driven HIGH during open-circuit test. |
| EOL short-circuit test | GPIO16 | Output | Driven HIGH during short-circuit test. |
| READY LED | GPIO33 | Output | On in `READY`. |
| PASS LED | GPIO25 | Output | On in `PASS`. |
| FAIL LED | GPIO26 | Output | On in `FAIL`. |
| Buzzer | GPIO27 | Output | Short beep on DUT insertion. |

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
