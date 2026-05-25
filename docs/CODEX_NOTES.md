You are helping build firmware for an ESP32-based automated production test jig called STR-MON-TESTJIG.

Development environment:

* PlatformIO
* VSCode
* Arduino framework
* ESP32 Dev Module

Project goals:

* Clean modular firmware architecture
* Expandable automated test framework
* Detailed serial logging
* State-machine-based workflow
* Simulation mode support before hardware arrives

Current project structure:

STR_MON_TEST_JIG/
├── docs/
├── include/
├── lib/
├── logs/
├── scripts/
├── src/
├── test/
└── platformio.ini

Please help generate the initial firmware skeleton using best-practice embedded C++ structure while keeping the code beginner-friendly and easy to maintain.

Requirements:

* Use modular .cpp and .h files
* Avoid over-engineering
* Use Arduino framework APIs
* Use enums for states and test results
* Use a central state machine
* Include simulation mode support
* All serial logging should go through a logger module
* Code should compile cleanly on ESP32

Please generate the following files with initial working implementations:

FILES:

* src/main.cpp
* src/test_manager.cpp
* src/serial_logger.cpp
* src/led_manager.cpp
* include/config.h
* include/pinmap.h
* include/system_state.h
* include/test_defs.h
* include/test_manager.h
* include/serial_logger.h
* include/led_manager.h

FUNCTIONAL REQUIREMENTS:

1. On startup:

   * Start serial at 115200
   * Print boot banner
   * Initialize LEDs
   * Enter IDLE state

2. State machine shall include:

   * BOOT
   * SELF_TEST
   * IDLE
   * WAIT_FOR_START
   * RUN_TESTS
   * PASS
   * FAIL
   * ERROR_STATE

3. Implement a fake test sequence in simulation mode:

   * Power Test
   * Alarm Test
   * Fault Relay Test

4. Test manager shall:

   * Run tests sequentially
   * Return PASS/FAIL
   * Log all results

5. Logger module shall support:

   * Info messages
   * Error messages
   * Test result messages

6. LED manager shall support:

   * Ready LED
   * Pass LED
   * Fail LED

7. Add placeholders for future:

   * ADC manager
   * Relay control
   * Calibration system

Coding style requirements:

* Keep functions small
* Use clear naming
* Add comments
* Avoid dynamic memory
* Keep architecture scalable

Please generate complete starter code for all files.
