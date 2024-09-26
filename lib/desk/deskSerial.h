#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <logger.h>

#define RXPIN  23
#define TXPIN  22

#define BAUDRATE 9600

class DeskSerial {
private:
    SoftwareSerial serial;
    Logger &logger;
public:
    DeskSerial(Logger &logger);
    void begin();
    void issueCommand();
    void consumeStream();
};