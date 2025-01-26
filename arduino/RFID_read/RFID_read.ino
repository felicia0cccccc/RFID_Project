#include "arduino_secrets.h"

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key defaultKey = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
}

bool ensureCardPresent() {
    mfrc522.PCD_StopCrypto1();
    
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return false;
    }
    
    if (!mfrc522.PICC_ReadCardSerial()) {
        return false;
    }
    
    return true;
}

void loop() {
    static bool processingCommand = false;
    
    // Check for commands first
    if (Serial.available()) {
        processingCommand = true;
        String command = Serial.readStringUntil(',');
        command.trim();
        
        if (command == "AUTH") {
            if (!ensureCardPresent()) {
                Serial.println("ERROR:NO_CARD");
                processingCommand = false;
                return;
            }
            
            String keyStr = Serial.readStringUntil('\n');
            keyStr.trim();
            
            MFRC522::MIFARE_Key key;
            
            // Convert hex string to key bytes
            for (byte i = 0; i < 6; i++) {
                key.keyByte[i] = strtol(keyStr.substring(i*2, i*2+2).c_str(), NULL, 16);
            }
            
            // Try to authenticate
            MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
                MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                7,  // sector trailer block
                &key,
                &(mfrc522.uid)
            );
            
            if (status == MFRC522::STATUS_OK) {
                Serial.println("AUTH_OK");
            } else {
                Serial.println("AUTH_ERROR");
            }
            
            processingCommand = false;
            return;
        } 
        else if (command == "WRITE_TRAILER" || command == "WRITE_BLOCK") {
            // Need authentication key first
            String keyStr = Serial.readStringUntil(',');  // Read key before other parameters
            keyStr.trim();
            
            MFRC522::MIFARE_Key key;
            // Convert authentication key
            for (byte i = 0; i < 6; i++) {
                key.keyByte[i] = strtol(keyStr.substring(i*2, i*2+2).c_str(), NULL, 16);
            }
            
            // Verify card presence and authenticate
            if (!ensureCardPresent()) {
                Serial.println("ERROR:NO_CARD");
                processingCommand = false;
                return;
            }
            
            // Authenticate first
            MFRC522::StatusCode authStatus = mfrc522.PCD_Authenticate(
                MFRC522::PICC_CMD_MF_AUTH_KEY_A,
                7,  // sector trailer block
                &key,
                &(mfrc522.uid)
            );
            
            if (authStatus != MFRC522::STATUS_OK) {
                Serial.println("ERROR:AUTH_FAILED");
                processingCommand = false;
                return;
            }
            
            if (command == "WRITE_TRAILER") {
                String data = Serial.readStringUntil('\n');
                byte trailerData[16];
                
                // Convert hex string to bytes
                for (byte i = 0; i < 16; i++) {
                    trailerData[i] = strtol(data.substring(i*2, i*2+2).c_str(), NULL, 16);
                }
                
                MFRC522::StatusCode status = mfrc522.MIFARE_Write(7, trailerData, 16);
                if (status == MFRC522::STATUS_OK) {
                    Serial.println("OK");
                } else {
                    Serial.println("ERROR");
                }
            }
            else if (command == "WRITE_BLOCK") {
                String blockStr = Serial.readStringUntil(',');
                String data = Serial.readStringUntil('\n');
                byte blockAddr = strtol(blockStr.c_str(), NULL, 10);
                byte blockData[16];
                
                // Convert hex string to bytes
                for (byte i = 0; i < 16; i++) {
                    blockData[i] = strtol(data.substring(i*2, i*2+2).c_str(), NULL, 16);
                }
                
                MFRC522::StatusCode status = mfrc522.MIFARE_Write(blockAddr, blockData, 16);
                if (status == MFRC522::STATUS_OK) {
                    Serial.println("OK");
                } else {
                    Serial.println("ERROR");
                }
            }
            
            processingCommand = false;
            return;
        }
        processingCommand = false;
    }

    // Only do card detection if not processing a command
    if (!processingCommand) {
        if (!ensureCardPresent()) {
            return;
        }

        // Get UID
        String uid = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
            if (mfrc522.uid.uidByte[i] < 0x10) {
                uid += "0";
            }
            uid += String(mfrc522.uid.uidByte[i], HEX);
        }

        // Try to authenticate with default key
        MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
            MFRC522::PICC_CMD_MF_AUTH_KEY_A,
            7,
            &defaultKey,
            &(mfrc522.uid)
        );

        if (status == MFRC522::STATUS_OK) {
            Serial.print(uid);
            Serial.println(",DEFAULT_OK");
        } else {
            Serial.println(uid);
            mfrc522.PICC_HaltA();
            mfrc522.PCD_StopCrypto1();
        }
    }
}