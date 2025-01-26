#ifndef AES_ENCRYPTION_H
#define AES_ENCRYPTION_H

#include <cstdint>
#include <cstddef>

void AESEncryption(uint8_t* text, uint8_t* key, size_t textSize);

void AESDecryption(uint8_t* text, uint8_t* key, size_t textSize);

#endif