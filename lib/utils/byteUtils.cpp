#include <byteUtils.h>

String formatByte(byte b) {
  String hex = String(b, HEX);
  if (b < 0x10) {
    return "0" + hex;
  } 
  hex.toUpperCase();
  return hex;
}      


String formatBytes(byte* b, size_t len) {
  String result = "";
  for (int i = 0; i < len; i++) {
    if (i > 0) {
      result += " ";
    }
    result += formatByte(b[i]);
  }
  return result;
}

byte parseByte(char* hex) {
  return strtol(hex, NULL, 16);
}

void parseBytes(char* hex, byte* b, size_t len) {
  for (int i = 0; i < len; i++) {
    char raw[2] = {hex[2*i], hex[(2*i)+1]};
    b[i] = parseByte(raw);
  }
}