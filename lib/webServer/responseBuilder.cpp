#include "responseBuilder.h"

String ResponseBuilder::buildRootMessage(const String &hostname, const String &ip)
{
    String message = "hello from " + hostname + "!\r\n";
    message += "Local IP: " + ip + "\r\n\r\n";
    message += "API Endpoints:\r\n";
    message += "  GET    /status                    - Device status\r\n";
    message += "  GET    /metrics                   - Prometheus metrics\r\n";
    message += "  GET    /enabled                   - Check if server is enabled\r\n";
    message += "  POST   /enabled                   - Enable server\r\n";
    message += "  DELETE /enabled                   - Disable server\r\n";
    message += "  GET    /height                    - Current desk height\r\n";
    message += "  POST   /height/preset/{1-4}       - Move to preset 1-4\r\n";
    message += "  POST   /height/preset/{sit|stand} - Move to sit/stand preset\r\n";
    message += "  DELETE /height                    - Stop movement\r\n";
    return message;
}

String ResponseBuilder::buildPrometheusMetrics(
    WifiManager &wifiManager,
    DeviceStats &deviceStats,
    DeskSerial &deskSerial,
    MovementDaemon &movementDaemon)
{
    String hostname = wifiManager.getHostname();
    String ip = wifiManager.getLocalIp();
    String macAddress = WiFi.macAddress();
    String ssid = WiFi.SSID();
    int rssi = WiFi.RSSI();
    HeightReading reading = deskSerial.getLastHeightReading();
    bool isMoving = movementDaemon.isMoving();

    uint32_t freeHeap = ESP.getFreeHeap();
    uint32_t totalHeap = ESP.getHeapSize();
    uint32_t usedHeap = totalHeap - freeHeap;
    uint32_t flashSize = ESP.getFlashChipSize();
    uint32_t flashUsed = ESP.getSketchSize();
    uint32_t flashFree = ESP.getFreeSketchSpace();
    String chipModel = ESP.getChipModel();

    uint32_t currentTimestamp = time(NULL);
    uint32_t bootCount = deviceStats.getBootCount();
    uint32_t totalRuntimeHours = deviceStats.getTotalRuntimeHours();
    uint32_t totalRuntimeSeconds = totalRuntimeHours * 3600;
    uint32_t uptimeSeconds = millis() / 1000;
    uint32_t wifiConnections = deviceStats.getWifiConnections();
    uint32_t failedHeightReadings = deviceStats.getFailedHeightReadings();
    uint32_t communicationErrors = deviceStats.getCommunicationErrors();
    uint32_t totalApiRequests = deviceStats.getTotalApiRequests();

    // Common labels for all metrics
    String commonLabels = "hostname=\"" + hostname + "\",chip_model=\"" + chipModel + "\",instance=\"" + ip + ":80\"";

    String metrics = "";

    // Current system timestamp
    metrics += "# HELP standing_desk_current_timestamp Current system Unix timestamp\n";
    metrics += "# TYPE standing_desk_current_timestamp gauge\n";
    metrics += "standing_desk_current_timestamp{" + commonLabels + "} " + String(currentTimestamp) + "\n\n";

    // Uptime
    metrics += "# HELP standing_desk_uptime_seconds Time since device boot in seconds\n";
    metrics += "# TYPE standing_desk_uptime_seconds gauge\n";
    metrics += "standing_desk_uptime_seconds{" + commonLabels + "} " + String(uptimeSeconds) + "\n\n";

    // Boot count
    metrics += "# HELP standing_desk_boot_count_total Total number of device boots\n";
    metrics += "# TYPE standing_desk_boot_count_total counter\n";
    metrics += "standing_desk_boot_count_total{" + commonLabels + "} " + String(bootCount) + "\n\n";

    // Total runtime
    metrics += "# HELP standing_desk_runtime_seconds_total Total runtime across all boots in seconds\n";
    metrics += "# TYPE standing_desk_runtime_seconds_total counter\n";
    metrics += "standing_desk_runtime_seconds_total{" + commonLabels + "} " + String(totalRuntimeSeconds) + "\n\n";

    // WiFi connections
    metrics += "# HELP standing_desk_wifi_connections_total Total number of WiFi connections\n";
    metrics += "# TYPE standing_desk_wifi_connections_total counter\n";
    metrics += "standing_desk_wifi_connections_total{" + commonLabels + ",ssid=\"" + ssid + "\"} " + String(wifiConnections) + "\n\n";

    // WiFi RSSI
    metrics += "# HELP standing_desk_wifi_rssi_dbm WiFi signal strength in dBm\n";
    metrics += "# TYPE standing_desk_wifi_rssi_dbm gauge\n";
    metrics += "standing_desk_wifi_rssi_dbm{" + commonLabels + ",ssid=\"" + ssid + "\"} " + String(rssi) + "\n\n";

    // Flash memory
    metrics += "# HELP standing_desk_flash_bytes Flash memory in bytes\n";
    metrics += "# TYPE standing_desk_flash_bytes gauge\n";
    metrics += "standing_desk_flash_bytes{" + commonLabels + ",type=\"total\"} " + String(flashSize) + "\n";
    metrics += "standing_desk_flash_bytes{" + commonLabels + ",type=\"used\"} " + String(flashUsed) + "\n";
    metrics += "standing_desk_flash_bytes{" + commonLabels + ",type=\"free\"} " + String(flashFree) + "\n\n";

    // Heap memory
    metrics += "# HELP standing_desk_heap_bytes Heap memory in bytes\n";
    metrics += "# TYPE standing_desk_heap_bytes gauge\n";
    metrics += "standing_desk_heap_bytes{" + commonLabels + ",type=\"total\"} " + String(totalHeap) + "\n";
    metrics += "standing_desk_heap_bytes{" + commonLabels + ",type=\"used\"} " + String(usedHeap) + "\n";
    metrics += "standing_desk_heap_bytes{" + commonLabels + ",type=\"free\"} " + String(freeHeap) + "\n\n";

    // API requests total
    metrics += "# HELP standing_desk_api_requests_total Total number of API requests\n";
    metrics += "# TYPE standing_desk_api_requests_total counter\n";
    metrics += "standing_desk_api_requests_total{" + commonLabels + "} " + String(totalApiRequests) + "\n\n";

    // API requests by endpoint
    const auto &endpoints = deviceStats.getApiRequestsByEndpoint();
    if (!endpoints.empty())
    {
        metrics += "# HELP standing_desk_api_requests_by_endpoint_total API requests by method and endpoint\n";
        metrics += "# TYPE standing_desk_api_requests_by_endpoint_total counter\n";
        for (const auto &entry : endpoints)
        {
            // Parse "METHOD /endpoint" format
            String key = entry.first;
            int spaceIndex = key.indexOf(' ');
            String method = key.substring(0, spaceIndex);
            String endpoint = key.substring(spaceIndex + 1);

            metrics += "standing_desk_api_requests_by_endpoint_total{" + commonLabels +
                       ",method=\"" + method + "\",endpoint=\"" + endpoint + "\"} " +
                       String(entry.second) + "\n";
        }
        metrics += "\n";
    }

    // Error counters
    metrics += "# HELP standing_desk_errors_total Total number of errors by type\n";
    metrics += "# TYPE standing_desk_errors_total counter\n";
    metrics += "standing_desk_errors_total{" + commonLabels + ",type=\"failed_height_readings\"} " + String(failedHeightReadings) + "\n";
    metrics += "standing_desk_errors_total{" + commonLabels + ",type=\"communication_errors\"} " + String(communicationErrors) + "\n\n";

    // Desk control enabled
    metrics += "# HELP standing_desk_control_enabled Whether desk control is enabled (1=enabled, 0=disabled)\n";
    metrics += "# TYPE standing_desk_control_enabled gauge\n";
    metrics += "standing_desk_control_enabled{" + commonLabels + "} " + String(deskSerial.isEnabled() ? 1 : 0) + "\n\n";

    // Desk height
    if (reading.isValid())
    {
        metrics += "# HELP standing_desk_height_mm Current desk height in millimeters\n";
        metrics += "# TYPE standing_desk_height_mm gauge\n";
        metrics += "standing_desk_height_mm{" + commonLabels + "} " + String(reading.getHeight()) + "\n\n";

        metrics += "# HELP standing_desk_height_reading_timestamp Unix timestamp when height reading was taken\n";
        metrics += "# TYPE standing_desk_height_reading_timestamp gauge\n";
        metrics += "standing_desk_height_reading_timestamp{" + commonLabels + "} " + String(reading.getTimestamp()) + "\n\n";

        metrics += "# HELP standing_desk_height_reading_age_ms Age of height reading in milliseconds\n";
        metrics += "# TYPE standing_desk_height_reading_age_ms gauge\n";
        metrics += "standing_desk_height_reading_age_ms{" + commonLabels + "} " + String(reading.getStaleness()) + "\n\n";
    }

    // Desk moving status
    metrics += "# HELP standing_desk_is_moving Whether desk is currently moving (1=moving, 0=stationary)\n";
    metrics += "# TYPE standing_desk_is_moving gauge\n";
    metrics += "standing_desk_is_moving{" + commonLabels + "} " + String(isMoving ? 1 : 0) + "\n";

    return metrics;
}

