#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include <logger.h>

#define PORT 80

class HeightServer {
private:
    int ledPin;
    Logger &logger;
    WebServer server;
    WebServer::THandlerFunction trackRequest(WebServer::THandlerFunction handler, const char* name);

    void getRoot();
    void getHeight();
    void putHeight();
    void deleteHeight();

public:
    HeightServer(Logger &logger, int ledPin);
    void start();
    void handleClient();
};
