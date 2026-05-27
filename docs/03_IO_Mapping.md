# IO Mapping

This table uses ESP32 GPIO numbers, not physical dev-module header pin numbers.

## Inputs

| Firmware name | GPIO | Purpose | Notes |
| --- | ---: | --- | --- |
| `PIN_TEST_SW` | 32 | TEST pushbutton | External pull-up. LOW when pressed. |
| `PIN_FLT_IN` | 21 | Fault input | Active LOW. Internal pull-up enabled in firmware. |
| `PIN_PWR_SENSE` | 36 | Power sense ADC | Future power measurement. |
| `PIN_ALM_SENSE` | 39 | Alarm sense ADC | Used for alarm output voltage test. |
| `PIN_FLT_NC` | 23 | Fault relay NC input | HIGH contributes to DUT-present detection. |
| `PIN_FLT_NO` | 22 | Fault relay NO input | HIGH contributes to DUT-present detection. |

## Outputs

| Firmware name | GPIO | Purpose | Notes |
| --- | ---: | --- | --- |
| `PIN_ALM_TEST_L` | 18 | Alarm negative test output | Driven during alarm negative test. |
| `PIN_ALM_TEST_H` | 19 | Alarm positive test output | Driven during alarm positive test. |
| `PIN_EOL_OC_TEST` | 17 | EOL open-circuit test output | Driven HIGH during open-circuit test. |
| `PIN_EOL_SC_TEST` | 16 | EOL short-circuit test output | Driven HIGH during short-circuit test. |
| `PIN_LED_RDY` | 33 | READY LED | On in `READY`. |
| `PIN_LED_PASS` | 25 | PASS LED | On in `PASS`. |
| `PIN_LED_FAIL` | 26 | FAIL LED | On in `FAIL`. |
| `PIN_BUZZER` | 27 | Buzzer | Short beep on DUT insertion. |

## Current Duplicate Check

No GPIO number is intentionally shared in the current mapping.
