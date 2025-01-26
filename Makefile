# Project - AES Encryption Program
CXX = g++
CXXFLAGS = -Wall -Werror -g -std=c++17 -arch arm64
INCLUDES = -I/opt/homebrew/opt/openssl@3/include -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -L/opt/homebrew/lib

# Executable name
TARGET = aes_program

# Source files
# SRCS = main.cpp encrypt.cpp XOR.cpp keySchedule.cpp subBytes.cpp shiftRows.cpp mixColumns.cpp sbox.cpp db_connection.cpp uidSerialCommunication.cpp tagWriting.cpp
SRCS = main.cpp aesEncryption/encrypt.cpp aesEncryption/XOR.cpp aesEncryption/keySchedule.cpp aesEncryption/subBytes.cpp aesEncryption/shiftRows.cpp aesEncryption/mixColumns.cpp aesEncryption/sbox.cpp db_connection.cpp uidSerialCommunication.cpp tagWriting.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
.PHONY: all
all: $(TARGET)

# Link all object files to create the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) -lssl -lcrypto -lpqxx -lpq

# Compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean up object files and the executable
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)