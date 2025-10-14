#include "deviceStats.h"
#include <esp_system.h>

DeviceStats::DeviceStats()
    : bootCount(0), totalRuntimeHours(0), totalApiRequests(0), wifiConnections(0), failedHeightReadings(0), communicationErrors(0), bootTimeMillis(0)
{
}

void DeviceStats::begin()
{
    bootTimeMillis = millis();
    resetReason = getResetReasonString();

    prefs.begin("device-stats", false);
    loadFromNVS();

    // Increment boot count
    bootCount++;
    prefs.putUInt("boot_count", bootCount);
    prefs.end();
}

void DeviceStats::loadFromNVS()
{
    bootCount = prefs.getUInt("boot_count", 0);
    totalRuntimeHours = prefs.getUInt("runtime_hrs", 0);
    totalApiRequests = prefs.getUInt("api_total", 0);
    wifiConnections = prefs.getUInt("wifi_conns", 0);
    failedHeightReadings = prefs.getUInt("failed_reads", 0);
    communicationErrors = prefs.getUInt("comm_errors", 0);

    // Load API endpoint counters
    apiRequestsByEndpoint.clear();
    uint32_t epCount = prefs.getUInt("ep_count", 0);
    for (uint32_t i = 0; i < epCount; i++)
    {
        String keyBase = "ep_" + String(i);
        String endpoint = prefs.getString((keyBase + "_k").c_str(), "");
        uint32_t count = prefs.getUInt((keyBase + "_v").c_str(), 0);
        if (endpoint.length() > 0)
        {
            apiRequestsByEndpoint[endpoint] = count;
        }
    }
}

void DeviceStats::saveToNVS()
{
    prefs.begin("device-stats", false);
    prefs.putUInt("runtime_hrs", totalRuntimeHours);
    prefs.putUInt("api_total", totalApiRequests);
    prefs.putUInt("wifi_conns", wifiConnections);
    prefs.putUInt("failed_reads", failedHeightReadings);
    prefs.putUInt("comm_errors", communicationErrors);

    // Save API endpoint counters
    // Clear old entries first
    prefs.remove("ep_count");
    int idx = 0;
    for (const auto &entry : apiRequestsByEndpoint)
    {
        String keyBase = "ep_" + String(idx);
        prefs.putString((keyBase + "_k").c_str(), entry.first);
        prefs.putUInt((keyBase + "_v").c_str(), entry.second);
        idx++;
    }
    prefs.putUInt("ep_count", idx);

    prefs.end();
}

void DeviceStats::update()
{
    // Update total runtime every hour
    unsigned long currentRuntime = millis() - bootTimeMillis;
    uint32_t currentHours = currentRuntime / 3600000; // milliseconds to hours

    if (currentHours > 0)
    {
        totalRuntimeHours += currentHours;
        bootTimeMillis += currentHours * 3600000; // Adjust base time
        saveToNVS();
    }
}

String DeviceStats::getResetReasonString()
{
    esp_reset_reason_t reason = esp_reset_reason();
    switch (reason)
    {
    case ESP_RST_POWERON:
        return "power_on";
    case ESP_RST_SW:
        return "software";
    case ESP_RST_PANIC:
        return "panic";
    case ESP_RST_INT_WDT:
        return "interrupt_watchdog";
    case ESP_RST_TASK_WDT:
        return "task_watchdog";
    case ESP_RST_WDT:
        return "watchdog";
    case ESP_RST_DEEPSLEEP:
        return "deep_sleep";
    case ESP_RST_BROWNOUT:
        return "brownout";
    case ESP_RST_SDIO:
        return "sdio";
    default:
        return "unknown";
    }
}

uint32_t DeviceStats::getBootCount()
{
    return bootCount;
}

String DeviceStats::getResetReason()
{
    return resetReason;
}

String DeviceStats::getUptime()
{
    unsigned long milli = millis();
    unsigned long secs = milli / 1000, mins = secs / 60;
    unsigned int hours = mins / 60, days = hours / 24;
    milli -= secs * 1000;
    secs -= mins * 60;
    mins -= hours * 60;
    hours -= days * 24;

    char buffer[32];
    sprintf(buffer, "P%dDT%2.2d:%2.2d:%2.2d.%3.3d", (byte) days, (byte) hours, (byte) mins, (byte) secs, (int) milli);
    return String(buffer);
}

uint32_t DeviceStats::getTotalRuntimeHours()
{
    return totalRuntimeHours;
}

uint32_t DeviceStats::getTotalApiRequests()
{
    return totalApiRequests;
}

uint32_t DeviceStats::getFailedHeightReadings()
{
    return failedHeightReadings;
}

uint32_t DeviceStats::getWifiConnections()
{
    return wifiConnections;
}

uint32_t DeviceStats::getCommunicationErrors()
{
    return communicationErrors;
}

void DeviceStats::incrementApiRequests(const String &method, const String &endpoint)
{
    totalApiRequests++;

    // Create key: "METHOD /endpoint"
    String key = method + " " + endpoint;
    apiRequestsByEndpoint[key]++;

    // Save every 100 requests to reduce NVS wear
    if (totalApiRequests % 100 == 0)
    {
        saveToNVS();
    }
}

const std::map<String, uint32_t> &DeviceStats::getApiRequestsByEndpoint()
{
    return apiRequestsByEndpoint;
}

void DeviceStats::incrementWifiConnections()
{
    wifiConnections++;
    saveToNVS();
}

void DeviceStats::incrementFailedHeightReadings()
{
    failedHeightReadings++;
    saveToNVS();
}

void DeviceStats::incrementCommunicationErrors()
{
    communicationErrors++;
    saveToNVS();
}
