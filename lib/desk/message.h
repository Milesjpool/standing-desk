#pragma once
#include <Arduino.h>
#include <modbusCRC.h>

enum MessageType {
  BUTTON_PRESS = 0x02,
  CURRENT_HEIGHT = 0x12,
  UNKNOWN_1 = 11,
  UNKNOWN_2 = 15
};

const byte START = 0x9B;
const byte END = 0x9D;

class Message {
private:
  byte contentLength;
  byte* content;
  byte* checksum;
public:
  byte type;
  int dataLen;
  byte* data;
  
  Message(byte type, byte* data, int dataLen);
  int getLength();
  void writeBytes(byte* bytes);
  boolean hasChecksum(byte* receivedChecksum);
};;
