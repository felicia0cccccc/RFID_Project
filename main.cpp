#include <openssl/rand.h>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
#include <thread>
#include <filesystem>
#include "encrypt.h"
#include "db_connection.h"
#include "uidSerialCommunication.h"
#include "tagWriting.h"
#include <nlohmann/json.hpp>

#define Nb 4
#define Nr 10
#define keySizeA 16
#define keySizeB 6

using json = nlohmann::json;
bool readKeysFromJson(const std::string& filename, 
                     std::string& encryptedCustomKey,
                     std::string& encryptedAccessKey) {
    try {
        // Read the file
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filename);
        }

        // Parse JSON
        json jsonData;
        file >> jsonData;

        // Extract the keys
        encryptedCustomKey = jsonData["encryptedCustomKey"].get<std::string>();
        encryptedAccessKey = jsonData["encryptedAccessKey"].get<std::string>();

        return true;
    } catch (const std::exception& e) {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

void generateKey(uint8_t* key, int length) {
    if (!RAND_bytes(key, length)) {
        std::cerr << "Error generating random key" << std::endl;
    }
}

void toHexString(uint8_t* data, int size, std::string &output) {
    std::ostringstream oss;
    for (int i = 0; i < size; i++) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
    }
    output = oss.str();
}

void textPadding(uint8_t *text, size_t textSize) {
    for (size_t i = textSize; i < 16; i++) {
        text[i] = text[i % textSize] ^ (i + 1);
    }
}

void generateKeyFromUID(const std::string& uid, DatabaseConnection& db) {
    uint8_t encryptionKey[keySizeA];
    uint8_t customKey[keySizeA];
    uint8_t accessKey[keySizeA];

    generateKey(encryptionKey, keySizeA);
    generateKey(customKey, keySizeB);
    generateKey(accessKey, keySizeA);

    textPadding(customKey, keySizeB);

    uint8_t encryptedCustomKey[keySizeA];
    uint8_t encryptedAccessKey[keySizeA];

    memcpy(encryptedCustomKey, customKey, keySizeA);
    memcpy(encryptedAccessKey, accessKey, keySizeA);

    AESEncryption(encryptedCustomKey, encryptionKey, keySizeA);
    AESEncryption(encryptedAccessKey, encryptionKey, keySizeA);

    // std::string encryptionKeyStr(reinterpret_cast<char*>(encryptionKey), keySizeA);
    // std::string customKeyStr(reinterpret_cast<char*>(customKey), keySizeA);
    // std::string accessKeyStr(reinterpret_cast<char*>(accessKey), keySizeA);

	uint8_t decryptedCustomKey[keySizeA];
    uint8_t decryptedAccessKey[keySizeA];
    memcpy(decryptedCustomKey, encryptedCustomKey, keySizeA);
    memcpy(decryptedAccessKey, encryptedAccessKey, keySizeA);
    AESDecryption(decryptedCustomKey, encryptionKey, keySizeA);
    AESDecryption(decryptedAccessKey, encryptionKey, keySizeA);

    std::string uidHex, encryptionKeyHex, customKeyHex, accessKeyHex;
    std::string encryptedCustomKeyHex, encryptedAccessKeyHex;
    std::string decryptedCustomKeyHex, decryptedAccessKeyHex;
    
    toHexString((uint8_t*)uid.c_str(), uid.size(), uidHex);
    toHexString(encryptionKey, keySizeA, encryptionKeyHex);
    toHexString(customKey, keySizeB, customKeyHex);
    toHexString(accessKey, keySizeA, accessKeyHex);
    toHexString(encryptedCustomKey, keySizeB, encryptedCustomKeyHex);
    toHexString(encryptedAccessKey, keySizeA, encryptedAccessKeyHex);
    toHexString(decryptedCustomKey, keySizeB, decryptedCustomKeyHex);
    toHexString(decryptedAccessKey, keySizeA, decryptedAccessKeyHex);


    // Store in database
    try {
        db.storeRFIDData(uid, customKeyHex, accessKeyHex, encryptionKeyHex);
        std::cout << "Keys stored in database successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error storing in database: " << e.what() << std::endl;
    }

	// Then write to output files
    std::ofstream outputKeyFile("outputKeys.json");
    if (!outputKeyFile.is_open()) {
        std::cerr << "Error: Could not create or open outputKeys.json" << std::endl;
        return;
    }
    outputKeyFile << "{\n";
    outputKeyFile << "  \"encryptedCustomKey\": \"" << encryptedCustomKeyHex << "\",\n";
    outputKeyFile << "  \"encryptedAccessKey\": \"" << encryptedAccessKeyHex << "\",\n";
    outputKeyFile << "  \"decryptedCustomKey\": \"" << decryptedCustomKeyHex << "\",\n";
    outputKeyFile << "  \"decryptedAccessKey\": \"" << decryptedAccessKeyHex << "\"\n";
    outputKeyFile << "}";
    outputKeyFile.close();

    std::cout << "Keys generated and saved to outputKeys.json" << std::endl;

    std::ofstream inputKeyFile("inputKeys.json");
    if (!inputKeyFile.is_open()) {
        std::cerr << "Error: Could not create or open inputKeys.json" << std::endl;
        return;
    }
    inputKeyFile << "{\n";
    inputKeyFile << "  \"UID\": \"" << uidHex << "\",\n";
    inputKeyFile << "  \"encryptionKey\": \"" << encryptionKeyHex << "\",\n";
    inputKeyFile << "  \"customKey\": \"" << customKeyHex << "\",\n";
    inputKeyFile << "  \"accessKey\": \"" << accessKeyHex << "\"\n";
    inputKeyFile << "}";
    inputKeyFile.close();

    std::cout << "Keys generated and saved to inputKeys.json" << std::endl;
}


