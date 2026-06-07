# Serial Protocol

Serial port settings:

- Baud: `115200`
- Line ending: newline / LF is recommended
- Commands are case-insensitive

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

## Notes

- The current INA sense input is GPIO36.
- GPIO36 is a GPIO number, not necessarily physical header pin number 36.
- Do not connect an analog signal to the ESP32 `EN` pin; pulling or driving `EN` can reset the module and make the serial port appear to lock up.
