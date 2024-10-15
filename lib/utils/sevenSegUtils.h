#pragma once
#include <Arduino.h>

#define DIGIT_COUNT 10
const byte digitMappings[DIGIT_COUNT] = {
  0x3f, //0
  0x06, //1
  0x5b, //2
  0x4f, //3
  0x66, //4
  0x6d, //5
  0x7d, //6
  0x07, //7
  0x7f, //8
  0x6f //9
};
const byte dotBit = 0x80;

struct Digit {
  bool isValidDigit;
  int value;
  bool hasDot;
  Digit(bool isValidDigit, int value, bool hasDot) : isValidDigit(isValidDigit), value(value), hasDot(hasDot) {}
};

Digit decodeDigit(byte data);