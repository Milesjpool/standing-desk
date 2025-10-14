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
    String ssid = WiFi.SSID();
    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    uint32_t heapUsagePercent = 100 - ((freeHeap * 100) / totalHeap);
    uint32_t flashSize = ESP.getFlashChipSize();
    uint32_t flashUsed = ESP.getSketchSize();
    uint32_t flashFree = ESP.getFreeSketchSpace();
    uint32_t flashUsagePercent = (flashUsed * 100) / flashSize;
    String chipModel = ESP.getChipModel();
    bool isMoving = movementDaemon.isMoving();

    String message = "{ \"uptime\": \"" + currentUptime +
                     "\", \"boot_count\": " + String(deviceStats.getBootCount()) +
                     ", \"last_reset_reason\": \"" + deviceStats.getResetReason() +
                     "\", \"total_runtime_hours\": " + String(deviceStats.getTotalRuntimeHours());

    message += ", \"hardware\": { \"chip_model\": \"" + chipModel +
               "\", \"firmware_version\": \"" + String(__DATE__) + " " + String(__TIME__) + "\" }";

    message += ", \"connectivity\": { \"hostname\": \"" + hostname +
               "\", \"ip\": \"" + ip +
               "\", \"mac\": \"" + macAddress +
               "\", \"wifi_ssid\": \"" + ssid +
               "\", \"wifi_connections\": " + String(wifiConnections) +
               ", \"wifi_rssi\": " + String(rssi) + " }";

    message += ", \"flash\": { \"total_bytes\": " + String(flashSize) +
               ", \"used_bytes\": " + String(flashUsed) +
               ", \"free_bytes\": " + String(flashFree) +
               ", \"usage_percent\": " + String(flashUsagePercent) + " }";

    message += ", \"heap\": { \"total_bytes\": " + String(totalHeap) +
               ", \"used_bytes\": " + String(usedHeap) +
               ", \"free_bytes\": " + String(freeHeap) +
               ", \"usage_percent\": " + String(heapUsagePercent) + " }";

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

    message += ", \"desk\": { \"control_enabled\": " + String(enabled ? "true" : "false");

    if (reading.isValid())
    {
        message += ", \"height_mm\": " + String(reading.getHeight()) +
                   ", \"age_ms\": " + String(reading.getStaleness()) +
                   ", \"is_moving\": " + String(isMoving ? "true" : "false");
    }
    else
    {
        message += ", \"height_mm\": null, \"age_ms\": null, \"is_moving\": false";
    }

    message += " }";

    message += " }";

    return message;
}