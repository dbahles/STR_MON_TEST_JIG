# Serial Protocol

Serial port settings:

- Baud: `115200`
- Line ending: newline / LF is recommended
- Commands are case-insensitive in the main firmware unless noted

## Main Jig Firmware

Environment: `esp32doit-devkit-v1`

| Command | Action |
| --- | --- |
| `H` or `?` | Print command help. |
| `S` | Print current state, DUT detect status, fault inputs, power/current ADC values, TEST switch state, and manual alarm output state. |
| `L` | List the active automated test sequence. |
| `T` | Start the automated test sequence when development commands are enabled. |
| `R` | Reset the jig state back to `IDLE` and clear outputs. |
| `M` | Toggle manual test mode. In manual mode each TEST switch press or `N` runs one test step. |
| `N` | Run the next manual test step. |
| `A` | Turn manual alarm positive output on. |
| `B` | Turn manual alarm negative output on. |
| `O` | Turn manual alarm outputs off. |

## Hardware Test Firmware

Environment: `hardware-test`

This firmware is for direct jig IO checks without the normal state machine.

Example output commands:

- `D19_H` sets GPIO19 HIGH.
- `D19_L` sets GPIO19 LOW.

Use `S` or `STATUS` to print input states. Inputs are reported in the form `D32 H` or `D32 L`.

## INA Test Firmware

Environment: `ina-test`

This firmware isolates the INA240 / GPIO36 ADC path and gives manual control of selected alarm test outputs.

| Command | Action |
| --- | --- |
| `S` | Take one INA/current reading. |
| `C` | Take a longer averaged calibration reading. |
| `Z` | Capture the present voltage as zero-current offset. |
| `K <mA>` | Calculate an effective gain from a known current. |
| `P` | Scan the available ADC pins and print raw/voltage readings. |
| `A` | Toggle automatic 1 second readings. |
| `+` | Toggle alarm positive test output GPIO33. |
| `O` | Toggle EOL open-circuit output GPIO26. |
| `X` | Turn all INA-test outputs off. |
| `H` or `?` | Print command help. |

## Notes

- The current INA sense input is GPIO36.
- GPIO36 is a GPIO number, not necessarily physical header pin number 36.
- Do not connect an analog signal to the ESP32 `EN` pin; pulling or driving `EN` can reset the module and make the serial port appear to lock up.
