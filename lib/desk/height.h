#pragma once
#include <Arduino.h>

struct HeightReading {
    int height_mm;
    long timestamp_ms;
    HeightReading(int millimeters, long milliseconds) : height_mm(millimeters), timestamp_ms(milliseconds) {}
    boolean isValid() {
        return height_mm != 0;
    }
    long getAge() {
        return millis() - timestamp_ms;
    }
    int getHeight() {
        return height_mm;
    }
};

int decodeHeight(byte* data, int dataLength);