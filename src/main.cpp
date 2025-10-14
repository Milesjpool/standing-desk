#include <Arduino.h>
#include <logger.h>
#include <wifiManager.h>
#include <heightServer.h>
#include <deskSerial.h>
#include <byteUtils.h>
#include <deviceStats.h>

#ifndef NAME
#define NAME "desk"
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL INFO
#endif

Logger logger(&Serial, LOG_LEVEL);

const int LED_PIN = LED_BUILTIN;

DeviceStats deviceStats;
DeskSerial deskSerial(logger);
WifiManager wifiManager(logger, deviceStats, NAME, LED_PIN);
HeightServer heightServer(logger, deskSerial, wifiManager, deviceStats);

void setup(void)
{
    Serial.begin(921600); // Inbuilt UART for debugging

    pinMode(LED_PIN, OUTPUT);

    deviceStats.begin();
    deskSerial.begin();
    wifiManager.connect(Serial);
    heightServer.start(LED_PIN);
}

void loop(void)
{
    // Monitor and reconnect WiFi if disconnected
    if (WiFi.status() != WL_CONNECTED)
    {
        logger.warn("WiFi disconnected, reconnecting...");
        heightServer.stop();
        wifiManager.connect(Serial);
        heightServer.start(LED_PIN);
    }

    deviceStats.update();
    heightServer.loop();
}
