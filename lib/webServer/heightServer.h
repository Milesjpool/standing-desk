#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>
#include <deskSerial.h>
#include <wifiManager.h>

#define PORT 80

class HeightServer {
private:
    String name;
    Logger &logger;
    WebServer server;
    DeskSerial &deskSerial;
    WifiManager &wifiManager;
    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char* name, int ledPin);

    void getRoot();
    void getHeight();
    void postHeightPreset(Message &command);
    void deleteHeight();
    // void postHeight();
    void postCommand();

public:
    HeightServer(Logger &logger, String name, DeskSerial &deskSerial, WifiManager wifiManager);
    void start(int ledPin);
    void loop();
};
