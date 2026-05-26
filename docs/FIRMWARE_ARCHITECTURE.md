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

- The firmware waits for the TEST pushbutton on GPIO16.
- The TEST pushbutton uses an external pull-up and reads LOW when pressed.

When TEST is pressed:

- The state changes to `TEST_RUNNING`.
- `runTests()` is called.
- The current implementation delegates to `TestManager::runAllTests()`.

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
| `P` | Force the `PASS` state for debug. |
| `F` | Force the `FAIL` state for debug. |

Serial-triggered test results and forced PASS/FAIL states are held on the indicators until `R` is sent. This allows bench testing with no DUT fitted. Physical button-triggered test results still clear when DUT removal is detected.

Example serial output:

```text
================================
 STR-MON Automated Test Jig
 Firmware version: 0.2.0
 Simulation mode: ON
 Development mode: ON
 Serial baud: 115200
================================
[120 ms] INFO: Test manager initialized
[121 ms] INFO: System ready. Send H for serial command help.
Command received: T
[8020 ms] STATE: IDLE -> TEST_RUNNING | Serial command T
[8021 ms] INFO: Starting automated test sequence
[8272 ms] TEST: Power Test -> PASS
[8523 ms] TEST: Alarm Test -> PASS
[8774 ms] TEST: Fault Relay Test -> PASS
[8775 ms] INFO: Automated test sequence complete
[8776 ms] STATE: TEST_RUNNING -> PASS | Automated test sequence passed
```

## Debugging Workflow

Recommended early bench workflow:

1. Open the PlatformIO serial monitor at 115200 baud.
2. Press reset on the ESP32 and confirm the boot banner appears.
3. Send `H` to confirm the command parser is active.
4. Send `S` to inspect the current state and raw input levels.
5. Send `T` to run the simulated test sequence without a DUT.
6. Send `P` or `F` to check the PASS and FAIL indicators.
7. Send `R` to return to `IDLE`.

## Pin Assignments

| Function | Pin | Direction | Notes |
| --- | --- | --- | --- |
| `PIN_FLT_NO` | GPIO22 | Input | Fault relay NO contact. HIGH means DUT present. |
| `PIN_FLT_NC` | GPIO23 | Input | Fault relay NC contact. HIGH means DUT present. |
| TEST pushbutton | GPIO16 | Input | External pull-up. LOW means pressed. |
| Power sense | GPIO36 | Input | Future ADC power measurement. |
| Alarm sense | GPIO39 | Input | Future ADC/alarm measurement. |
| Fault input | GPIO32 | Input | Active LOW fault input. Use internal pull-up. |

| Alarm test low | GPIO25 | Output | Future relay/control output. |
| Alarm test high | GPIO33 | Output | Future relay/control output. |
| EOL open-circuit test | GPIO26 | Output | Future relay/control output. |
| EOL short-circuit test | GPIO27 | Output | Future relay/control output. |
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
