#pragma once
#include <Arduino.h>
#include <modbusCRC.h>

enum MessageType {
  BUTTON_PRESS = 0x02,
  DISPLAY_OUT = 0x12,
  UNKNOWN_1 = 0x11,
  UNKNOWN_2 = 0x15
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
  int dataLength;
  byte* data;
  
  Message(byte type, byte* data, int dataLen);
  ~Message();
  int getLength();
  void writeBytes(byte* bytes);
  boolean hasChecksum(byte* receivedChecksum);
  String toString();
  boolean operator==(const Message &other) const;
  boolean operator!=(const Message &other) const;
};;
