# IO Mapping

This table uses ESP32 GPIO numbers, not physical dev-module header pin numbers.

## Inputs

| Firmware name | GPIO | Purpose | Notes |
| --- | ---: | --- | --- |
| `PIN_TEST_SW` | 16 | TEST pushbutton | External pull-up. LOW when pressed. |
| `PIN_FLT_IN` | 32 | Fault input | Active LOW. Internal pull-up enabled in firmware. |
| `PIN_PWR_SENSE` | 36 | INA240 current sense ADC | GPIO36 / ADC input only. Used with 0.5 ohm shunt. |
| `PIN_FLT_NC` | 23 | Fault relay NC input | HIGH contributes to DUT-present detection. |
| `PIN_FLT_NO` | 22 | Fault relay NO input | HIGH contributes to DUT-present detection. |

## Outputs

| Firmware name | GPIO | Purpose | Notes |
| --- | ---: | --- | --- |
| `PIN_ALM_TEST_L` | 25 | Alarm negative test output | Driven during alarm negative test. |
| `PIN_ALM_TEST_H` | 33 | Alarm positive test output | Driven during alarm positive test. |
| `PIN_EOL_OC_TEST` | 26 | EOL open-circuit test output | Driven HIGH during open-circuit test. |
| `PIN_EOL_SC_TEST` | 27 | EOL short-circuit test output | Driven HIGH during short-circuit test. |
| `PIN_LED_RDY` | 21 | READY LED | On in `READY`. |
| `PIN_LED_PASS` | 19 | PASS LED | On in `PASS`. |
| `PIN_LED_FAIL` | 18 | FAIL LED | On in `FAIL`. |
| `PIN_BUZZER` | 17 | Buzzer | Short beep on DUT insertion. |

## Current Duplicate Check

No GPIO number is intentionally shared in the current mapping.

## Bring-Up Notes

- GPIO numbers are firmware GPIO numbers, not physical dev-module header pin numbers.
- The INA240 output must go to GPIO36. Do not wire it to `EN`; driving `EN` can reset the ESP32 and make serial communications appear faulty.
- GPIO36 is input-only, which is fine for the INA240 analog output.
