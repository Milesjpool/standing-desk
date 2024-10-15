#include <rxStream.h>
#include <byteUtils.h>
#include <message.h>

const byte NULL_MSG_TYPE = 0xFF;
const Message NULL_MSG = Message(NULL_MSG_TYPE, {}, 0);

const int MIN_LENGTH = 3; // start + length + end

void consumeMessageStream(SoftwareSerial &stream, Logger &logger, HeightReading &currentHeight, boolean consumeFully)
{
  while (stream.available() >= MIN_LENGTH)
  {
    byte startByte = stream.read();
    if (startByte == START)
    {
      Message message = readMessage(stream, logger);
      processMessage(logger, message, currentHeight);
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
    if (!consumeFully)
    {
      break;
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
  int remainingLength = dataLength + CHECKSUM_SIZE + sizeof(END);
  while (stream.available() < remainingLength && timeout > 0)
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

void processMessage(Logger &logger, Message message, HeightReading &currentHeight)
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
  case DISPLAY_OUT:
  {
    // TODO: Handle {0x77 0x6D 0x31} //ASR error
    // TODO: Handle {0x00 0x00 0x00} //Blank display, without decoding.

    int height = decodeHeight(message.data, message.dataLength);
    if (height > 0)
    {
      currentHeight = HeightReading(height, millis());
      logger.info("Received height value: " + String(height));
    } else if (height == 0)
    {
      logger.debug("Ignored empty height message: " + message.toString());
    }
    else
    {
      logger.warn("Invalid height message: " + message.toString());
    }
    break;
  }
  default:
    logger.debug("Unknown message type: #" + formatByte(type) + ": [" + message.toString() + "]");
    break;
  }
}