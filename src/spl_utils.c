#include <stdlib.h>
#include "spl_utils.h"

void toByteArray(uint8_t * splitNumber, uint8_t bytes, int index) {
    for (int i = 0; i < bytes; i++) {
            splitNumber[i] = index >> ((3-i) * 8);
    }
}
int byteArrayToInteger(uint8_t * byteArray, uint8_t bytes) {
    int number = 0;
    for(int i = 0; i < 4; i++) {
        number |= byteArray[i] << ((3 - i) * 8);
    }
    return number;
}