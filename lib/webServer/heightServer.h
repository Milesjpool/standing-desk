#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>
#include <deskSerial.h>
#include <wifiManager.h>
#include <movementDaemon.h>
#include <deviceStats.h>

#define PORT 80

const uint COMMAND_INTERVAL = 1500; // Min time between accepted commands.

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
    bool enabled = true;
    int ledPin;

    void getRoot();
    void getStatus();
    void getHeight();
    void postHeight();
    void postHeightPreset(Message &command);
    void deleteHeight();
    void postCommand();
    void getEnabled();
    void postEnabled();
    void deleteEnabled();
    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char *method, const char *endpoint);
    void abortCommand();
    void moveTowardsTargetHeight();

public:
    HeightServer(Logger &logger, DeskSerial &deskSerial, WifiManager &wifiManager, DeviceStats &deviceStats);
    void start(int ledPin);
    void stop();
    void loop();
};
