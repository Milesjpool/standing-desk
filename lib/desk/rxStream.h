#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <logger.h>
#include <deviceStats.h>
#include <message.h>
#include <height.h>

void consumeMessageStream(SoftwareSerial &stream, Logger &logger, DeviceStats &deviceStats, HeightReading &currentHeight, boolean consumeFully);
void readMessage(SoftwareSerial &stream, Logger &logger, DeviceStats &deviceStats, HeightReading &currentHeight);
void processMessage(Logger &logger, DeviceStats &deviceStats, Message &message, HeightReading &currentHeight);