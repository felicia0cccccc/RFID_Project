#include "arduino_secrets.h"

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define SECTOR 1
#define ACCESS_BLOCK 4    // Block for storing encrypted access key
#define TRAILER_BLOCK 7   // Block for storing sector key (encrypted custom key)

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key defaultKey = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    Serial.println("Ready for initialization commands");
}

void loop() {
    if (Serial.available()) {
        String command = Serial.readStringUntil(',');
        
        if (command == "WRITE_SEC_KEY") {
            // Command for writing new sector key (encrypted custom key)
            handleWriteSectorKey();
        } else if (command == "WRITE_ACC_KEY") {
            // Command for writing encrypted access key
            handleWriteAccessKey();
        }
    }

    // Check for new card
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
        return;
    }

    // Send UID when new card is detected
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        if (mfrc522.uid.uidByte[i] < 0x10) uid += "0";
        uid += String(mfrc522.uid.uidByte[i], HEX);
    }
    Serial.println(uid);  // Send only UID for new card

    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    delay(500);
}

void handleWriteSectorKey() {
    String newKey = Serial.readStringUntil('\n');
    
    // Authenticate with default key first
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                TRAILER_BLOCK, &defaultKey, 
                                &(mfrc522.uid)) != MFRC522::STATUS_OK) {
        Serial.println("ERROR:AUTH_DEFAULT");
        return;
    }

    // Prepare sector trailer data
    byte trailerBlock[16];
    
    // First 6 bytes: Key A (encrypted custom key)
    for (byte i = 0; i < 6; i++) {
        trailerBlock[i] = strtol(newKey.substring(i*2, i*2+2).c_str(), NULL, 16);
    }
    
    // Access bits (default)
    trailerBlock[6] = 0xFF;
    trailerBlock[7] = 0x07;
    trailerBlock[8] = 0x80;
    trailerBlock[9] = 0x69;
    
    // Key B (optional, using same as Key A)
    for (byte i = 0; i < 6; i++) {
        trailerBlock[10 + i] = trailerBlock[i];
    }

    if (mfrc522.MIFARE_Write(TRAILER_BLOCK, trailerBlock, 16) == MFRC522::STATUS_OK) {
        Serial.println("OK:SEC_KEY");
    } else {
        Serial.println("ERROR:WRITE_SEC_KEY");
    }
}

void handleWriteAccessKey() {
    String encAccessKey = Serial.readStringUntil('\n');
    String sectorKey = Serial.readStringUntil('\n');
    
    // Convert sector key string to MIFARE key
    MFRC522::MIFARE_Key key;
    for (byte i = 0; i < 6; i++) {
        key.keyByte[i] = strtol(sectorKey.substring(i*2, i*2+2).c_str(), NULL, 16);
    }
    
    // Authenticate with sector key
    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 
                                TRAILER_BLOCK, &key, 
                                &(mfrc522.uid)) != MFRC522::STATUS_OK) {
        Serial.println("ERROR:AUTH_SEC_KEY");
        return;
    }

    // Write encrypted access key
    byte accessKeyData[16];
    for (byte i = 0; i < 16; i++) {
        accessKeyData[i] = strtol(encAccessKey.substring(i*2, i*2+2).c_str(), NULL, 16);
    }

    if (mfrc522.MIFARE_Write(ACCESS_BLOCK, accessKeyData, 16) == MFRC522::STATUS_OK) {
        Serial.println("OK:ACC_KEY");
    } else {
        Serial.println("ERROR:WRITE_ACC_KEY");
    }
}