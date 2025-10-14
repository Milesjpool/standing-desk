#include <heightServer.h>
#include <ESPmDNS.h>
#include <height.h>
#include <buttons.h>
#include <byteUtils.h>
#include <uri/UriRegex.h>

void HeightServer::getRoot()
{
    String hostname = wifiManager.getHostname();
    String ip = wifiManager.getLocalIp();

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

    server.send(200, "text/plain", message);
}

void HeightServer::getStatus()
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

    server.send(200, "application/json", message);
}

// E.g curl -XPOST http://.../command/02/data/0100
void HeightServer::postCommand()
{
    if (!enabled)
    {
        server.send(400, "application/json", "{ \"error\": \"Server is disabled\" }");
        return;
    }

    String typeString = server.pathArg(0);
    byte type = parseByte(const_cast<char *>(typeString.c_str()));

    String dataString = server.pathArg(1);
    int length = dataString.length() / 2;
    byte data[length];
    parseBytes(const_cast<char *>(dataString.c_str()), data, length);

    Message command(type, data, length);
    deskSerial.issueCommand(command);
    server.send(200, "application/json", "{ \"type\": \"" + formatByte(type) + "\", \"body\": \"" + formatBytes(data, length) + "\" }");
}

void HeightServer::getHeight()
{
    HeightReading reading = deskSerial.getLastHeightReading();

    // Only request new height if the cached reading is stale
    // This avoids interrupting ongoing movements
    if (enabled && (!reading.isValid() || reading.isStale()))
    {
        deskSerial.issueCommand(NO_CMD);
        deskSerial.consumeStream();
        reading = deskSerial.getLastHeightReading();
    }

    if (!reading.isValid())
    {
        server.send(500, "application/json", "{ \"error\": \"No height reading available\" }");
        return;
    }

    bool isMoving = movementDaemon.isMoving();
    server.send(200, "application/json", "{ \"height_mm\": " + String(reading.getHeight()) + ", \"age_ms\": " + String(reading.getStaleness()) + ", \"is_moving\": " + String(isMoving ? "true" : "false") + " }");
}

void HeightServer::postHeightPreset(Message &presetCommand)
{
    if (!enabled)
    {
        server.send(400, "application/json", "{ \"error\": \"Server is disabled\" }");
        return;
    }

    if (movementDaemon.isMoving() || targetHeight != 0)
    {
        abortCommand();
        delay(COMMAND_INTERVAL);
    }
    deskSerial.issueCommand(presetCommand);

    server.send(200, "application/json", "{ }");
}

void HeightServer::postHeight()
{
    if (!enabled)
    {
        server.send(400, "application/json", "{ \"error\": \"Server is disabled\" }");
        return;
    }

    String heightString = server.pathArg(0);
    int heightValue = atoi(heightString.c_str());
    if (heightValue < MIN_HEIGHT || heightValue > MAX_HEIGHT)
    {
        server.send(400, "application/json", "{ \"error\": \"Invalid height\" }");
        return;
    }

    abortCommand();
    delay(COMMAND_INTERVAL);
    deskSerial.consumeStream();
    HeightReading currentHeight = deskSerial.getLastHeightReading();

    if (currentHeight.isStale())
    {
        server.send(500, "application/json", "{ \"error\": \"Unable to get current height\" }");
        return;
    }

    targetHeight = heightValue;
    targetHeightDelta = targetHeight - currentHeight.getHeight();
    server.send(400, "application/json", "{ \"error\": \"Not implemented\", \"requested_height\": " + String(targetHeight) + " }");
}

void HeightServer::deleteHeight()
{
    abortCommand();
    server.send(200, "application/json", "{ }");
}

void HeightServer::getEnabled()
{
    server.send(200, "application/json", "{ \"enabled\": " + String(enabled) + " }");
}

void HeightServer::postEnabled()
{
    enabled = true;
    server.send(200, "application/json", "{ \"enabled\": " + String(enabled) + " }");
}

void HeightServer::deleteEnabled()
{
    enabled = false;
    server.send(200, "application/json", "{ \"enabled\": " + String(enabled) + " }");
}

HeightServer::HeightServer(Logger &logger, DeskSerial &deskSerial, WifiManager &wifiManager, DeviceStats &deviceStats)
    : deskSerial(deskSerial), logger(logger), server(PORT), wifiManager(wifiManager), deviceStats(deviceStats), movementDaemon(logger, deskSerial)
{
}

