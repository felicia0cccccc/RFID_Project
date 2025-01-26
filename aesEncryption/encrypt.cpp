#include <cstdint>
#include <cstring>
#include <cstddef>
#include "XOR.h"
#include "keySchedule.h"
#include "subBytes.h"
#include "shiftRows.h"
#include "mixColumns.h"
#include "encrypt.h"

void extractKey(uint32_t expandedKey[], uint8_t* roundKey, int round) {
	int index = 4 * round;
	for (int i = 0; i < 4; i++) {
		roundKey[4 * i] = (expandedKey[index] >> 24) & 0xFF;
		roundKey[4 * i + 1] = (expandedKey[index] >> 16) & 0xFF;
		roundKey[4 * i + 2] = (expandedKey[index] >> 8) & 0xFF;
		roundKey[4 * i + 3] = expandedKey[index] & 0xFF;
		index++;
	}
}

void AESEncryption(uint8_t* text, uint8_t* key, size_t textSize) {
	// Key Schedule
	uint32_t expandedEncryptionKey[4 * (10 + 1)];
	keyExpansion(key, expandedEncryptionKey);

    for (size_t offset = 0; offset < textSize; offset += 16) {
        uint8_t cipherText[16];
        size_t currentSize = (textSize - offset < 16) ? textSize - offset : 16;

        memcpy(cipherText, text + offset, currentSize);

        // round 0
        xorOperation(cipherText, key, 16);

        uint8_t roundNKey[16];

        // round 1 - 9
        for (int i = 1; i < 10; i++) {
            extractKey(expandedEncryptionKey, roundNKey, i);
            bytesSubtitution(cipherText);
            rowShifting(cipherText);
            columnsMixing(cipherText);
            xorOperation(cipherText, roundNKey, 16);
        }

        // round 10
        extractKey(expandedEncryptionKey, roundNKey, 10);
        bytesSubtitution(cipherText);
        rowShifting(cipherText);
        xorOperation(cipherText, roundNKey, 16);

        memcpy(text + offset, cipherText, currentSize);
    }
}

void AESDecryption(uint8_t* text, uint8_t* key, size_t textSize) {
    // Key Schedule
	uint32_t expandedEncryptionKey[4 * (10 + 1)];
	keyExpansion(key, expandedEncryptionKey);

    for (size_t offset = 0; offset < textSize; offset += 16) {
        uint8_t plainText[16];
        size_t currentSize = (textSize - offset < 16) ? textSize - offset : 16;

        memcpy(plainText, text + offset, currentSize);

        // round 10
        uint8_t roundNKey[16];
        extractKey(expandedEncryptionKey, roundNKey, 10);

        xorOperation(plainText, roundNKey, 16);

        // round 9 - 1
        for (int i = 9; i > 0; i--) {
            extractKey(expandedEncryptionKey, roundNKey, i);
            invRowShifting(plainText);
            invBytesSubstitution(plainText);
            xorOperation(plainText, roundNKey, 16);
            invColumnsMixing(plainText);
        }

        // round 0
        invRowShifting(plainText);
        invBytesSubstitution(plainText);
        xorOperation(plainText, key, 16);

        memcpy(text + offset, plainText, currentSize);
    }
}