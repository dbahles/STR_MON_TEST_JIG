#pragma once

// ESP32 Dev Module pin assignments.

// Inputs
#define PIN_TEST_SW 16
#define PIN_FLT_IN 32  // this is an input sw 0v when in fault needs internal pullup enabled

#define PIN_PWR_SENSE 36
#define PIN_FLT_NC 23
#define PIN_FLT_NO 22

// Outputs
#define PIN_ALM_TEST_L 25
#define PIN_ALM_TEST_H 33
#define PIN_EOL_OC_TEST 26
#define PIN_EOL_SC_TEST 27

#define PIN_LED_RDY 21
#define PIN_LED_PASS 19
#define PIN_LED_FAIL 18

#define PIN_BUZZER 17
