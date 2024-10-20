#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <logger.h>

#if !defined STA_SSID || !defined STA_PASS 
#error "STA_SSID and STA_PASS must be defined"
#endif

class WifiManager {
private:
    int ledPin;
    const char* ssid = STA_SSID;
    const char* password = STA_PASS;
    Logger &logger;
    String name;
public:
    WifiManager(Logger &logger, String name, int ledPin);
    void connect(Stream &outputSerial);
    String getLocalIp();
    String getHostname();
};
