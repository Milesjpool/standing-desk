#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>
#include <deskSerial.h>
#include <wifiManager.h>
#include <movementDaemon.h>
#include <deviceStats.h>
#include <httpUtils.h>

#define PORT 80

const uint COMMAND_INTERVAL = 1500;                       // Min time between accepted commands.
const uint DEFAULT_FRESHNESS_TOLERANCE = 5 * 60 * 1000UL; // Default height reading freshness tolerance: 5 minutes

const uint MIN_HEIGHT = 675;
const uint MAX_HEIGHT = 1260;

class HeightServer
{
private:
    Logger &logger;
    WebServer server;
    DeskSerial &deskSerial;
    WifiManager &wifiManager;
    DeviceStats &deviceStats;
    MovementDaemon movementDaemon;

    uint targetHeight = 0;
    int targetHeightDelta = 0;
    int ledPin;

    void getRoot();
    void getStatus();
    void getMetrics();
    void getHeight();
    void postHeight();
    void postHeightPreset(Message &command);
    void deleteHeight();
    void postCommand();
    void getEnabled();
    void postEnabled();
    void deleteEnabled();
    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char *method, const char *endpoint);

    template <typename PathType>
    void registerRoute(HTTPMethod method, const PathType &path, WebServer::THandlerFunction handler, const char *endpointName = nullptr)
    {
        const char *methodStr = ::httpMethodToString(method);
        const char *endpoint = endpointName ? endpointName : path;
        server.on(path, method, trackRequest(handler, methodStr, endpoint));
    }

    void abortCommand();
    void moveTowardsTargetHeight();
    unsigned long getFreshnessToleranceFromQuery();

public:
    HeightServer(Logger &logger, DeskSerial &deskSerial, WifiManager &wifiManager, DeviceStats &deviceStats);
    void start(int ledPin);
    void stop();
    void loop();
};
