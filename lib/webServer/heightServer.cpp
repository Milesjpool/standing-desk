#include <heightServer.h>
#include <ESPmDNS.h>

void HeightServer::getRoot() {
  server.send(200, "text/plain", "hello from esp32!\r\n");
}

void HeightServer::getHeight() {
  char hstr[4];
//   sprintf(hstr, "%d", currentHeight);
  server.send(200, "text/plain", hstr);
}

void HeightServer::putHeight() {
  //   String message;
  //   // Stop all ongoing operations 
  //   if(currentOperation != NOOP) {
  //     // 409 is conflict status code
  //     server.send(409, "text/plain", "A height adjustment operation is going on, please try later");
  //   }else{
  //     message = server.arg(0);
  //     setHeight(atoi(message.c_str()));
  //     server.send(200, "text/plain", message);  
  //   }
  server.send(405, "text/plain", "Not implemented");
}

void HeightServer::deleteHeight() {
  //   stopMoving();
  server.send(405, "text/plain", "Not implemented");
}


HeightServer::HeightServer(DeskSerial &deskSerial, Logger &logger, int ledPin) : deskSerial(deskSerial), logger(logger), ledPin(ledPin), server(PORT) {}

void HeightServer::start(String name) {
  if (MDNS.begin(name)) {
    logger.info("MDNS responder started");
  }

  server.on("/", HTTP_GET, trackRequest(std::bind(&HeightServer::getRoot, this), "GET /"));
  server.on("/height", HTTP_GET, trackRequest(std::bind(&HeightServer::getHeight, this), "GET /height"));
  server.on("/height", HTTP_PUT, trackRequest(std::bind(&HeightServer::putHeight, this), "PUT /height"));
  server.on("/height", HTTP_DELETE, trackRequest(std::bind(&HeightServer::deleteHeight, this), "DELETE /height"));

  server.begin();
  logger.info("HTTP server started");
}

void HeightServer::handleClient() {
  server.handleClient();
}

WebServer::THandlerFunction HeightServer::trackRequest(WebServer::THandlerFunction handler, const char* name)
{
  return [handler, name, this]() {
    digitalWrite(ledPin, 1);
    logger.info("Request received: " + String(name));
    handler();
    digitalWrite(ledPin, 0);
  };
}