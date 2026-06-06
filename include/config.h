#pragma once

// Project-wide configuration values.

#define SERIAL_BAUDRATE 115200
#define FIRMWARE_VERSION "0.6.0"

// Set to 1 only when running without real hardware attached.
#define SIMULATION_MODE 0

// Serial debug commands are enabled while the jig is being developed.
#define DEVELOPMENT_MODE 1

// Temporary hardware bring-up switches. Set these back to 1 when the
// corresponding power/alarm sense hardware is connected again.
#define ENABLE_POWER_TEST 0
#define ENABLE_ALARM_OUTPUT_TESTS 1
#define ENABLE_AUTO_DUT_STATE_MACHINE 1

#define DUT_MIN_VOLTAGE 22.0f
#define DUT_MAX_VOLTAGE 28.0f

// ADC conversion values. These are calibration placeholders until final hardware is measured.
#define ADC_REFERENCE_VOLTAGE 3.3f
#define ADC_MAX_READING 4095.0f
#define DUT_POWER_ADC_SCALE 11.0f
#define INA240_GAIN 20.0f
#define INA240_SHUNT_OHMS 0.5f
#define INA240_ZERO_CURRENT_V 0.0f

#define ADC_SAMPLE_COUNT 32

#define TEST_TIMEOUT_MS 1000UL
#define TEST_STEP_DELAY_MS 0UL
#define TEST_OUTPUT_VISIBLE_ON_MS 500UL
#define TEST_OUTPUT_VISIBLE_OFF_MS 500UL
#define FAULT_RELAY_TIMEOUT_MS 2000UL
#define FAULT_RELAY_POLL_MS 10UL
#define DUT_DEBOUNCE_MS 200UL
#define TEST_BUTTON_DEBOUNCE_MS 50UL
#define TEST_RESTART_LOCKOUT_MS 1500UL
#define BUZZER_CONFIRM_MS 75UL
#define LOOP_DELAY_MS 50UL

#define ALARM_TEST_DURATION_MS 1500UL
#define ALARM_SETTLE_DELAY_MS 500UL
#define ALARM_INTERTEST_DELAY_MS 500UL
#define ALARM_CURRENT_SETTLE_MS 1000UL
#define ALARM_CURRENT_INCREASE_MIN_MA 25.0f
#define ALARM_CURRENT_INCREASE_MAX_MA 70.0f
#define ALARM_OC_CURRENT_DROP_MIN_MA 10.0f
#define ALARM_OC_CURRENT_DROP_MAX_MA 20.0f
#define ALARM_CURRENT_RESTORE_TOLERANCE_MA 8.0f

#define ALARM_SENSE_MIN_V 2.5f
#define ALARM_SENSE_MAX_V 3.3f
#define ALARM_ADC_SAMPLES 10
