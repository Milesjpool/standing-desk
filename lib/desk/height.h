#pragma once
#include <Arduino.h>

struct HeightReading {
    unsigned int height_mm;
    unsigned long firstRecorded_ms;
    unsigned long lastRecorded_ms;
    
    HeightReading(int millimeters, unsigned long firstRecorded, unsigned long lastRecorded);
    boolean isValid();
    unsigned int getHeight();
    unsigned long getStaleness();
    unsigned long getDuration();
};

int decodeHeight(byte* data, int dataLength);