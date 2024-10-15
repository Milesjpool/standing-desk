#include <Arduino.h>
#include <logger.h>
#include <wifiManager.h>
#include <heightServer.h>
#include <deskSerial.h>
#include <byteUtils.h>

#ifndef NAME
#define NAME "desk"
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO
#endif

Logger logger(&Serial, LOG_LEVEL);

const int LED_PIN = LED_BUILTIN;

DeskSerial deskSerial(logger);
WifiManager wifiManager(logger, LED_PIN);
HeightServer heightServer(logger, NAME, deskSerial, wifiManager);

void setup(void) {
  Serial.begin(921600); // Inbuilt UART for debugging 

  pinMode(LED_PIN, OUTPUT);

  deskSerial.begin();
  wifiManager.connect(Serial);
  heightServer.start(LED_PIN);
}

void loop(void) {
  heightServer.loop();
}
