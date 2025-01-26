#include "uidSerialCommunication.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>

std::string readArduino(SerialPort& arduino) {
    return arduino.readLine();
}

RFIDData parseRFIDData(const std::string& data) {
    RFIDData result;
    
    // Split the data by comma
    size_t commaPos = data.find(",");
    if (commaPos != std::string::npos) {
        // If we found a comma, extract just the UID part
        result.uid = data.substr(0, commaPos);
        std::string flag = data.substr(commaPos + 1);
        result.isInitialRead = (flag == "DEFAULT_OK");
    } else {
        // No comma means just a UID
        result.uid = data;
        result.isInitialRead = false;
    }
    
    std::cout << "Parsed UID: " << result.uid << std::endl;
    std::cout << "isInitialRead: " << (result.isInitialRead ? "true" : "false") << std::endl;
    
    return result;
}

RFIDData getRFIDDataFromArduino() {
    try {
        const std::string portName = "/dev/cu.usbmodem11401";
        SerialPort arduino(portName.c_str());

        if (!arduino.isConnected()) {
            throw std::runtime_error("Can't connect to Arduino at " + portName);
        }

        std::cout << "Connected to Arduino. Waiting for RFID tag..." << std::endl;

        while (true) {
            std::string data = readArduino(arduino);

            if (!data.empty()) {
                // Clean up the data (remove whitespace)
                data.erase(std::remove_if(data.begin(), data.end(), ::isspace), data.end());
                
                // Parse and validate the data
                RFIDData rfidData = parseRFIDData(data);
                
                // Validate UID format (hex string)
                if (rfidData.uid.find_first_not_of("0123456789ABCDEFabcdef") == std::string::npos) {
                    if (rfidData.isInitialRead) {
                        std::cout << "Initial read - UID: " << rfidData.uid << std::endl;
                    } else {
                        std::cout << "Existing card - UID: " << rfidData.uid << std::endl;
                    }
                    return rfidData;
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Arduino error: " + std::string(e.what()));
    }
}