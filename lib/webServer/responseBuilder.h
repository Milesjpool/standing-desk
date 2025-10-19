#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <wifiManager.h>
#include <deviceStats.h>
#include <deskSerial.h>
#include <movementDaemon.h>

class ResponseBuilder
{
public:
    static String buildRootMessage(
        const String &hostname,
        const String &ip);

    static String buildStatusJson(
        WifiManager &wifiManager,
        DeviceStats &deviceStats,
        DeskSerial &deskSerial,
        MovementDaemon &movementDaemon);

    static String buildPrometheusMetrics(
        WifiManager &wifiManager,
        DeviceStats &deviceStats,
        DeskSerial &deskSerial,
        MovementDaemon &movementDaemon);
};
