#pragma once

// Project-wide configuration values.

#define SERIAL_BAUDRATE 115200
#define FIRMWARE_VERSION "0.4.0"

// Set to 0 when real hardware test inputs are implemented.
#define SIMULATION_MODE 1

// Serial debug commands are enabled while the jig is being developed.
#define DEVELOPMENT_MODE 1

#define DUT_MIN_VOLTAGE 22.0f
#define DUT_MAX_VOLTAGE 28.0f

#define ADC_SAMPLE_COUNT 10

#define TEST_TIMEOUT_MS 1000UL
#define FAULT_RELAY_TIMEOUT_MS 2000UL
#define FAULT_RELAY_POLL_MS 10UL
#define DUT_DEBOUNCE_MS 200UL
#define TEST_BUTTON_DEBOUNCE_MS 50UL
#define BUZZER_CONFIRM_MS 75UL
#define LOOP_DELAY_MS 50UL

#define ALARM_TEST_DURATION_MS 1500UL
#define ALARM_SETTLE_DELAY_MS 500UL
#define ALARM_INTERTEST_DELAY_MS 500UL

#define ALARM_SENSE_MIN_V 2.5f
#define ALARM_SENSE_MAX_V 3.3f
#define ALARM_ADC_SAMPLES 10
