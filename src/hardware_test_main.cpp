#include <Arduino.h>
#include <ctype.h>
#include <string.h>

#include "config.h"
#include "pinmap.h"

namespace
{
    struct NamedPin
    {
        const char *name;
        uint8_t pin;
    };

    const NamedPin OUTPUTS[] = {
        {"ALM_L", PIN_ALM_TEST_L},
        {"ALM_H", PIN_ALM_TEST_H},
        {"EOL_OC", PIN_EOL_OC_TEST},
        {"EOL_SC", PIN_EOL_SC_TEST},
        {"LED_RDY", PIN_LED_RDY},
        {"LED_PASS", PIN_LED_PASS},
        {"LED_FAIL", PIN_LED_FAIL},
        {"BUZZER", PIN_BUZZER},
    };

    const NamedPin DIGITAL_INPUTS[] = {
        {"TEST_SW", PIN_TEST_SW},
        {"FLT_IN", PIN_FLT_IN},
        {"FLT_NC", PIN_FLT_NC},
        {"FLT_NO", PIN_FLT_NO},
    };

    constexpr size_t COMMAND_BUFFER_SIZE = 96;
    constexpr unsigned long AUTO_STATUS_PERIOD_MS = 1000UL;
    constexpr unsigned long MAX_PULSE_MS = 5000UL;
    constexpr bool FIRMWARE_LOCAL_ECHO = true;

    char commandBuffer[COMMAND_BUFFER_SIZE];
    size_t commandLength = 0;
    bool autoStatusEnabled = false;
    unsigned long lastAutoStatusAt = 0;

    bool pulseActive = false;
    uint8_t pulsePin = 0;
    unsigned long pulseOffAt = 0;

    bool namesMatch(const char *left, const char *right)
    {
        while (*left != '\0' && *right != '\0')
        {
            if (toupper(*left) != toupper(*right))
            {
                return false;
            }

            left++;
            right++;
        }

        return *left == '\0' && *right == '\0';
    }

    const NamedPin *findOutput(const char *name)
    {
        for (const NamedPin &output : OUTPUTS)
        {
            if (namesMatch(output.name, name))
            {
                return &output;
            }
        }

        return nullptr;
    }

    const NamedPin *findOutputByPin(uint8_t pin)
    {
        for (const NamedPin &output : OUTPUTS)
        {
            if (output.pin == pin)
            {
                return &output;
            }
        }

        return nullptr;
    }

    const char *levelText(int value)
    {
        return value == HIGH ? "HIGH" : "LOW";
    }

    const char *shortLevelText(int value)
    {
        return value == HIGH ? "H" : "L";
    }

    void setAllOutputsLow()
    {
        for (const NamedPin &output : OUTPUTS)
        {
            digitalWrite(output.pin, LOW);
        }

        pulseActive = false;
    }

    void configurePins()
    {
        for (const NamedPin &output : OUTPUTS)
        {
            digitalWrite(output.pin, LOW);
            pinMode(output.pin, OUTPUT);
        }

        pinMode(PIN_TEST_SW, INPUT);
        pinMode(PIN_FLT_IN, INPUT_PULLUP);
        pinMode(PIN_FLT_NC, INPUT_PULLDOWN);
        pinMode(PIN_FLT_NO, INPUT_PULLDOWN);
        pinMode(PIN_PWR_SENSE, INPUT);
        analogReadResolution(12);
        analogSetPinAttenuation(PIN_PWR_SENSE, ADC_11db);
    }

    float readPowerSenseVoltage()
    {
        uint32_t total = 0;
        for (uint8_t sample = 0; sample < ADC_SAMPLE_COUNT; sample++)
        {
            total += analogRead(PIN_PWR_SENSE);
            delay(2);
        }

        const float raw = static_cast<float>(total) / ADC_SAMPLE_COUNT;
        return (raw / ADC_MAX_READING) * ADC_REFERENCE_VOLTAGE * DUT_POWER_ADC_SCALE;
    }

    void printOutputStates()
    {
        Serial.println("Outputs:");
        for (const NamedPin &output : OUTPUTS)
        {
            Serial.print("  D");
            Serial.print(output.pin);
            Serial.print(" ");
            Serial.print(shortLevelText(digitalRead(output.pin)));
            Serial.print("  ");
            Serial.println(output.name);
        }
    }

    void printInputStates()
    {
        Serial.println("Inputs:");
        for (const NamedPin &input : DIGITAL_INPUTS)
        {
            Serial.print("  D");
            Serial.print(input.pin);
            Serial.print(" ");
            Serial.print(shortLevelText(digitalRead(input.pin)));
            Serial.print("  ");
            Serial.println(input.name);
        }

        Serial.print("  D");
        Serial.print(PIN_PWR_SENSE);
        Serial.print(" ADC  PWR_SENSE raw=");
        Serial.print(analogRead(PIN_PWR_SENSE));
        Serial.print(" estimated=");
        Serial.print(readPowerSenseVoltage(), 2);
        Serial.println(" V");
    }

