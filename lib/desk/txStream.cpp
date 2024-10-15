#include <txStream.h>
#include <byteUtils.h>

void sendCommand(SoftwareSerial &stream, Message &command)
{
  int length = command.getLength();
  byte message[length];
  command.writeBytes(message);
  stream.write(message, length);
}