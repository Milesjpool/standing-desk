#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <logger.h>

#ifndef STASSID
#define STASSID "SSID"
#define STAPSK  "Password"
#endif

class WifiManager {
private:
    int ledPin;
    const char* ssid = STASSID;
    const char* password = STAPSK;
    Logger &logger;
public:
    WifiManager(Logger &logger, int ledPin);
    void connect(Stream &outputSerial);
};
