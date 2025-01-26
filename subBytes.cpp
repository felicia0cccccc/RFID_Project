#include <cstdint>
#include "subBytes.h"
#include "sbox.h"
#include <iostream>
#include <iomanip>

void bytesSubtitution(uint8_t* cipherText) {
    for (int i = 0; i < 16; i++) {
        cipherText[i] = sbox[cipherText[i]];
    }
}

void invBytesSubstitution(uint8_t* cipherText) {
    for (int i = 0; i < 16; i++) {
        cipherText[i] = inv_sbox[cipherText[i]];
    }
}