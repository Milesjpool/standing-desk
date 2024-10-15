#include <heightServer.h>
#include <ESPmDNS.h>
#include <height.h>
#include <buttons.h>
#include <byteUtils.h>
#include <uri/UriRegex.h>

void HeightServer::getRoot()
{
  String message = "hello from " + name + "!\r\nLocal IP: " + wifiManager.getLocalIp() + "\r\n";
  server.send(200, "text/plain", message);
}

// E.g curl -XPOST http://.../command/02/data/0100
void HeightServer::postCommand()
{
  String typeString = server.pathArg(0);
  byte type = parseByte(const_cast<char*>(typeString.c_str()));
  
  String dataString = server.pathArg(1);
  int length = dataString.length()/2;
  byte data[length] = {0};
  parseBytes(const_cast<char*>(dataString.c_str()), data, length);

  Message command(type, data, length);
  deskSerial.issueCommand(command);
  server.send(200, "text/plain", "{ 'type': '" + formatByte(type) + "', 'body': '" + formatBytes(data, length) + "' }");
}

void HeightServer::getHeight()
{
  deskSerial.issueCommand(NO_CMD);
  deskSerial.consumeStream();

  HeightReading reading = deskSerial.getLastHeightReading();
  if (!reading.isValid())
  {
    server.send(500, "text/plain", "{ 'error': 'No height reading available' }");
    return;
  }
  server.send(200, "text/plain", "{ 'height_mm' " + String(reading.getHeight()) + ", 'age_ms': " + String(reading.getAge()) + " }");
}


void HeightServer::postHeightPreset(Message &command)
{
  deskSerial.issueCommand(command);
  server.send(200, "text/plain", "{ }");
}

// void HeightServer::postHeight()
// {
//   //   String message;
//   //   // Stop all ongoing operations
//   //   if(currentOperation != NOOP) {
//   //     // 409 is conflict status code
//   //     server.send(409, "text/plain", "A height adjustment operation is going on, please try later");
//   //   }else{
//   //     message = server.arg(0);
//   //     setHeight(atoi(message.c_str()));
//   //     server.send(200, "text/plain", message);
//   //   }
//   server.send(405, "text/plain", "Not implemented");
// }

void HeightServer::deleteHeight()
{
  deskSerial.issueCommand(NO_CMD);
  server.send(200, "text/plain", "{ }");
}

HeightServer::HeightServer(Logger &logger, String name, DeskSerial &deskSerial, WifiManager wifiManager) : name(name), deskSerial(deskSerial), logger(logger), server(PORT), wifiManager(wifiManager) {}

void HeightServer::start(int ledPin)
{
  if (MDNS.begin(name))
  {
    logger.info("MDNS responder started");
  }

  server.on("/", HTTP_GET, trackRequest(std::bind(&HeightServer::getRoot, this), "GET /", ledPin));
  server.on(UriRegex("/command/([0-9a-fA-F]{2})/data/([0-9a-fA-F]*)"), HTTP_POST, trackRequest(std::bind(&HeightServer::postCommand, this), "POST /command/*", ledPin));

  server.on("/height", HTTP_GET, trackRequest(std::bind(&HeightServer::getHeight, this), "GET /height", ledPin));
  server.on("/height", HTTP_DELETE, trackRequest(std::bind(&HeightServer::deleteHeight, this), "DELETE /height", ledPin));
  server.on("/height/1", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, M1_CMD), "POST /height/1", ledPin));
  server.on("/height/2", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, M2_CMD), "POST /height/2", ledPin));
  server.on("/height/3", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, M3_CMD), "POST /height/3", ledPin));
  server.on("/height/4", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, M4_CMD), "POST /height/4", ledPin));
  server.on("/height/stand", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, STAND_CMD), "POST /height/stand", ledPin));
  server.on("/height/sit", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, SIT_CMD), "POST /height/sit", ledPin));

  server.begin();
  logger.info("HTTP server started");
}

void HeightServer::loop()
{
  deskSerial.consumeMessage();
  server.handleClient();
}

WebServer::THandlerFunction HeightServer::trackRequest(WebServer::THandlerFunction handler, const char *name, int ledPin)
{
  return [handler, name, ledPin, this]()
  {
    digitalWrite(ledPin, 1);
    logger.info("Request received: " + String(name));
    handler();
    digitalWrite(ledPin, 0);
  };
}