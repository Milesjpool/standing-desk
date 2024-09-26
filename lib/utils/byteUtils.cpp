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