void HeightServer::start(int ledPin)
{
    this->ledPin = ledPin;
    String hostname = wifiManager.getHostname();
    if (MDNS.begin(hostname))
    {
        logger.info("MDNS responder started");
    }

    server.on("/", HTTP_GET, trackRequest(std::bind(&HeightServer::getRoot, this), "GET", "/"));
    server.on("/status", HTTP_GET, trackRequest(std::bind(&HeightServer::getStatus, this), "GET", "/status"));

#ifdef COMMAND_EXPLORER
    server.on(UriRegex("/command/(c[0-9a-fA-F]{2})/data/([0-9a-fA-F]*)"), HTTP_POST, trackRequest(std::bind(&HeightServer::postCommand, this), "POST", "/command"));
#endif

    server.on("/enabled", HTTP_GET, trackRequest(std::bind(&HeightServer::getEnabled, this), "GET", "/enabled"));
    server.on("/enabled", HTTP_POST, trackRequest(std::bind(&HeightServer::postEnabled, this), "POST", "/enabled"));
    server.on("/enabled", HTTP_DELETE, trackRequest(std::bind(&HeightServer::deleteEnabled, this), "DELETE", "/enabled"));
    server.on("/height", HTTP_GET, trackRequest(std::bind(&HeightServer::getHeight, this), "GET", "/height"));
    server.on("/height", HTTP_DELETE, trackRequest(std::bind(&HeightServer::deleteHeight, this), "DELETE", "/height"));
    server.on("/height/preset/1", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M1_CMD)), "POST", "/height/preset/1"));
    server.on("/height/preset/2", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M2_CMD)), "POST", "/height/preset/2"));
    server.on("/height/preset/3", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M3_CMD)), "POST", "/height/preset/3"));
    server.on("/height/preset/4", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M4_CMD)), "POST", "/height/preset/4"));
    server.on("/height/preset/stand", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(STAND_CMD)), "POST", "/height/preset/stand"));
    server.on("/height/preset/sit", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(SIT_CMD)), "POST", "/height/preset/sit"));

#ifdef HEIGHT_INPUT
    server.on(UriRegex("/height/([0-9]{1,4})"), HTTP_POST, trackRequest(std::bind(&HeightServer::postHeight, this), "POST", "/height/{mm}"));
#endif

    server.begin();
    logger.info("HTTP server started");
}

void HeightServer::stop()
{
    server.stop();
    server.close();
    MDNS.end();
    logger.info("HTTP server stopped");
}

void HeightServer::loop()
{
    server.handleClient();
    deskSerial.consumeMessage();
    moveTowardsTargetHeight();
    movementDaemon.update();
}

void HeightServer::moveTowardsTargetHeight()
{
    // TODO: implement this
    targetHeight = 0;
    targetHeightDelta = 0;
}

// void HeightServer::moveTowardsTargetHeight()
// {
//   if (targetHeight == 0)
//   {
//     return;
//   }

//   HeightReading currentHeight = deskSerial.getLastHeightReading();

//   if (currentHeight.getStaleness() < (abs(targetHeightDelta) * 30) + 50)
//   {
//     // logger.warn("Unable to get current height.");

//     //TODO: add retry & timeout on target height.
//     // deskSerial.issueCommand(NO_CMD);
//     // abortCommand();
//     return;
//   }

//   if (targetHeight > currentHeight.getHeight() && targetHeightDelta > 0) {
//     targetHeightDelta = targetHeight - currentHeight.getHeight();
//     deskSerial.issueCommand(UP_CMD);

//     deskSerial.consumeStream();
//   } else if (targetHeight < currentHeight.getHeight() && targetHeightDelta < 0) {
//     targetHeightDelta = targetHeight - currentHeight.getHeight();
//     deskSerial.issueCommand(DOWN_CMD);
//     deskSerial.consumeStream();
//   } else {
//     abortCommand();
//   }
// }

void HeightServer::abortCommand()
{
    targetHeight = 0;
    targetHeightDelta = 0;
    deskSerial.issueCommand(NO_CMD);
}

WebServer::THandlerFunction HeightServer::trackRequest(WebServer::THandlerFunction handler, const char *method, const char *endpoint)
{
    return [handler, method, endpoint, this]()
    {
        digitalWrite(ledPin, 1);
        logger.info("Request received: " + String(method) + " " + String(endpoint));
        deviceStats.incrementApiRequests(method, endpoint);
        handler();
        digitalWrite(ledPin, 0);
    };
}