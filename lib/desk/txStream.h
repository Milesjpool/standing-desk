#pragma once
#include <Arduino.h>
#include <logger.h>
#include <message.h>
#include <SoftwareSerial.h>

#define TXWAKE 33

void sendCommand(SoftwareSerial &stream, Message &command);