#pragma once
#include <Arduino.h>

const int STALENESS_THRESHOLD = 500; // Min age of a height reading to be considered stale.

struct HeightReading
{
    unsigned int height_mm;
    unsigned long timestamp_unix;
    unsigned long recorded_ms;

    HeightReading(int millimeters, unsigned long timestamp, unsigned long milliseconds);
    boolean isValid();
    unsigned int getHeight();
    unsigned long getTimestamp();
    unsigned long getStaleness();
    boolean isStale(unsigned long tolerance_ms);
    boolean isStale();
};

int decodeHeight(byte *data, int dataLength);