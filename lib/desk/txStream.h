#pragma once
#include <Arduino.h>
#include <logger.h>
#include <message.h>
#include <SoftwareSerial.h>

#define TXWAKE 14

void issueCommand(SoftwareSerial &stream, Message &command, Logger &logger);