String ResponseBuilder::buildStatusJson(
    WifiManager &wifiManager,
    DeviceStats &deviceStats,
    DeskSerial &deskSerial,
    MovementDaemon &movementDaemon)
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
    uint32_t currentTimestamp = time(NULL);

    String message = "{ \"uptime\": \"" + currentUptime +
                     "\", \"boot_count\": " + String(deviceStats.getBootCount()) +
                     ", \"last_reset_reason\": \"" + deviceStats.getResetReason() +
                     "\", \"total_runtime_hours\": " + String(deviceStats.getTotalRuntimeHours()) +
                     ", \"current_timestamp\": " + String(currentTimestamp);

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

    message += ", \"desk\": { \"control_enabled\": " + String(deskSerial.isEnabled() ? "true" : "false");

    if (reading.isValid())
    {
        message += ", \"height_mm\": " + String(reading.getHeight()) +
                   ", \"height_timestamp\": " + String(reading.getTimestamp()) +
                   ", \"age_ms\": " + String(reading.getStaleness()) +
                   ", \"is_moving\": " + String(isMoving ? "true" : "false");
    }
    else
    {
        message += ", \"height_mm\": null, \"height_timestamp\": null, \"age_ms\": null, \"is_moving\": false";
    }

    message += " }";

    message += " }";

    return message;
}