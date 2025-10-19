#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <logger.h>
#include <height.h>
#include <message.h>

#define RXPIN 27
#define TXPIN 12

#define BAUDRATE 9600

class DeskSerial
{
private:
    SoftwareSerial serial;
    Logger &logger;
    HeightReading currentHeight;
    bool enabled;

public:
    DeskSerial(Logger &logger);
    void begin();
    void issueCommand(Message &command);
    void consumeMessage();
    void consumeStream();
    void refreshHeightReading(unsigned long tolerance_ms);
    HeightReading getLastHeightReading();
    bool isEnabled();
    void setEnabled(bool enabled);
};