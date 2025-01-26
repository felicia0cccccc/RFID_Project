#include <iostream>
#include <iomanip>
#include <cstring>
#include "keySchedule.h"
#include "sbox.h"

using namespace std;

#define Nb 4  // Number of columns (32-bit words) comprising the State. Nb=4 for AES
#define Nk 4  // Number of 32-bit words comprising the Cipher Key. Nk=4 for AES-128
#define Nr 10 // Number of rounds for AES-128

// Rcon (round constants used in key schedule)
uint8_t Rcon[11] = {0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

// SubWord: Apply S-box to each byte of the word
uint32_t SubWord(uint32_t word) {
    uint32_t result = 0;
    for (int i = 0; i < 4; i++) {
        uint8_t byte = (word >> (24 - i * 8)) & 0xFF;
        result |= sbox[byte] << (24 - i * 8);
    }
    return result;
}

// RotWord: Rotate a word (4 bytes) left by 1 byte
uint32_t RotWord(uint32_t word) {
    return (word << 8) | (word >> 24);
}

// Key Expansion function for AES-128
void keyExpansion(uint8_t* key, uint32_t* expandedKey) {
    uint32_t temp;

    // The first Nk words are just the key itself
    for (int i = 0; i < Nk; i++) {
        expandedKey[i] = (key[4 * i] << 24) | (key[4 * i + 1] << 16) | (key[4 * i + 2] << 8) | key[4 * i + 3];
    }

    // All subsequent words are based on previous words
    for (int i = Nk; i < Nb * (Nr + 1); i++) {
        temp = expandedKey[i - 1];
        if (i % Nk == 0) {
            temp = SubWord(RotWord(temp)) ^ (Rcon[i / Nk] << 24);
        }
        expandedKey[i] = expandedKey[i - Nk] ^ temp;
    }
}

// Print the expanded key in hexadecimal format
void printExpandedKey(uint32_t* expandedKey) {
    for (int i = 0; i < Nb * (Nr + 1); i++) {
        cout << hex << setw(8) << setfill('0') << expandedKey[i] << " ";
        if ((i + 1) % 4 == 0) cout << endl;
    }
}