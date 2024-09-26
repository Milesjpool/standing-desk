#include <rxStream.h>
#include <byteUtils.h>
#include <message.h>

const byte NULL_MSG_TYPE = 0xFF;
const Message NULL_MSG = Message(NULL_MSG_TYPE, {}, 0);

void consumeMessageStream(SoftwareSerial &stream, Logger &logger)
{
  int minLength = 3; // start + length + end

  while (stream.available() >= minLength)
  {
    byte startByte = stream.read();
    if (startByte == START)
    {
      Message message = readMessage(stream, logger);
      processMessage(logger, message);
    }
    else
    {
      String excessData = formatByte(startByte);
      while (stream.available() > 0 && stream.peek() != START)
      {
        byte b = stream.read();
        excessData += " " + formatByte(b);
      }
      logger.debug("Flushed excess data - " + excessData);
    }
  }
}

Message readMessage(SoftwareSerial &stream, Logger &logger)
{
  byte messageLength = stream.read();
  int minimumLength = 2 + CHECKSUM_SIZE; // length + type + checksum

  if (messageLength < minimumLength)
  {
    logger.warn("message length too short - " + formatByte(messageLength));
    return NULL_MSG;
  }

  byte messageType = stream.read();

  int dataLength = messageLength - minimumLength;
  byte data[dataLength];
  byte checksum[CHECKSUM_SIZE];

  int timeout = 1000;
  while (stream.available() < (dataLength + CHECKSUM_SIZE) && timeout > 0)
  {
    delay(1);
    timeout--;
    if (timeout == 0)
    {
      logger.warn("timeout waiting for data");
      return NULL_MSG;
    }
  }
  stream.read(data, dataLength);
  stream.read(checksum, CHECKSUM_SIZE);

  byte endByte = stream.read();
  if (endByte != END)
  {
    logger.warn(String("End byte not found. ") +
                "Expected: '... " + formatByte(END) + "'. " +
                "Recieved: '" + formatByte(START) + " " + formatByte(messageLength) + " " + formatByte(messageType) + " " + formatBytes(data, dataLength) + " " + formatBytes(checksum, CHECKSUM_SIZE) + " " + formatByte(endByte) + "'");
    return NULL_MSG;
  }

  Message message = Message(messageType, data, dataLength);
  if (!message.hasChecksum(checksum))
  {
    logger.warn("Checksum mismatch. Recieved: '" + formatByte(START) + " " + formatByte(messageLength) + " " + formatByte(messageType) + " " + formatBytes(data, dataLength) + " " + formatBytes(checksum, CHECKSUM_SIZE) + " " + formatByte(endByte) + "'");
    return NULL_MSG;
  }

  logger.debug("message read successfully. #" + formatByte(messageType) + ": [" + formatBytes(data, dataLength) + "]");
  return message;
}

void processMessage(Logger &logger, Message message)
{
  byte type = message.type;
  int length = message.getLength();
  byte bytes[length];
  message.writeBytes(bytes);

  switch (type)
  {
  case NULL_MSG_TYPE:
    logger.debug("Invalid message");
    break;
  case CURRENT_HEIGHT:
    logger.info("Received height message: " + formatBytes(bytes, length));
    break;
  default:
    logger.debug("Unknown message type: #" + formatByte(type) + ": [" + formatBytes(bytes, length) + "]");
    break;
  }
}