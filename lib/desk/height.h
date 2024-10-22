#pragma once
#include <Arduino.h>

const int STALENESS_THRESHOLD = 500; // Min age of a height reading to be considered stale.

struct HeightReading {
    unsigned int height_mm;
    unsigned long firstRecorded_ms;
    unsigned long lastRecorded_ms;
    
    HeightReading(int millimeters, unsigned long firstRecorded, unsigned long lastRecorded);
    boolean isValid();
    unsigned int getHeight();
    unsigned long getStaleness();
    unsigned long getDuration();
    boolean isStale();
};

int decodeHeight(byte* data, int dataLength);