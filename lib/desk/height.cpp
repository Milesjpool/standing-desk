#include <height.h>
#include <sevenSegUtils.h>

//Converts an array of digits with a decimal point in centimeters, 
// into a height in millimeters
//
// Examples:
// [1 ][0 ][1 ] > 1010
// [9 ][9.][5 ] >  995
// [9.][9 ][5 ] >   99

int decodeHeight(byte* data, int dataLength) {
    int height_cm = 0;
    bool decimalPlace = false;
    float divisor = 0.1; //convert to millimeters
    
    for (size_t i = 0; i < dataLength; i++)
    {
        Digit d = decodeDigit(data[i]);
        if (!d.isValidDigit) {
            if (d.value != 0 || height_cm != 0) {
                return -1;
            }
        } else {
            height_cm = height_cm * 10 + d.value;

            if (decimalPlace) {
                divisor *= 10;
            }
            decimalPlace = decimalPlace || d.hasDot;
        }
    }
    
    return height_cm / divisor;
}
