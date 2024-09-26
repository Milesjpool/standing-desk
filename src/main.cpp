#include <Arduino.h>
#include <logger.h>
#include <wifiManager.h>
#include <heightServer.h>
#include <deskSerial.h>

#define NAME "desk"
Logger logger(&Serial, ERROR);

const int LED_PIN = LED_BUILTIN;

DeskSerial deskSerial(logger);
WifiManager wifiManager(logger, LED_PIN);
HeightServer heightServer(deskSerial, logger, LED_PIN);

// HeightReading currentHeight;

void setup(void) {
  Serial.begin(921600); // Inbuilt UART for debugging 

  pinMode(LED_PIN, OUTPUT);

  deskSerial.begin();
  wifiManager.connect(Serial);
  heightServer.start(NAME);
}

void loop(void) {
  deskSerial.consumeStream();
  heightServer.handleClient();
}
