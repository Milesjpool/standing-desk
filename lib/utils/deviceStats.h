#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include <map>

class DeviceStats
{
private:
    Preferences prefs;
    uint32_t bootCount;
    String resetReason;
    uint32_t totalRuntimeHours;

    // API request counters by endpoint (key: "METHOD /endpoint")
    uint32_t totalApiRequests;
    std::map<String, uint32_t> apiRequestsByEndpoint;

    uint32_t wifiConnections;
    uint32_t failedHeightReadings;
    uint32_t communicationErrors;
    unsigned long bootTimeMillis;

    void loadFromNVS();
    void saveToNVS();
    String getResetReasonString();

public:
    DeviceStats();
    void begin();
    void update();

    uint32_t getBootCount();
    String getResetReason();
    String getUptime();
    uint32_t getTotalRuntimeHours();
    uint32_t getTotalApiRequests();
    const std::map<String, uint32_t> &getApiRequestsByEndpoint();
    uint32_t getWifiConnections();
    uint32_t getFailedHeightReadings();
    uint32_t getCommunicationErrors();

    void incrementApiRequests(const String &method, const String &endpoint);
    void incrementWifiConnections();
    void incrementFailedHeightReadings();
    void incrementCommunicationErrors();
};
