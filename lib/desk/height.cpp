#include <height.h>
#include <sevenSegUtils.h>

// Converts an array of digits with a decimal point in centimeters,
//  into a height in millimeters
//
//  Examples:
//  [1 ][0 ][1 ] > 1010
//  [9 ][9.][5 ] >  995
//  [9.][9 ][5 ] >   99

int decodeHeight(byte *data, int dataLength)
{
    int height_cm = 0;
    bool decimalPlace = false;
    float divisor = 0.1; // convert to millimeters

    for (size_t i = 0; i < dataLength; i++)
    {
        Digit d = decodeDigit(data[i]);
        if (!d.isValidDigit)
        {
            if (d.value != 0 || height_cm != 0)
            {
                return -1;
            }
        }
        else
        {
            height_cm = height_cm * 10 + d.value;

            if (decimalPlace)
            {
                divisor *= 10;
            }
            decimalPlace = decimalPlace || d.hasDot;
        }
    }

    return height_cm / divisor;
}

HeightReading::HeightReading(int millimeters, unsigned long timestamp, unsigned long milliseconds)
    : height_mm(millimeters), timestamp_unix(timestamp), recorded_ms(milliseconds)
{
}
boolean HeightReading::isValid()
{
    return height_mm != 0;
}
unsigned int HeightReading::getHeight()
{
    return height_mm;
}
unsigned long HeightReading::getStaleness()
{
    unsigned long now = millis();
    if (now < recorded_ms)
    {
        return ULONG_MAX - recorded_ms + now;
    }
    else
    {
        return now - recorded_ms;
    }
}
unsigned long HeightReading::getTimestamp()
{
    return timestamp_unix;
}
boolean HeightReading::isStale()
{
    return getStaleness() > STALENESS_THRESHOLD;
}