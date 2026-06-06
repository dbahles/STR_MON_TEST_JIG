# STR-MON Test Jig Documentation Pack

---

# Project Overview

Project: STR-MON Automated Test Jig
Controller: ESP32
Framework: Arduino Framework (PlatformIO Recommended)
Version: 0.1 Draft
Date: 25/05/2026

This document pack defines the initial firmware, testing, operator, and architecture requirements for the STR-MON automated test jig.

The purpose of the test jig is to automate electrical and functional verification of STR-MON assemblies during production and servicing.

---

# Recommended Project Structure

```text
STR-MON-TESTJIG/
│
├── docs/
│   ├── 01_Functional_Design_Spec.md
│   ├── 02_Test_Matrix.md
│   ├── 03_IO_Mapping.md
│   ├── 04_Serial_Protocol.md
│   ├── 05_Error_Codes.md
│   ├── 06_Firmware_Architecture.md
│   ├── 07_Operator_Guide.md
│   ├── 08_Calibration_Spec.md
│   └── CHANGELOG.md
│
├── src/
├── include/
├── lib/
├── test/
└── platformio.ini
```

---

# 01_Functional_Design_Spec.md

## 1. Purpose

The STR-MON Test Jig is designed to automate functional verification of STR-MON assemblies using an ESP32-based control system.

The system shall:

* Automate electrical testing
* Simulate alarm and fault conditions
* Measure DUT response
* Provide operator feedback
* Generate detailed serial logs
* Improve production consistency
* Reduce manual testing time

---

## 2. System Overview

### Main Components

* ESP32 Controller
* Relay Simulation Circuitry
* INA240 Current Sense Amplifier
* DUT Interface Connector
* Operator Push Button
* Ready / Pass / Fail LEDs
* Audible Buzzer
* Mechanical Clamp Fixture

---

## 3. Operator Workflow

1. Operator inserts DUT into fixture
2. Operator closes clamp
3. Ready LED illuminates
4. Operator presses TEST button
5. Firmware executes automated test sequence
6. PASS or FAIL indication displayed
7. Detailed serial log transmitted
8. Operator removes DUT

---

## 4. Firmware Startup Behaviour

On power-up firmware shall:

* Initialize GPIO
* Initialize ADC systems
* Initialize serial interface
* Verify all relay outputs OFF
* Verify power rails
* Perform self-test
* Enter IDLE state

---

## 5. State Machine

```text
BOOT
 ↓
SELF_TEST
 ↓
IDLE
 ↓
WAIT_FOR_START
 ↓
RUN_TESTS
 ↓
PASS / FAIL
 ↓
RESULT_DISPLAY
 ↓
RETURN_TO_IDLE
```

---

## 6. Test Sequence Overview

The firmware shall support:

* Power verification
* Alarm output testing
* Open circuit simulation
* Short circuit simulation
* Fault relay verification
* Current monitoring
* ADC threshold checking

---

## 7. Serial Logging

The firmware shall generate detailed serial logs including:

* Test name
* Measured values
* Expected values
* Pass/fail status
* Timing information
* Error codes

Example:

```text
[TEST] PowerVerification
Measured Voltage: 24.18V
Result: PASS
```

---

# 02_Test_Matrix.md

# STR-MON Test Matrix

| Test ID | Test Name      | Description                | Expected Result    | Pass Criteria        | Timeout |
| ------- | -------------- | -------------------------- | ------------------ | -------------------- | ------- |
| T001    | DUT Power      | Verify DUT voltage         | Valid voltage      | 22V - 28V            | 500ms   |
| T002    | Alarm Low      | Activate AL_IP_LOW         | Relay activates    | Input detected       | 1s      |
| T003    | Alarm High     | Activate AL_IP_HIGH        | Relay activates    | Input detected       | 1s      |
| T004    | Open Circuit   | Simulate OC fault          | DUT fault detected | Correct fault output | 1s      |
| T005    | Short Circuit  | Simulate SC fault          | DUT fault detected | Correct fault output | 1s      |
| T006    | Fault Relay NO | Verify NO state            | Relay state valid  | GPIO correct         | 500ms   |
| T007    | Fault Relay NC | Verify NC state            | Relay state valid  | GPIO correct         | 500ms   |
| T008    | Current Sense  | Verify current measurement | Valid current      | Within range         | 1s      |

---

## Future Tests

* Temperature compensation
* Timing verification
* Long-duration soak test
* Relay cycle count
* ADC calibration verification

---

# 03_IO_Mapping.md

# ESP32 IO Mapping

| ESP32 Pin | Signal Name | Direction | Function                 |
| --------- | ----------- | --------- | ------------------------ |
| GPIO36    | PWR_SENSE   | ADC Input | DUT voltage monitor      |
| GPIO16    | TEST_SW     | Input     | Start button             |
| GPIO32    | FLT_IN      | Input     | Active LOW fault input   |
| GPIO25    | ALM_TEST_L  | Output    | Alarm negative drive     |
| GPIO33    | ALM_TEST_H  | Output    | Alarm positive drive     |
| GPIO26    | EOL_OC_TEST | Output    | Open circuit simulation  |
| GPIO27    | EOL_SC_TEST | Output    | Short circuit simulation |
| GPIO21    | LED_RDY     | Output    | Ready LED                |
| GPIO19    | LED_PASS    | Output    | Pass LED                 |
| GPIO18    | LED_FAIL    | Output    | Fail LED                 |
| GPIO17    | BUZZER      | Output    | Audible buzzer           |
| GPIO22    | FLT_NO      | Input     | Fault relay NO           |
| GPIO23    | FLT_NC      | Input     | Fault relay NC           |

