#include <wifiManager.h>
#include <deviceStats.h>
#include <time.h>

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

    syncTime();
}

void WifiManager::syncTime()
{
    configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.google.com");

    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;

    logger.info("Syncing time via NTP...");
    while (timeinfo.tm_year < (2024 - 1900) && ++retry < retry_count)
    {
        delay(1000);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

    if (timeinfo.tm_year >= (2024 - 1900))
    {
        logger.info("Time synchronized: " + String(asctime(&timeinfo)));
    }
    else
    {
        logger.warn("Failed to sync time via NTP");
    }
}

String WifiManager::getLocalIp()
{
    return WiFi.localIP().toString();
}

String WifiManager::getHostname()
{
    return String(WiFi.getHostname());
}
