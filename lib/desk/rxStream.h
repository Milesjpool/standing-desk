#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <logger.h>
#include <message.h>
#include <height.h>

void consumeMessageStream(SoftwareSerial &stream, Logger &logger, HeightReading &currentHeight, boolean consumeFully);
void readMessage(SoftwareSerial &stream, Logger &logger, HeightReading &currentHeight);
void processMessage(Logger &logger, Message &message, HeightReading &currentHeight);