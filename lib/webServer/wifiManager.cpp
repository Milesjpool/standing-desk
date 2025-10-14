#include <wifiManager.h>
#include <deviceStats.h>

WifiManager::WifiManager(Logger &logger, DeviceStats &deviceStats, String name, int ledPin)
    : logger(logger), deviceStats(deviceStats), name(name), ledPin(ledPin)
{
}

void WifiManager::connect(Stream &outputSerial)
{
    deviceStats.incrementWifiConnections();

    WiFi.mode(WIFI_STA);
    WiFi.setHostname(name.c_str());
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    WiFi.begin(ssid, password);
    outputSerial.print("\nConnecting.");
    digitalWrite(ledPin, 0);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        digitalWrite(ledPin, !digitalRead(ledPin));
        outputSerial.print(".");
    }
    digitalWrite(ledPin, 0);
    outputSerial.println();

    logger.info("Connected to " + String(ssid) + ". IP address: " + WiFi.localIP().toString());
}

String WifiManager::getLocalIp()
{
    return WiFi.localIP().toString();
}

String WifiManager::getHostname()
{
    return String(WiFi.getHostname());
}
