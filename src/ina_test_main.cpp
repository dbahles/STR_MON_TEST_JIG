#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>

namespace
{
    constexpr uint8_t PIN_INA_SENSE = 36;
    constexpr uint8_t PIN_ALARM_POSITIVE = 33;
    constexpr uint8_t PIN_OC_FAULT = 26;
    constexpr uint8_t ADC_PINS[] = {32, 33, 34, 35, 36, 39};
    constexpr uint32_t BAUD_RATE = 115200;
    constexpr uint16_t ADC_MAX_READING = 4095;
    constexpr float ADC_REFERENCE_VOLTAGE = 3.3f;
    constexpr float INA240_GAIN = 20.0f;
    constexpr float SHUNT_OHMS = 0.5f;
    constexpr float ZERO_CURRENT_V = 0.0f;
    constexpr uint8_t SAMPLE_COUNT = 32;
    constexpr uint16_t CAL_SAMPLE_COUNT = 256;

    char commandBuffer[48];
    size_t commandLength = 0;
    bool autoReadEnabled = false;
    bool alarmPositiveOn = false;
    bool ocFaultOn = false;
    float capturedZeroVoltage = ZERO_CURRENT_V;
    bool zeroCaptured = false;
    unsigned long lastAutoReadAt = 0;

    float rawToVoltage(uint16_t raw)
    {
        return static_cast<float>(raw) * (ADC_REFERENCE_VOLTAGE / ADC_MAX_READING);
    }

    float voltageToMilliamps(float voltage, float zeroVoltage = ZERO_CURRENT_V)
    {
        return ((voltage - zeroVoltage) / (INA240_GAIN * SHUNT_OHMS)) * 1000.0f;
    }

    uint16_t readAveragedRaw(uint8_t pin, uint16_t sampleCount, uint16_t &minimum, uint16_t &maximum)
    {
        uint32_t total = 0;
        minimum = ADC_MAX_READING;
        maximum = 0;

        for (uint16_t index = 0; index < sampleCount; index++)
        {
            const uint16_t raw = analogRead(pin);
            total += raw;
            minimum = min(minimum, raw);
            maximum = max(maximum, raw);
            delay(2);
        }

        return static_cast<uint16_t>(total / SAMPLE_COUNT);
    }

    void printPinReading(uint8_t pin)
    {
        uint16_t minimum = 0;
        uint16_t maximum = 0;
        const uint16_t average = readAveragedRaw(pin, SAMPLE_COUNT, minimum, maximum);
        const float voltage = rawToVoltage(average);

        Serial.print("GPIO");
        Serial.print(pin);
        Serial.print(" raw avg=");
        Serial.print(average);
        Serial.print(" min=");
        Serial.print(minimum);
        Serial.print(" max=");
        Serial.print(maximum);
        Serial.print(" adc=");
        Serial.print(voltage, 3);
        Serial.print(" V current=");
        Serial.print(voltageToMilliamps(voltage), 1);
        if (zeroCaptured)
        {
            Serial.print(" mA zeroed=");
            Serial.print(voltageToMilliamps(voltage, capturedZeroVoltage), 1);
        }
        Serial.println(" mA");
    }

    void printOutputState()
    {
        Serial.print("Outputs GPIO");
        Serial.print(PIN_ALARM_POSITIVE);
        Serial.print(" alarm+=");
        Serial.print(alarmPositiveOn ? "HIGH" : "LOW");
        Serial.print(" GPIO");
        Serial.print(PIN_OC_FAULT);
        Serial.print(" oc_fault=");
        Serial.println(ocFaultOn ? "HIGH" : "LOW");
    }

    void setAlarmPositive(bool isOn)
    {
        alarmPositiveOn = isOn;
        digitalWrite(PIN_ALARM_POSITIVE, alarmPositiveOn ? HIGH : LOW);
        printOutputState();
    }

    void setOcFault(bool isOn)
    {
        ocFaultOn = isOn;
        digitalWrite(PIN_OC_FAULT, ocFaultOn ? HIGH : LOW);
        printOutputState();
    }

    void allOutputsOff()
    {
        alarmPositiveOn = false;
        ocFaultOn = false;
        digitalWrite(PIN_ALARM_POSITIVE, LOW);
        digitalWrite(PIN_OC_FAULT, LOW);
        printOutputState();
    }

    uint16_t readCalibrationRaw(float &voltage, uint16_t &minimum, uint16_t &maximum)
    {
        const uint16_t average = readAveragedRaw(PIN_INA_SENSE, CAL_SAMPLE_COUNT, minimum, maximum);
        voltage = rawToVoltage(average);
        return average;
    }

    void printCalibrationReading(const char *label)
    {
        uint16_t minimum = 0;
        uint16_t maximum = 0;
        float voltage = 0.0f;
        const uint16_t average = readCalibrationRaw(voltage, minimum, maximum);

        Serial.print(label);
        Serial.print(" raw avg=");
        Serial.print(average);
        Serial.print(" min=");
        Serial.print(minimum);
        Serial.print(" max=");
        Serial.print(maximum);
        Serial.print(" adc=");
        Serial.print(voltage, 4);
        Serial.print(" V current=");
        Serial.print(voltageToMilliamps(voltage), 2);
        if (zeroCaptured)
        {
            Serial.print(" mA zeroed=");
            Serial.print(voltageToMilliamps(voltage, capturedZeroVoltage), 2);
        }
        Serial.println(" mA");
    }

    void captureZero()
    {
        uint16_t minimum = 0;
        uint16_t maximum = 0;
        float voltage = 0.0f;
        const uint16_t average = readCalibrationRaw(voltage, minimum, maximum);

        capturedZeroVoltage = voltage;
        zeroCaptured = true;

        Serial.print("Zero captured raw=");
        Serial.print(average);
        Serial.print(" min=");
        Serial.print(minimum);
        Serial.print(" max=");
        Serial.print(maximum);
        Serial.print(" zero=");
        Serial.print(capturedZeroVoltage, 4);
        Serial.println(" V");
    }

