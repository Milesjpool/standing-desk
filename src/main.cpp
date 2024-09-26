#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <SoftwareSerial.h>
#include <logger.h>
#include <rxStream.h>
#include <txStream.h>
#include <wifiManager.h>
#include <heightServer.h>

#define NAME "desk"
Logger logger(&Serial, ERROR);

#define RXPIN  23
#define TXPIN  22
SoftwareSerial deskSerial(RXPIN, TXPIN);

const int LED_PIN = LED_BUILTIN;

WifiManager wifiManager(logger, LED_PIN);
HeightServer heightServer(logger, LED_PIN);

// HeightReading currentHeight;

void setup(void) {
  Serial.begin(921600); // Inbuilt UART for debugging 
  deskSerial.begin(9600); // The controller uses 9600 bauds for serial communication 

  pinMode(LED_PIN, OUTPUT);
  pinMode(TXWAKE, OUTPUT);

  wifiManager.connect(Serial);

  if (MDNS.begin(NAME)) {
    logger.info("MDNS responder started");
  }

  heightServer.start();
}

void loop(void) {
  // Message command = NO_CMD;

  // issueCommand(deskSerial, command, logger);

  consumeMessageStream(deskSerial, logger);
  
  heightServer.handleClient();
}
