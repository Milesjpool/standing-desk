#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>
#include <deskSerial.h>

#define PORT 80

class HeightServer {
private:
    int ledPin;
    Logger &logger;
    WebServer server;
    DeskSerial &deskSerial;
    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char* name);

    void getRoot();
    void getHeight();
    void putHeight();
    void deleteHeight();

public:
    HeightServer(DeskSerial &deskSerial, Logger &logger, int ledPin);
    void start(String name);
    void handleClient();
};
