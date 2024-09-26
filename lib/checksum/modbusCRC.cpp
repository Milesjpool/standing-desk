#include <modbusCRC.h>

std::array<byte, CHECKSUM_SIZE> ModRTU_CRC(byte buf[], int len) {
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos]; // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) { // Loop over each bit
      if ((crc & 0x0001) != 0) {   // If the LSB is set
        crc >>= 1;                 // Shift right and XOR 0xA001
        crc ^= 0xA001;
      } else       // Else LSB is not set
        crc >>= 1; // Just shift right
    }
  }
  
  byte msb = (crc & 0xFF00U) >> 8U;
  byte lsb = (crc & 0x00FFU);
  return {msb, lsb};
}