#pragma once
#include <Arduino.h>
#include <array>

#define CHECKSUM_SIZE 2

std::array<byte, CHECKSUM_SIZE> ModRTU_CRC(byte buf[], int len);