int main() {
    try {
        // Connect to database
        DatabaseConnection db(
            "dbname=rfid_db "
            "user=rfid "
            "password=rfid123 "
            "host=localhost "
            "port=5432"
        );

        // Setup serial connection
        const std::string portName = "/dev/cu.usbmodem11401";
        SerialPort arduino(portName.c_str());
        RFIDWriter writer(arduino);

        while (true) {
            // Get RFID data
            RFIDData rfidData = getRFIDDataFromArduino();

            if (rfidData.isInitialRead) {
                // Generate new keys
                generateKeyFromUID(rfidData.uid, db);
                
                // Parse JSON (simplified for example)
                std::string encryptedCustomKey, encryptedAccessKey;
                if (!readKeysFromJson("outputKeys.json", encryptedCustomKey, encryptedAccessKey)) {
                    std::cerr << "Failed to read keys from JSON" << std::endl;
                    continue;
                }

                // Write keys to card
                if (!writer.writeInitialKeys(encryptedCustomKey, encryptedAccessKey)) {
                    std::cerr << "Failed to write initial keys to card" << std::endl;
                    continue;
                }
                
                std::cout << "Card initialized successfully" << std::endl;
            } else {
                // Verify the read data
				uint8_t encryptionKey[16];
				uint8_t customKey[16];

				std::string hexEncKey = db.getEncryptionKey(rfidData.uid);
				std::string hexCustomKey = db.getCustomKey(rfidData.uid);
                // Convert hex strings to byte arrays
				for (int i = 0; i < 16; i++) {
					encryptionKey[i] = std::stoi(hexEncKey.substr(i*2, 2), nullptr, 16);
					customKey[i] = std::stoi(hexCustomKey.substr(i*2, 2), nullptr, 16);
				}
                AESEncryption(customKey, encryptionKey, 16);
				std::string hexEncCustomKey;
				toHexString(customKey, 6, hexEncCustomKey);

                std::string encryptedAccessKey;

				if (writer.readAccessKey(hexEncCustomKey, encryptedAccessKey)) {
					uint8_t accessKey[16];
					// std::string hexAccKey = rfidData.encryptedAccessKey;
					for (int i = 0; i < 16; i++) {
						accessKey[i] = std::stoi(encryptedAccessKey.substr(i*2, 2), nullptr, 16);
					}

					AESDecryption(accessKey, encryptionKey, 16);
                    std::string hexAccKey;
					toHexString(accessKey, 16, hexAccKey);
					if (db.hasAccess(hexAccKey)) {
						std::cout << "Access granted!" << std::endl;
                        // Generate new custom key
                        uint8_t newCustomKey[16];
                        generateKey(newCustomKey, 6);

                        textPadding(newCustomKey, keySizeB);

                        uint8_t newEncCustomKey[16];
                        memcpy(newEncCustomKey, newCustomKey, 16);
                        AESEncryption(newEncCustomKey, encryptionKey, 16);
                        
                        std::string newEncCusKeyHex;
                        toHexString(newEncCustomKey, 6, newEncCusKeyHex);
                        // Update card with new custom key
                        if (!writer.updateCustomKey(hexEncCustomKey, 
                                                newEncCusKeyHex)) {
                            std::cerr << "Failed to update custom key" << std::endl;
                            continue;
                        }

                        // Update database
                        std::string newCustomKeyHex;
                        toHexString(newCustomKey, 16, newCustomKeyHex);
                        db.updateCustomKey(rfidData.uid, newCustomKeyHex);
                        
                        std::cout << "Session key updated successfully" << std::endl;
                    } else {
                        std::cout << "Access denied!" << std::endl;
                    }
				}
            }

            // Wait before next read
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
 
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}