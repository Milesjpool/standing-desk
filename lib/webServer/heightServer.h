#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>
#include <deskSerial.h>
#include <wifiManager.h>

#define PORT 80

const int COMMAND_INTERVAL = 1500;
const int STALENESS_TIMEOUT = 1000;
const int MOVEMENT_TIMEOUT = 250;

class HeightServer {
private:
    Logger &logger;
    WebServer server;
    DeskSerial &deskSerial;
    WifiManager &wifiManager;
    
    boolean deskMoving = false;

    void getRoot();
    void getHeight();
    void postHeightPreset(Message &command);
    void deleteHeight();
    // void postHeight();
    void postCommand();

    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char* name, int ledPin);
    void updateDeskState();
public:
    HeightServer(Logger &logger, DeskSerial &deskSerial, WifiManager wifiManager);
    void start(int ledPin);
    void loop();
};
