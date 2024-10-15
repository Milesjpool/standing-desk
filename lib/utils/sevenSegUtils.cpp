#include <sevenSegUtils.h>

Digit BLANK = Digit(false, 0, false);
Digit INVALID = Digit(false, -1, false);

Digit decodeDigit(byte data) {
    if (data == 0) {
        return BLANK;
    }

    boolean hasDot = (data & dotBit);
    byte numeric = (data & ~dotBit);

    for (int i = 0; i < DIGIT_COUNT; i++) {
        if (numeric == digitMappings[i]) {
            return Digit(true, i, hasDot);
        }
    }
    
    return INVALID;
}