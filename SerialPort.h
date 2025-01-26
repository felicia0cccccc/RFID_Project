#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <string>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

class SerialPort {
private:
    int handler;
    bool connected;

public:
    SerialPort(const char* portName) {
        connected = false;
        handler = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
        
        if (handler == -1) return;

        struct termios options;
        tcgetattr(handler, &options);
        
        // Set baud rate
        cfsetispeed(&options, B9600);
        cfsetospeed(&options, B9600);
        
        // Configure other settings
        options.c_cflag &= ~PARENB;          // No parity
        options.c_cflag &= ~CSTOPB;          // 1 stop bit
        options.c_cflag &= ~CSIZE;           // Mask the character size bits
        options.c_cflag |= CS8;              // 8 bit data
        options.c_cflag &= ~CRTSCTS;         // No hardware flow control
        options.c_cflag |= CREAD | CLOCAL;   // Enable receiver, ignore modem controls
        
        options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
        options.c_oflag &= ~OPOST;
        
        // Set timeout
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 10;
        
        tcsetattr(handler, TCSANOW, &options);
        connected = true;
    }

    ~SerialPort() {
        if (connected) {
            close(handler);
        }
    }

    bool writeLine(const std::string& data) {
        if (!connected) return false;
        
        int bytes = write(handler, data.c_str(), data.length());
        return bytes == static_cast<int>(data.length());
    }

    std::string readLine() {
        if (!connected) return "";
        
        char buffer[1024] = {0};
        int bytes = read(handler, buffer, sizeof(buffer)-1);
        
        if (bytes > 0) {
            return std::string(buffer, bytes);
        }
        return "";
    }

    bool isConnected() const {
        return connected;
    }
};

#endif