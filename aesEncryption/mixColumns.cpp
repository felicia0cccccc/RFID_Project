#include <cstdint>
#include "mixColumns.h"
#include <cstddef>
#include <iostream>
#include <iomanip>

uint8_t gf(uint8_t hex, uint8_t byte) {
    uint8_t result = 0;
    uint8_t hi_bit_set;
    for (int i = 0; i < 8; i++) {
        if (byte & 1)
            result ^= hex;
        hi_bit_set = hex & 0x80;
        hex <<= 1;
        if (hi_bit_set)
            hex ^= 0x1b; // AES irreducible polynomial
        byte >>= 1;
    }
    return result;
}

void columnsMixing(uint8_t* cipherText) {
    uint8_t temp[16];
    
    // Process each column
    for (int i = 0; i < 4; i++) {
        // Store the result in temp array
        temp[4 * i + 0] = gf(0x02, cipherText[4 * i + 0]) ^ 
                         gf(0x03, cipherText[4 * i + 1]) ^ 
                         cipherText[4 * i + 2] ^ 
                         cipherText[4 * i + 3];
                         
        temp[4 * i + 1] = cipherText[4 * i + 0] ^ 
                         gf(0x02, cipherText[4 * i + 1]) ^ 
                         gf(0x03, cipherText[4 * i + 2]) ^ 
                         cipherText[4 * i + 3];
                         
        temp[4 * i + 2] = cipherText[4 * i + 0] ^ 
                         cipherText[4 * i + 1] ^ 
                         gf(0x02, cipherText[4 * i + 2]) ^ 
                         gf(0x03, cipherText[4 * i + 3]);
                         
        temp[4 * i + 3] = gf(0x03, cipherText[4 * i + 0]) ^ 
                         cipherText[4 * i + 1] ^ 
                         cipherText[4 * i + 2] ^ 
                         gf(0x02, cipherText[4 * i + 3]);
    }
    
    // Copy result back to state array
    for (int i = 0; i < 16; i++) {
        cipherText[i] = temp[i];
    }
}

uint8_t invGf(uint8_t hex, uint8_t byte) {
    uint8_t result = 0;
    while (byte) {
        if (byte & 1) {
            result ^= hex;
        }
        uint8_t highBit = hex & 0x80;
        hex = (hex << 1) & 0xFF;
        if (highBit) {
            hex ^= 0x1B;
        }
        byte >>= 1;
    }
    return result;

}

void invColumnsMixing(uint8_t* cipherText) {
    uint8_t temp[16];
    
    // Process each column
    for (int i = 0; i < 4; i++) {
        // Store the result in temp array
        temp[4 * i + 0] = invGf(0x0E, cipherText[4 * i + 0]) ^ 
                         invGf(0x0B, cipherText[4 * i + 1]) ^ 
                         invGf(0x0D, cipherText[4 * i + 2]) ^ 
                         invGf(0x09, cipherText[4 * i + 3]);
                         
        temp[4 * i + 1] = invGf(0x09, cipherText[4 * i + 0]) ^ 
                         invGf(0x0E, cipherText[4 * i + 1]) ^ 
                         invGf(0x0B, cipherText[4 * i + 2]) ^ 
                         invGf(0x0D, cipherText[4 * i + 3]);
                         
        temp[4 * i + 2] = invGf(0x0D, cipherText[4 * i + 0]) ^ 
                         invGf(0x09, cipherText[4 * i + 1]) ^ 
                         invGf(0x0E, cipherText[4 * i + 2]) ^ 
                         invGf(0x0B, cipherText[4 * i + 3]);
                         
        temp[4 * i + 3] = invGf(0x0B, cipherText[4 * i + 0]) ^ 
                         invGf(0x0D, cipherText[4 * i + 1]) ^ 
                         invGf(0x09, cipherText[4 * i + 2]) ^ 
                         invGf(0x0E, cipherText[4 * i + 3]);
    }
    
    // Copy result back to state array
    for (int i = 0; i < 16; i++) {
        cipherText[i] = temp[i];
    }
}