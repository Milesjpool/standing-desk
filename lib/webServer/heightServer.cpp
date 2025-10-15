#include <heightServer.h>
#include <ESPmDNS.h>
#include <height.h>
#include <buttons.h>
#include <byteUtils.h>
#include <uri/UriRegex.h>
#include <responseBuilder.h>

void HeightServer::getRoot()
{
    String message = ResponseBuilder::buildRootMessage(wifiManager.getHostname(), wifiManager.getLocalIp());
    server.send(200, "text/plain", message);
}

void HeightServer::getStatus()
{
    // Ensure a reasonably fresh height reading before building status
    unsigned long maxAgeMs = getMaxAgeMsFromQuery();
    ensureHeightFreshness(maxAgeMs);
    String message = ResponseBuilder::buildStatusJson(wifiManager, deviceStats, deskSerial, movementDaemon, enabled);
    server.send(200, "application/json", message);
}

void HeightServer::getMetrics()
{
    // Ensure a reasonably fresh height reading before building metrics
    unsigned long maxAgeMs = getMaxAgeMsFromQuery();
    ensureHeightFreshness(maxAgeMs);
    String metrics = ResponseBuilder::buildPrometheusMetrics(wifiManager, deviceStats, deskSerial, movementDaemon, enabled);
    server.send(200, "text/plain; version=0.0.4", metrics);
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
    unsigned long maxAgeMs = getMaxAgeMsFromQuery();
    HeightReading reading = ensureHeightFreshness(maxAgeMs);

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

    registerRoute(HTTP_GET, "/", std::bind(&HeightServer::getRoot, this));
    registerRoute(HTTP_GET, "/status", std::bind(&HeightServer::getStatus, this));
    registerRoute(HTTP_GET, "/metrics", std::bind(&HeightServer::getMetrics, this));

#ifdef COMMAND_EXPLORER
    registerRoute(HTTP_POST, UriRegex("/command/(c[0-9a-fA-F]{2})/data/([0-9a-fA-F]*)"), std::bind(&HeightServer::postCommand, this), "/command");
#endif

    registerRoute(HTTP_GET, "/enabled", std::bind(&HeightServer::getEnabled, this));
    registerRoute(HTTP_POST, "/enabled", std::bind(&HeightServer::postEnabled, this));
    registerRoute(HTTP_DELETE, "/enabled", std::bind(&HeightServer::deleteEnabled, this));
    registerRoute(HTTP_GET, "/height", std::bind(&HeightServer::getHeight, this));
    registerRoute(HTTP_DELETE, "/height", std::bind(&HeightServer::deleteHeight, this));
    registerRoute(HTTP_POST, "/height/preset/1", std::bind(&HeightServer::postHeightPreset, this, std::ref(M1_CMD)));
    registerRoute(HTTP_POST, "/height/preset/2", std::bind(&HeightServer::postHeightPreset, this, std::ref(M2_CMD)));
    registerRoute(HTTP_POST, "/height/preset/3", std::bind(&HeightServer::postHeightPreset, this, std::ref(M3_CMD)));
    registerRoute(HTTP_POST, "/height/preset/4", std::bind(&HeightServer::postHeightPreset, this, std::ref(M4_CMD)));
    registerRoute(HTTP_POST, "/height/preset/stand", std::bind(&HeightServer::postHeightPreset, this, std::ref(STAND_CMD)));
    registerRoute(HTTP_POST, "/height/preset/sit", std::bind(&HeightServer::postHeightPreset, this, std::ref(SIT_CMD)));

#ifdef HEIGHT_INPUT
    registerRoute(HTTP_POST, UriRegex("/height/([0-9]{1,4})"), std::bind(&HeightServer::postHeight, this), "/height/{mm}");
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

unsigned long HeightServer::getMaxAgeMsFromQuery()
{
    // Default freshness tolerance: 5 minutes
    const unsigned long defaultSeconds = 300;
    if (!server.hasArg("max_age_seconds"))
    {
        return defaultSeconds * 1000UL;
    }

    String value = server.arg("max_age_seconds");
    long seconds = value.toInt();
    if (seconds < 0)
    {
        seconds = 0; // clamp negative to 0 (force poll)
    }
    return static_cast<unsigned long>(seconds) * 1000UL;
}

HeightReading HeightServer::ensureHeightFreshness(unsigned long maxAgeMs)
{
    HeightReading reading = deskSerial.getLastHeightReading();

    bool needPoll = false;
    if (!reading.isValid())
    {
        needPoll = true;
    }
    else if (maxAgeMs == 0)
    {
        // Explicit forced poll
        needPoll = true;
    }
    else
    {
        unsigned long ageMs = reading.getStaleness();
        if (ageMs > maxAgeMs)
        {
            needPoll = true;
        }
    }

    // Only poll if server is enabled; otherwise return whatever is available
    if (enabled && needPoll)
    {
        deskSerial.issueCommand(NO_CMD);
        deskSerial.consumeStream();
        reading = deskSerial.getLastHeightReading();
    }

    return reading;
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