---

## Notes

* ADC inputs should use averaging
* Relay outputs default OFF during boot
* GPIO reassignment must be updated here

---

# 04_Serial_Protocol.md

# Serial Communication Specification

## Serial Configuration

```text
115200 baud
8 data bits
No parity
1 stop bit
```

---

## Output Modes

### Human Readable Mode

```text
[TEST] OpenCircuit
Measured Voltage: 0.18V
Expected Fault: ACTIVE
Result: PASS
```

---

### JSON Mode

```json
{
  "test":"OpenCircuit",
  "value":0.18,
  "result":"PASS"
}
```

---

## Proposed Commands

| Command     | Description                |
| ----------- | -------------------------- |
| test run    | Execute full test sequence |
| adc read    | Dump ADC values            |
| relay on X  | Enable relay               |
| relay off X | Disable relay              |
| status      | Show system state          |
| cal start   | Start calibration          |
| version     | Show firmware version      |

---

## Future Expansion

* USB CDC support
* WiFi logging
* CSV export
* PC application integration

---

# 05_Error_Codes.md

# Error Code Reference

| Error Code | Description                     |
| ---------- | ------------------------------- |
| E100       | DUT not detected                |
| E101       | DUT voltage low                 |
| E102       | DUT voltage high                |
| E103       | ADC read failure                |
| E104       | Alarm test failed               |
| E105       | Open circuit simulation failed  |
| E106       | Short circuit simulation failed |
| E107       | Fault relay mismatch            |
| E108       | Test timeout                    |
| E109       | INA240 measurement invalid      |
| E110       | Relay output failure            |
| E111       | Firmware watchdog reset         |

---

## Example Serial Output

```text
[FAIL] E105
Open circuit simulation failed
```

---

# 06_Firmware_Architecture.md

# Firmware Architecture

## Design Goals

* Modular structure
* Expandable test system
* Stable operation
* Clear debugging
* Reusable test framework

---

## Recommended File Structure

```text
src/
│
├── main.cpp
├── test_manager.cpp
├── adc_manager.cpp
├── relay_control.cpp
├── serial_logger.cpp
├── calibration.cpp
├── led_manager.cpp
└── buzzer.cpp
```

---

## Core Modules

### main.cpp

* System startup
* State machine
* Main loop

### test_manager.cpp

* Executes test sequence
* Pass/fail management
* Timing control

### adc_manager.cpp

* ADC averaging
* Voltage conversion
* Filtering

### relay_control.cpp

* Relay control abstraction
* Output protection

### serial_logger.cpp

* Unified logging
* Debug formatting
* JSON generation

---

## Recommended Architecture

```text
MAIN LOOP
   ↓
STATE MACHINE
   ↓
TEST MANAGER
   ↓
INDIVIDUAL TEST FUNCTIONS
```

---

## Future Expansion

* WiFi support
* OTA updates
* SD card logging
* Remote diagnostics

---

# 07_Operator_Guide.md

# Operator Guide

## Normal Operation

1. Insert DUT into fixture
2. Close clamp securely
3. Verify Ready LED ON
4. Press TEST button
5. Wait for completion
6. Observe PASS or FAIL indicator
7. Remove DUT

---

## LED Indications

| LED   | Meaning                      |
| ----- | ---------------------------- |
| Ready | Jig ready for testing        |
| Pass  | DUT passed all tests         |
| Fail  | DUT failed one or more tests |

---

## Audible Indications

| Beep Pattern      | Meaning        |
| ----------------- | -------------- |
| Single short beep | PASS           |
| Triple short beep | FAIL           |
| Continuous beep   | Critical fault |

---

## Failure Handling

If DUT fails:

1. Record serial log
2. Remove DUT
3. Notify technician
4. Retest if required

---

# 08_Calibration_Spec.md

# Calibration Specification

## Purpose

Calibration ensures ADC measurements and current measurements remain accurate.

---

## Calibration Items

| Item           | Description               |
| -------------- | ------------------------- |
| Voltage Offset | ADC voltage correction    |
| Current Offset | INA240 current correction |
| ADC Scaling    | ADC gain correction       |

---

## Calibration Procedure

1. Connect known reference voltage
2. Enter calibration mode
3. Record measured value
4. Store correction factor
5. Save to NVS memory

---

## Proposed Calibration Commands

```text
cal start
cal voltage 24.00
cal current 0.500
cal save
```

---

## Storage Requirements

Calibration values shall be stored in:

* ESP32 NVS memory
* Non-volatile storage
* Loaded during startup

---

# CHANGELOG.md

# Changelog

## Version 0.1 - 25/05/2026

### Initial Draft

* Created documentation structure
* Added functional design specification
* Added initial test matrix
* Added IO mapping
* Added serial protocol draft
* Added firmware architecture recommendations
* Added calibration specification
* Added operator guide

---

# Notes

## Recommended Development Environment

Recommended setup:

* Visual Studio Code
* PlatformIO
* Arduino Framework
* ESP32 Platform

---

## Future Enhancements

Potential future features:

* Fixture closed detection switch
* Barcode scanner support
* Automated report generation
* WiFi result upload
* SD card logging
* Ethernet support
* PC manufacturing application
* Database integration

---

# Reference Material

## Hardware Reference

* STR-MON Test Jig Schematic V1
* Mechanical fixture layout
* ESP32 DEV Board
* INA240A1DR current sense amplifier

---

# End Of Documentation Pack


