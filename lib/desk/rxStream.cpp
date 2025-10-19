#include <rxStream.h>
#include <byteUtils.h>
#include <message.h>

const int MIN_LENGTH = 3; // start + length + end

void consumeMessageStream(SoftwareSerial &stream, Logger &logger, HeightReading &currentHeight, boolean consumeFully)
{
    while (stream.available() >= MIN_LENGTH)
    {
        byte startByte = stream.read();
        if (startByte == START)
        {
            readMessage(stream, logger, currentHeight);
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

void readMessage(SoftwareSerial &stream, Logger &logger, HeightReading &currentHeight)
{
    byte messageLength = stream.read();
    int minimumLength = 2 + CHECKSUM_SIZE; // length + type + checksum

    if (messageLength < minimumLength)
    {
        logger.warn("message length too short - " + formatByte(messageLength));
        return;
    }

    byte messageType = stream.read();

    int dataLength = messageLength - minimumLength;
    byte data[dataLength];
    byte checksum[CHECKSUM_SIZE];

    int timeout = 50; // 50ms timeout to avoid blocking the main loop
    int remainingLength = dataLength + CHECKSUM_SIZE + sizeof(END);
    while (stream.available() < remainingLength && timeout > 0)
    {
        delay(1);
        timeout--;
        if (timeout == 0)
        {
            logger.warn("timeout waiting for data");
            return;
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
        return;
    }

    Message message = Message(messageType, data, dataLength);
    if (!message.hasChecksum(checksum))
    {
        logger.warn("Checksum mismatch. Recieved: '" + formatByte(START) + " " + formatByte(messageLength) + " " + formatByte(messageType) + " " + formatBytes(data, dataLength) + " " + formatBytes(checksum, CHECKSUM_SIZE) + " " + formatByte(endByte) + "'");
        return;
    }

    logger.debug("message read successfully. #" + formatByte(messageType) + ": [" + formatBytes(data, dataLength) + "]");

    processMessage(logger, message, currentHeight);
}

void processMessage(Logger &logger, Message &message, HeightReading &currentHeight)
{
    byte type = message.type;
    int length = message.getLength();
    byte bytes[length];
    message.writeBytes(bytes);

    switch (type)
    {
    case DISPLAY_OUT:
    {
        // TODO: Handle {0x77 0x6D 0x31} //ASR error
        // TODO: Handle {0x00 0x00 0x00} //Blank display, without decoding.

        int height = decodeHeight(message.data, message.dataLength);
        if (height > 0)
        {
            unsigned long timestamp = time(NULL);
            unsigned long firstRecorded = height == currentHeight.height_mm ? currentHeight.firstRecorded_ms : millis();
            unsigned long lastRecorded = millis();
            currentHeight = HeightReading(height, firstRecorded, lastRecorded, timestamp);
            logger.info("Received height value: " + String(height));
        }
        else if (height == 0)
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