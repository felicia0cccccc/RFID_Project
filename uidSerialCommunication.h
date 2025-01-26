#ifndef UID_SERIAL_COMMUNICATION_H
#define UID_SERIAL_COMMUNICATION_H

#include <string>
#include "SerialPort.h"

struct RFIDData {
    std::string uid;
    bool isInitialRead;
};

// Function declarations
std::string readArduino(SerialPort& arduino);
RFIDData parseRFIDData(const std::string& data);
RFIDData getRFIDDataFromArduino();

#endif