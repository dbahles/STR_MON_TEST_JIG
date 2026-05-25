#pragma once

// Project-wide configuration values.

#define SERIAL_BAUDRATE 115200

// Set to 0 when real hardware test inputs are implemented.
#define SIMULATION_MODE 1

#define DUT_MIN_VOLTAGE 22.0f
#define DUT_MAX_VOLTAGE 28.0f

#define ADC_SAMPLE_COUNT 10

#define TEST_TIMEOUT_MS 1000UL
#define START_DELAY_MS 2000UL
#define LOOP_DELAY_MS 50UL
