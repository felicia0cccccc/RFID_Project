#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdint>
#include <cstddef>
#include "XOR.h"

void xorOperation(uint8_t* cipherText, const uint8_t* key, size_t length) {
    for (size_t i = 0; i < length; i++) {
        cipherText[i] ^= key[i];
    }
}

// test for output function
void printHex(const uint8_t* data, size_t length) {
    for (size_t i = 0; i < length; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
}

void xorOperationPrint(uint8_t* cipherText, const uint8_t* plainText, const uint8_t* key, size_t length) {
    xorOperation(cipherText, key, length);
    printHex(cipherText, length);
    xorOperation(cipherText, key, length);
    printHex(cipherText, length);
}