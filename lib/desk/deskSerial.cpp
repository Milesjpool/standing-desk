#include <deskSerial.h>
#include <rxStream.h>
#include <txStream.h>

DeskSerial::DeskSerial(Logger &logger) : serial(RXPIN, TXPIN), logger(logger) {}

void DeskSerial::begin() {
    pinMode(TXWAKE, OUTPUT);
    serial.begin(BAUDRATE);
}

void DeskSerial::consumeStream() {
    consumeMessageStream(serial, logger);
}

void DeskSerial::issueCommand() {
  // Message command = NO_CMD;
  // issueCommand(deskSerial, command, logger);
}