    void calibrateKnownCurrent(float knownMilliamps)
    {
        if (!zeroCaptured)
        {
            Serial.println("Capture zero first with Z.");
            return;
        }

        uint16_t minimum = 0;
        uint16_t maximum = 0;
        float voltage = 0.0f;
        const uint16_t average = readCalibrationRaw(voltage, minimum, maximum);
        const float deltaVoltage = voltage - capturedZeroVoltage;
        const float measuredMilliamps = voltageToMilliamps(voltage, capturedZeroVoltage);
        const float effectiveGain = deltaVoltage / ((knownMilliamps / 1000.0f) * SHUNT_OHMS);

        Serial.print("Known current calibration raw=");
        Serial.print(average);
        Serial.print(" adc=");
        Serial.print(voltage, 4);
        Serial.print(" V delta=");
        Serial.print(deltaVoltage, 4);
        Serial.print(" V known=");
        Serial.print(knownMilliamps, 2);
        Serial.print(" mA measured=");
        Serial.print(measuredMilliamps, 2);
        Serial.print(" mA effective_gain=");
        Serial.print(effectiveGain, 2);
        Serial.println(" V/V");
    }

    void printReading()
    {
        printPinReading(PIN_INA_SENSE);
    }

    void printAllAdcPins()
    {
        Serial.println("ADC1 pin scan:");
        for (uint8_t index = 0; index < (sizeof(ADC_PINS) / sizeof(ADC_PINS[0])); index++)
        {
            printPinReading(ADC_PINS[index]);
        }
    }

    void printHelp()
    {
        Serial.println();
        Serial.print("INA240 GPIO");
        Serial.print(PIN_INA_SENSE);
        Serial.println(" isolation test");
        Serial.println("Commands:");
        Serial.println("  S = single ADC/current reading");
        Serial.println("  C = calibration averaged reading");
        Serial.println("  Z = capture zero-current offset");
        Serial.println("  K <mA> = compute effective gain from known current");
        Serial.println("  P = scan GPIO32, 33, 34, 35, 36, 39");
        Serial.println("  A = toggle 1 second auto-read");
        Serial.println("  + = toggle alarm positive output GPIO33");
        Serial.println("  O = toggle open-circuit fault output GPIO26");
        Serial.println("  X = all test outputs OFF");
        Serial.println("  H = help");
        Serial.println();
        Serial.println("Only GPIO33 alarm+ and GPIO26 OC fault are driven in this firmware.");
        Serial.println("GPIO33 and GPIO26 are forced LOW on boot.");
        Serial.println();
    }

    void handleCommandLine(char *line)
    {
        while (*line == ' ' || *line == '\t')
        {
            line++;
        }

        if (*line == '\0')
        {
            return;
        }

        const char command = static_cast<char>(toupper(*line));
        Serial.print("Command received: ");
        Serial.println(command);

        switch (command)
        {
        case 'S':
            printReading();
            break;

        case 'C':
            printCalibrationReading("Calibration");
            break;

        case 'Z':
            captureZero();
            break;

        case 'K':
            calibrateKnownCurrent(strtof(line + 1, nullptr));
            break;

        case 'P':
            printAllAdcPins();
            break;

        case 'A':
            autoReadEnabled = !autoReadEnabled;
            Serial.print("Auto-read ");
            Serial.println(autoReadEnabled ? "ON" : "OFF");
            break;

        case '+':
            setAlarmPositive(!alarmPositiveOn);
            break;

        case 'O':
            setOcFault(!ocFaultOn);
            break;

        case 'X':
            allOutputsOff();
            break;

        case 'H':
        case '?':
            printHelp();
            break;

        default:
            Serial.println("Unknown command. Send H for help.");
            break;
        }
    }

    void handleSerial()
    {
        while (Serial.available() > 0)
        {
            const char next = static_cast<char>(Serial.read());

            if (next == '\r' || next == '\n')
            {
                commandBuffer[commandLength] = '\0';
                handleCommandLine(commandBuffer);
                commandLength = 0;
                continue;
            }

            if (commandLength < (sizeof(commandBuffer) - 1))
            {
                commandBuffer[commandLength++] = next;
                Serial.write(next);
            }
            else
            {
                commandLength = 0;
                Serial.println();
                Serial.println("Command buffer full; cleared.");
            }
        }
    }

    void updateAutoRead()
    {
        if (!autoReadEnabled || (millis() - lastAutoReadAt) < 1000UL)
        {
            return;
        }

        lastAutoReadAt = millis();
        printReading();
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(300);

    pinMode(PIN_INA_SENSE, INPUT);
    pinMode(PIN_ALARM_POSITIVE, OUTPUT);
    pinMode(PIN_OC_FAULT, OUTPUT);
    allOutputsOff();

    analogReadResolution(12);
    analogSetPinAttenuation(PIN_INA_SENSE, ADC_11db);
    for (uint8_t index = 0; index < (sizeof(ADC_PINS) / sizeof(ADC_PINS[0])); index++)
    {
        pinMode(ADC_PINS[index], INPUT);
        analogSetPinAttenuation(ADC_PINS[index], ADC_11db);
    }

    Serial.println();
    Serial.println("================================");
    Serial.print(" INA240 GPIO");
    Serial.print(PIN_INA_SENSE);
    Serial.println(" Isolation Test");
    Serial.println(" No outputs driven");
    Serial.println("================================");
    printHelp();
}

void loop()
{
    handleSerial();
    updateAutoRead();
    delay(10);
}