    void printTemperature()
    {
        Serial.print("Internal temperature estimate: ");
        Serial.print(temperatureRead(), 1);
        Serial.println(" C");
        Serial.println("Note: ESP32 internal temperature is rough and varies by board/package.");
    }

    void printStatus()
    {
        Serial.println();
        Serial.print("Hardware test status ms=");
        Serial.println(millis());
        printInputStates();
        printOutputStates();
        printTemperature();
        Serial.println();
    }

    void printPrompt()
    {
        Serial.print("READY> ");
    }

    void printHelp()
    {
        Serial.println();
        Serial.println("STR-MON hardware test firmware");
        Serial.println("Commands:");
        Serial.println("  H                         Help, works as a single key");
        Serial.println("  S                         Status, works as a single key");
        Serial.println("  A                         Toggle 1 second status monitor, works as a single key");
        Serial.println("  ON <output>               Drive output HIGH");
        Serial.println("  OFF <output>              Drive output LOW");
        Serial.println("  D19_H                     Drive GPIO19 HIGH");
        Serial.println("  D19_L                     Drive GPIO19 LOW");
        Serial.println("  OFF ALL                   Drive every output LOW");
        Serial.println("  PULSE <output> <ms>       Drive output HIGH briefly, max 5000 ms");
        Serial.println("  T                         Temperature, works as a single key");
        Serial.println();
        Serial.println("Outputs: ALM_L, ALM_H, EOL_OC, EOL_SC, LED_RDY, LED_PASS, LED_FAIL, BUZZER");
        Serial.println("Output pins: D25, D33, D26, D27, D21, D19, D18, D17");
        Serial.println("Boot safety: every output is configured LOW before accepting commands.");
        Serial.println();
    }

    void printUnknownOutput(const char *name)
    {
        Serial.print("Unknown output: ");
        Serial.println(name);
        Serial.println("Send H for output names.");
    }

    void printUnknownOutputPin(uint8_t pin)
    {
        Serial.print("D");
        Serial.print(pin);
        Serial.println(" is not a configured output pin.");
        Serial.println("Send H for output pins.");
    }

    bool parseLevel(const char *text, int &level)
    {
        if (namesMatch(text, "1") || namesMatch(text, "HIGH") || namesMatch(text, "ON"))
        {
            level = HIGH;
            return true;
        }

        if (namesMatch(text, "0") || namesMatch(text, "LOW") || namesMatch(text, "OFF"))
        {
            level = LOW;
            return true;
        }

        return false;
    }

    void setOutputLevel(const char *name, int level)
    {
        const NamedPin *output = findOutput(name);
        if (output == nullptr)
        {
            printUnknownOutput(name);
            return;
        }

        digitalWrite(output->pin, level);
        Serial.print(output->name);
        Serial.print(" GPIO");
        Serial.print(output->pin);
        Serial.print(" set ");
        Serial.println(levelText(level));
    }

    void setOutputPinLevel(uint8_t pin, int level)
    {
        const NamedPin *output = findOutputByPin(pin);
        if (output == nullptr)
        {
            printUnknownOutputPin(pin);
            return;
        }

        digitalWrite(output->pin, level);
        Serial.print("D");
        Serial.print(output->pin);
        Serial.print(" ");
        Serial.print(shortLevelText(level));
        Serial.print("  ");
        Serial.println(output->name);
    }

    bool handleDirectPinCommand(const char *command)
    {
        if (toupper(command[0]) != 'D' || !isdigit(command[1]))
        {
            return false;
        }

        char *end = nullptr;
        const unsigned long pin = strtoul(command + 1, &end, 10);
        if (pin > 255 || end == nullptr || *end != '_')
        {
            return false;
        }

        const char levelChar = static_cast<char>(toupper(*(end + 1)));
        if ((levelChar != 'H' && levelChar != 'L') || *(end + 2) != '\0')
        {
            return false;
        }

        setOutputPinLevel(static_cast<uint8_t>(pin), levelChar == 'H' ? HIGH : LOW);
        return true;
    }


    void pulseOutput(const char *name, unsigned long pulseMs)
    {
        const NamedPin *output = findOutput(name);
        if (output == nullptr)
        {
            printUnknownOutput(name);
            return;
        }

        if (pulseMs > MAX_PULSE_MS)
        {
            Serial.println("Pulse rejected: max is 5000 ms.");
            return;
        }

        digitalWrite(output->pin, HIGH);
        pulseActive = true;
        pulsePin = output->pin;
        pulseOffAt = millis() + pulseMs;

        Serial.print(output->name);
        Serial.print(" pulse HIGH for ");
        Serial.print(pulseMs);
        Serial.println(" ms");
    }

