#pragma once
#include <Arduino.h>

String formatByte(byte b);
String formatBytes(byte* b, size_t len);

byte parseByte(char* hex);
void parseBytes(char* hex, byte* b, size_t len);