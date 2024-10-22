#include <heightServer.h>
#include <ESPmDNS.h>
#include <height.h>
#include <buttons.h>
#include <byteUtils.h>
#include <uri/UriRegex.h>
#include <uptime.h>

void HeightServer::getRoot()
{
  String hostname = wifiManager.getHostname();
  String ip = wifiManager.getLocalIp();
  String currentUptime = uptime();

  String message = "hello from " + hostname + "!\r\nLocal IP: " + ip + "\r\nUptime: " + currentUptime + "\r\n";
  
  if (server.hasArg("f") && server.arg("f") == "JSON") {
    message = "{ 'hostname': '" + hostname + "', 'ip': '" + ip + "', 'uptime': '" + currentUptime + "' }";
  }

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
  server.send(200, "text/plain", "{ 'height_mm' " + String(reading.getHeight()) + ", 'age_ms': " + String(reading.getStaleness()) + " }");
}

void HeightServer::postHeightPreset(Message &presetCommand)
{
  if (deskMoving || targetHeight != 0)
  {
    abortCommand();
    delay(COMMAND_INTERVAL);
  }
  deskSerial.issueCommand(presetCommand);
  
  server.send(200, "text/plain", "{ }");
}

void HeightServer::postHeight()
{
  
  String heightString = server.pathArg(0);
  int heightValue = atoi(heightString.c_str());
  if (heightValue < MIN_HEIGHT || heightValue > MAX_HEIGHT)
  {
    server.send(400, "text/plain", "{ 'error': 'Invalid height' }");
    return;
  }
  
  abortCommand();
  delay(COMMAND_INTERVAL);
  deskSerial.consumeStream();
  HeightReading currentHeight = deskSerial.getLastHeightReading();

  if (currentHeight.isStale())
  {
    server.send(500, "text/plain", "{ 'error': 'Unable to get current height' }");
    return;
  }

  targetHeight = heightValue;
  targetHeightDelta = targetHeight - currentHeight.getHeight();
  server.send(400, "text/plain", "{ 'error': 'Not implemented', 'requested_height': " + String(targetHeight) + " }");
}

void HeightServer::deleteHeight()
{
  abortCommand();
  server.send(200, "text/plain", "{ }");
}

HeightServer::HeightServer(Logger &logger, DeskSerial &deskSerial, WifiManager wifiManager) : deskSerial(deskSerial), logger(logger), server(PORT), wifiManager(wifiManager){
}

void HeightServer::start(int ledPin)
{
  String hostname = wifiManager.getHostname();
  if (MDNS.begin(hostname))
  {
    logger.info("MDNS responder started");
  }

  server.on("/", HTTP_GET, trackRequest(std::bind(&HeightServer::getRoot, this), "GET /", ledPin));

  //TODO: Disable this in release.
  server.on(UriRegex("/command/([0-9a-fA-F]{2})/data/([0-9a-fA-F]*)"), HTTP_POST, trackRequest(std::bind(&HeightServer::postCommand, this), "POST /command/*", ledPin));

  server.on("/height", HTTP_GET, trackRequest(std::bind(&HeightServer::getHeight, this), "GET /height", ledPin));
  server.on("/height", HTTP_DELETE, trackRequest(std::bind(&HeightServer::deleteHeight, this), "DELETE /height", ledPin));
  server.on(UriRegex("/height/([0-9]{1,4})"), HTTP_POST, trackRequest(std::bind(&HeightServer::postHeight, this), "POST /height/*", ledPin));
  server.on("/height/preset/1", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M1_CMD)), "POST /height/preset/1", ledPin));
  server.on("/height/preset/2", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M2_CMD)), "POST /height/preset/2", ledPin));
  server.on("/height/preset/3", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M3_CMD)), "POST /height/preset/3", ledPin));
  server.on("/height/preset/4", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(M4_CMD)), "POST /height/preset/4", ledPin));
  server.on("/height/preset/stand", HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(STAND_CMD)), "POST /height/preset/stand", ledPin));
  server.on("/height/preset/sit",   HTTP_POST, trackRequest(std::bind(&HeightServer::postHeightPreset, this, std::ref(SIT_CMD)),   "POST /height/preset/sit", ledPin));

  server.begin();
  logger.info("HTTP server started");
}

void HeightServer::loop()
{
  server.handleClient();
  deskSerial.consumeMessage();
  moveTowardsTargetHeight();
  updateDeskMovingState();
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

void HeightServer::updateDeskMovingState()
{
  HeightReading height = deskSerial.getLastHeightReading();

  boolean newState = height.isValid() && !height.isStale()
              && height.getDuration() < MOVEMENT_TIMEOUT;

  if (deskMoving != newState)
  {
    String transition = newState ? "started" : "stopped";
    logger.info("Desk " + transition + " moving.");
    deskMoving = newState;
  }
}

void HeightServer::abortCommand()
{
  targetHeight = 0;
  targetHeightDelta = 0;
  deskSerial.issueCommand(NO_CMD);
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