    void handleCommand(char *line)
    {
        char *command = strtok(line, " \t");
        if (command == nullptr)
        {
            printPrompt();
            return;
        }

        if (handleDirectPinCommand(command))
        {
            printPrompt();
            return;
        }

        if (namesMatch(command, "H") || namesMatch(command, "HELP") || namesMatch(command, "?"))
        {
            printHelp();
            printPrompt();
            return;
        }

        if (namesMatch(command, "S") || namesMatch(command, "STATUS"))
        {
            printStatus();
            printPrompt();
            return;
        }

        if (namesMatch(command, "A") || namesMatch(command, "AUTO"))
        {
            autoStatusEnabled = !autoStatusEnabled;
            Serial.print("Auto status monitor ");
            Serial.println(autoStatusEnabled ? "ON" : "OFF");
            printPrompt();
            return;
        }

        if (namesMatch(command, "T") || namesMatch(command, "TEMP"))
        {
            printTemperature();
            printPrompt();
            return;
        }

        if (namesMatch(command, "ON") || namesMatch(command, "OFF"))
        {
            char *name = strtok(nullptr, " \t");
            if (name == nullptr)
            {
                Serial.println("Missing output name.");
                printPrompt();
                return;
            }

            if (namesMatch(command, "OFF") && namesMatch(name, "ALL"))
            {
                setAllOutputsLow();
                Serial.println("All outputs LOW");
                printPrompt();
                return;
            }

            setOutputLevel(name, namesMatch(command, "ON") ? HIGH : LOW);
            printPrompt();
            return;
        }

        if (namesMatch(command, "SET"))
        {
            char *name = strtok(nullptr, " \t");
            char *levelTextValue = strtok(nullptr, " \t");
            int level = LOW;
            if (name == nullptr || levelTextValue == nullptr || !parseLevel(levelTextValue, level))
            {
                Serial.println("Usage: SET <output> <0|1|LOW|HIGH>");
                printPrompt();
                return;
            }

            setOutputLevel(name, level);
            printPrompt();
            return;
        }

        if (namesMatch(command, "PULSE"))
        {
            char *name = strtok(nullptr, " \t");
            char *durationText = strtok(nullptr, " \t");
            if (name == nullptr || durationText == nullptr)
            {
                Serial.println("Usage: PULSE <output> <ms>");
                printPrompt();
                return;
            }

            pulseOutput(name, strtoul(durationText, nullptr, 10));
            printPrompt();
            return;
        }

        Serial.println("Unknown command. Send H for help.");
        printPrompt();
    }

    void handleSerial()
    {
        while (Serial.available() > 0)
        {
            const char next = static_cast<char>(Serial.read());
            if (commandLength == 0)
            {
                const char command = static_cast<char>(toupper(next));
                if (command == 'S' || command == 'H' || command == 'A' || command == 'T' || command == '?')
                {
                    char singleKeyCommand[] = {command, '\0'};
                    Serial.println(command);
                    handleCommand(singleKeyCommand);
                    continue;
                }
            }

            if (next == '\b' || next == 0x7f)
            {
                if (commandLength > 0)
                {
                    commandLength--;
                    if (FIRMWARE_LOCAL_ECHO)
                    {
                        Serial.print("\b \b");
                    }
                }
                continue;
            }

            if (next == '\r' || next == '\n')
            {
                if (FIRMWARE_LOCAL_ECHO && commandLength > 0)
                {
                    Serial.println();
                }
                commandBuffer[commandLength] = '\0';
                handleCommand(commandBuffer);
                commandLength = 0;
                continue;
            }

            if (commandLength < (COMMAND_BUFFER_SIZE - 1))
            {
                commandBuffer[commandLength++] = next;
                if (FIRMWARE_LOCAL_ECHO)
                {
                    Serial.write(next);
                }
            }
            else
            {
                commandLength = 0;
                Serial.println("Command too long; buffer cleared.");
            }
        }
    }

    void updatePulse()
    {
        if (pulseActive && static_cast<long>(millis() - pulseOffAt) >= 0)
        {
            digitalWrite(pulsePin, LOW);
            pulseActive = false;
            Serial.println("Pulse complete; output LOW");
        }
    }

    void updateAutoStatus()
    {
        if (!autoStatusEnabled || (millis() - lastAutoStatusAt) < AUTO_STATUS_PERIOD_MS)
        {
            return;
        }

        lastAutoStatusAt = millis();
        printStatus();
    }
}

void setup()
{
    Serial.begin(SERIAL_BAUDRATE);
    delay(200);
    configurePins();
    setAllOutputsLow();

    Serial.println();
    Serial.println("================================");
    Serial.println(" STR-MON Hardware Test Firmware");
    Serial.println(" Outputs forced LOW on boot");
    Serial.print(" Serial baud: ");
    Serial.println(SERIAL_BAUDRATE);
    Serial.println("================================");
    printHelp();
    printPrompt();
}

void loop()
{
    handleSerial();
    updatePulse();
    updateAutoStatus();
    delay(10);
}
