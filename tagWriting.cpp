#include "tagWriting.h"
#include <iostream>
#include <thread>
#include <chrono>

bool RFIDWriter::authenticateWithKey(const std::string& key) {
    // Clear any pending data first
    while (!arduino.readLine().empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Send authentication command
    std::string command = "AUTH," + key + "\n";
    
    if (!arduino.writeLine(command)) {
        std::cerr << "Failed to send auth command" << std::endl;
        return false;
    }
    
    // Wait for response with timeout
    auto startTime = std::chrono::steady_clock::now();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::string response = arduino.readLine();
        if (!response.empty()) {
            std::cout << "Received response: '" << response << "'" << std::endl;
            
            if (response.find("AUTH_OK") != std::string::npos) {
                return true;
            }
            if (response.find("AUTH_ERROR") != std::string::npos || 
                response.find("ERROR:NO_CARD") != std::string::npos) {
                return false;
            }
        }
        
        // Check timeout
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() > 2000) {
            std::cerr << "Authentication timeout" << std::endl;
            return false;
        }
    }
}

bool RFIDWriter::writeToBlock(uint8_t block, const std::string& data, const std::string& authKey) {
    
    std::string command;
    if (block == SECTOR_TRAILER) {
        command = "WRITE_TRAILER," + authKey + "," + data + "\n";
    } else {
        command = "WRITE_BLOCK," + authKey + "," + 
                 std::to_string(block) + "," + data + "\n";
    }
    
    // Add small delay before writing
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (!arduino.writeLine(command)) {
        std::cerr << "Failed to send write command" << std::endl;
        return false;
    }
    
    // Wait for response
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string response = arduino.readLine();
    std::cout << "Received response: '" << response << "'" << std::endl;
    
    return response.find("OK") != std::string::npos;
}

std::string RFIDWriter::readBlock(uint8_t block) {
    // Note: READ command not implemented in current Arduino code
    std::string command = "READ," + std::to_string(block) + "\n";
    if (!arduino.writeLine(command)) {
        return "";
    }
    
    return arduino.readLine();
}

bool RFIDWriter::writeInitialKeys(const std::string& encryptedCustomKey,
                                const std::string& encryptedAccessKey) {
    const std::string DEFAULT_KEY = "FFFFFFFFFFFF";  // lowercase for consistency
    
    std::cout << "\nStarting write process..." << std::endl;
    
    // Add delay after initial card detection
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Try authentication multiple times
    bool authenticated = false;
    for (int i = 0; i < 3; i++) {
        std::cout << "Authentication attempt " << (i + 1) << "..." << std::endl;
        if (authenticateWithKey(DEFAULT_KEY)) {
            authenticated = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    if (!authenticated) {
        std::cerr << "Failed to authenticate with default key after multiple attempts" << std::endl;
        return false;
    }
    
    std::cout << "Authentication successful!" << std::endl;
    
    // Write sector trailer (encrypted custom key)
    std::cout << "\nWriting sector trailer..." << std::endl;
    if (!writeToBlock(SECTOR_TRAILER, encryptedCustomKey, DEFAULT_KEY)) {
        std::cerr << "Failed to write sector key" << std::endl;
        return false;
    }
    std::cout << "Sector trailer written successfully" << std::endl;
    
    // Write encrypted access key
    std::cout << "\nWriting access key..." << std::endl;
    if (!writeToBlock(ACCESS_KEY_BLOCK, encryptedAccessKey, encryptedCustomKey)) {
        std::cerr << "Failed to write access key" << std::endl;
        return false;
    }
    std::cout << "Access key written successfully" << std::endl;
    
    return true;
}

bool RFIDWriter::readAccessKey(const std::string& customKey, std::string& accessKey) {
    std::cout << "Attempting to read access key..." << std::endl;
    
    // Authenticate with custom key
    if (!authenticateWithKey(customKey)) {
        std::cerr << "Failed to authenticate with custom key" << std::endl;
        return false;
    }
    
    // Read access key block
    accessKey = readBlock(ACCESS_KEY_BLOCK);
    return !accessKey.empty();
}

bool RFIDWriter::updateCustomKey(const std::string& currentCustomKey,
                               const std::string& newEncryptedCustomKey) {
    std::cout << "Attempting to update custom key..." << std::endl;
    
    // Authenticate with current custom key
    if (!authenticateWithKey(currentCustomKey)) {
        std::cerr << "Failed to authenticate with current custom key" << std::endl;
        return false;
    }
    
    // Write new custom key to sector trailer
    if (!writeToBlock(SECTOR_TRAILER, newEncryptedCustomKey, currentCustomKey)) {
        std::cerr << "Failed to write new custom key" << std::endl;
        return false;
    }
    
    return true;
}