#pragma once
#include <cstdint>
#include <ctime>

struct HeightReading {
    int height_mm;
    long timestamp_ms;
    HeightReading(int millimeters, long milliseconds : height_mm(h), timestamp_ms(milliseconds) {}
};