#include <cstdint>
#include "shiftRows.h"
#include <iostream>
#include <iomanip>

void rowShifting(uint8_t* cipherText) {
    uint8_t temp = cipherText[1];

    cipherText[1] = cipherText[5];
    cipherText[5] = cipherText[9];
    cipherText[9] = cipherText[13];
    cipherText[13] = temp;

    temp = cipherText[2];
    uint8_t temp1 = cipherText[6];
    cipherText[2] = cipherText[10];
    cipherText[6] = cipherText[14];
    cipherText[10] = temp;
    cipherText[14] = temp1;

    temp = cipherText[15];
    cipherText[15] = cipherText[11];
    cipherText[11] = cipherText[7];
    cipherText[7] = cipherText[3];
    cipherText[3] = temp;
}

void invRowShifting(uint8_t* cipherText) {
    uint8_t temp = cipherText[13];

    cipherText[13] = cipherText[9];
    cipherText[9] = cipherText[5];
    cipherText[5] = cipherText[1];
    cipherText[1] = temp;

    temp = cipherText[14];
    uint8_t temp1 = cipherText[10];
    cipherText[14] = cipherText[6];
    cipherText[10] = cipherText[2];
    cipherText[6] = temp;
    cipherText[2] = temp1;

    temp = cipherText[3];
    cipherText[3] = cipherText[7];
    cipherText[7] = cipherText[11];
    cipherText[11] = cipherText[15];
    cipherText[15] = temp;
}