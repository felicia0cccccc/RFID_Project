#ifndef TAG_WRITING_H
#define TAG_WRITING_H

#include "SerialPort.h"
#include <string>

class RFIDWriter {
private:
    SerialPort& arduino;
    static const uint8_t ACCESS_KEY_BLOCK = 4;
    static const uint8_t SECTOR_TRAILER = 7;
    
    bool authenticateWithKey(const std::string& key);
    bool writeToBlock(uint8_t block, const std::string& data, const std::string& authKey);
    std::string readBlock(uint8_t block);

public:
    RFIDWriter(SerialPort& serial) : arduino(serial) {}
    
    // Initialize new card
    bool writeInitialKeys(const std::string& encryptedCustomKey,
                         const std::string& encryptedAccessKey);
    
    // Read access key from existing card
    bool readAccessKey(const std::string& customKey, std::string& accessKey);
    
    // Update custom key after successful access
    bool updateCustomKey(const std::string& currentCustomKey,
                        const std::string& newEncryptedCustomKey);
};

#endif