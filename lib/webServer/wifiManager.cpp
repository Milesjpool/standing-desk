#include <wifiManager.h>

WifiManager::WifiManager(Logger &logger, int ledPin): logger(logger), ledPin(ledPin) {}

void WifiManager::connect(Stream &outputSerial) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  outputSerial.print("\nConnecting.");
  digitalWrite(ledPin, 0);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(ledPin, !digitalRead(ledPin));
    outputSerial.print(".");
  }
  digitalWrite(ledPin, 0);
  outputSerial.println();

  logger.info("Connected to " + String(ssid) + ". IP address: " + WiFi.localIP().toString());
}
