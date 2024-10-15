#include <message.h>
#include <byteUtils.h>

Message::Message(byte type, byte* data, int dataLen): type(type), data(data), dataLength(dataLen) {
  contentLength = sizeof(contentLength) + sizeof(type) + dataLen + CHECKSUM_SIZE;
  content = new byte[contentLength]();
  content[0] = contentLength;
  content[1] = type;
  for (int i = 0; i < dataLen; i++) {
    content[2 + i] = data[i];
  }

  std::array<byte, CHECKSUM_SIZE> modbus = ModRTU_CRC(content, contentLength - CHECKSUM_SIZE);
  for (int i = 0; i < CHECKSUM_SIZE; i++) {
    content[contentLength - CHECKSUM_SIZE + i] = modbus.at(i);
  }
}

int Message::getLength() {
  return sizeof(START) + contentLength + sizeof(END);
}

void Message::writeBytes(byte* bytes) {
  bytes[0] = START;
  for (int i = 0; i < contentLength; i++) {
    bytes[1 + i] = content[i];
  }
  bytes[contentLength + 1] = END;
}

boolean Message::hasChecksum(byte* checksum) {
  for (int i = 0; i < CHECKSUM_SIZE; i++) {
    if (content[contentLength - CHECKSUM_SIZE + i] != checksum[i]) {
      return false;
    }
  }
  return true;
}

String Message::toString() {
  String str = formatBytes(content, contentLength);
  return str;
}