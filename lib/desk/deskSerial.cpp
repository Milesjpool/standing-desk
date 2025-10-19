#include <deskSerial.h>
#include <rxStream.h>
#include <txStream.h>
#include <buttons.h>

DeskSerial::DeskSerial(Logger &logger)
    : serial(RXPIN, TXPIN), logger(logger), currentHeight(0, 0, 0), enabled(true)
{
}

void DeskSerial::begin()
{
    pinMode(TXWAKE, OUTPUT);
    serial.begin(BAUDRATE);
}

void DeskSerial::consumeStream()
{
    consumeMessageStream(serial, logger, currentHeight, true);
}

void DeskSerial::consumeMessage()
{
    consumeMessageStream(serial, logger, currentHeight, false);
}

void DeskSerial::issueCommand(Message &command)
{
    digitalWrite(TXWAKE, HIGH);
    delay(5);
    sendCommand(serial, command);
    logger.info("Sent command: " + command.toString());
    delay(100);
    digitalWrite(TXWAKE, LOW);
}

HeightReading DeskSerial::getLastHeightReading()
{
    return currentHeight;
}

void DeskSerial::refreshHeightReading()
{
    HeightReading reading = getLastHeightReading();

    // Only request new height if the cached reading is stale
    // This avoids interrupting ongoing movements
    if (enabled && (!reading.isValid() || reading.isStale()))
    {
        issueCommand(NO_CMD);
        consumeStream();
    }
}

bool DeskSerial::isEnabled()
{
    return enabled;
}

void DeskSerial::setEnabled(bool enabled)
{
    this->enabled = enabled;
}