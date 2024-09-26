#include <txStream.h>
#include <byteUtils.h>

void issueCommand(SoftwareSerial &stream, Message &command, Logger &logger)
{
  int length = command.getLength();
  byte message[length];
  command.writeBytes(message);

  digitalWrite(TXWAKE, HIGH);
  stream.write(message, length);
  logger.info("Sent command: " + formatBytes(message, length));
  delay(100);
  digitalWrite(TXWAKE, LOW);
}