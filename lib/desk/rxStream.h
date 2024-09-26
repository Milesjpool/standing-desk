#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <logger.h>
#include <message.h>

void consumeMessageStream(SoftwareSerial &stream, Logger &logger);
Message readMessage(SoftwareSerial &stream, Logger &logger);

void processMessage(Logger &logger, Message message);