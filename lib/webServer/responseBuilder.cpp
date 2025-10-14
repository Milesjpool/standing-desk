#include "responseBuilder.h"

String ResponseBuilder::buildRootMessage(const String &hostname, const String &ip)
{
    String message = "hello from " + hostname + "!\r\n";
    message += "Local IP: " + ip + "\r\n\r\n";
    message += "API Endpoints:\r\n";
    message += "  GET    /status                    - Device status\r\n";
    message += "  GET    /enabled                   - Check if server is enabled\r\n";
    message += "  POST   /enabled                   - Enable server\r\n";
    message += "  DELETE /enabled                   - Disable server\r\n";
    message += "  GET    /height                    - Current desk height\r\n";
    message += "  POST   /height/preset/{1-4}       - Move to preset 1-4\r\n";
    message += "  POST   /height/preset/{sit|stand} - Move to sit/stand preset\r\n";
    message += "  DELETE /height                    - Stop movement\r\n";
    return message;
}

String ResponseBuilder::buildStatusJson(
    WifiManager &wifiManager,
    DeviceStats &deviceStats,
    DeskSerial &deskSerial,
    MovementDaemon &movementDaemon,
    bool enabled)
{
    String hostname = wifiManager.getHostname();
    String ip = wifiManager.getLocalIp();
    String currentUptime = deviceStats.getUptime();
    int wifiConnections = deviceStats.getWifiConnections();
    String macAddress = WiFi.macAddress();
    HeightReading reading = deskSerial.getLastHeightReading();
    int rssi = WiFi.RSSI();
    uint32_t freeHeap = ESP.getFreeHeap();
    bool isMoving = movementDaemon.isMoving();

    String message = "{ \"hostname\": \"" + hostname +
                     "\", \"ip\": \"" + ip +
                     "\", \"mac\": \"" + macAddress +
                     "\", \"uptime\": \"" + currentUptime +
                     "\", \"wifi_connections\": " + String(wifiConnections) +
                     ", \"wifi_rssi\": " + String(rssi) +
                     ", \"free_heap_bytes\": " + String(freeHeap) +
                     ", \"boot_count\": " + String(deviceStats.getBootCount()) +
                     ", \"last_reset_reason\": \"" + deviceStats.getResetReason() +
                     "\", \"total_runtime_hours\": " + String(deviceStats.getTotalRuntimeHours()) +
                     ", \"enabled\": " + String(enabled);

    message += ", \"api_requests\": { \"total\": " + String(deviceStats.getTotalApiRequests());

    const auto &endpoints = deviceStats.getApiRequestsByEndpoint();
    if (!endpoints.empty())
    {
        message += ", \"endpoints\": {";
        bool first = true;
        for (const auto &entry : endpoints)
        {
            if (!first)
                message += ", ";
            first = false;
            // Escape the key for JSON (replace spaces with underscores for easier parsing)
            String safeKey = entry.first;
            safeKey.replace(" ", "_");
            message += "\"" + safeKey + "\": " + String(entry.second);
        }
        message += "}";
    }
    message += " }";

    message += ", \"errors\": { \"failed_height_readings\": " + String(deviceStats.getFailedHeightReadings()) +
               ", \"communication_errors\": " + String(deviceStats.getCommunicationErrors()) + " }";

    if (reading.isValid())
    {
        message += ", \"desk\": { \"height_mm\": " + String(reading.getHeight()) +
                   ", \"age_ms\": " + String(reading.getStaleness()) +
                   ", \"is_moving\": " + String(isMoving ? "true" : "false") + " }";
    }
    else
    {
        message += ", \"desk\": null";
    }

    message += " }";

    return message;
}