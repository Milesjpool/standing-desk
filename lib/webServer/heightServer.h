#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>
#include <deskSerial.h>
#include <wifiManager.h>
#include <movementDaemon.h>

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
    MovementDaemon movementDaemon;

    uint targetHeight = 0;
    int targetHeightDelta = 0;

    void getRoot();
    void getHeight();
    void postHeight();
    void postHeightPreset(Message &command);
    void deleteHeight();
    void postCommand();

    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char *name, int ledPin);
    void abortCommand();
    void moveTowardsTargetHeight();

public:
    HeightServer(Logger &logger, DeskSerial &deskSerial, WifiManager wifiManager);
    void start(int ledPin);
    void loop();
};
