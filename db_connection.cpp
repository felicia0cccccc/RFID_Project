#include "db_connection.h"
#include <iostream>
#include <string>

DatabaseConnection::DatabaseConnection(const std::string& conninfo) {
    conn = new pqxx::connection(conninfo);
}

DatabaseConnection::~DatabaseConnection() {
    delete conn;
}

void DatabaseConnection::storeRFIDData(const std::string& uid, 
                                       const std::string& customKey,
                                       const std::string& accessKey,
                                       const std::string& encryptionKey) {
    try {
        pqxx::work txn(*conn);
        
        txn.exec_params(
            "INSERT INTO rfid_keys (uid, custom_key, access_key, encryption_key) "
            "VALUES ($1, $2, $3, $4) "
            "ON CONFLICT (uid) DO UPDATE SET "
            "custom_key = $2, "
            "access_key = $3, "
            "encryption_key = $4",
            uid, 
            customKey, 
            accessKey, 
            encryptionKey);
        
        txn.commit();
        std::cout << "Stored RFID data for UID: " << uid << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error storing data: " << e.what() << std::endl;
        throw;
    }
}

bool DatabaseConnection::hasAccess(const std::string& accessKey) {
    try {
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec_params(
            "SELECT 1 FROM permitted_access "
            "WHERE access_key = $1 AND access_granted = true",
            accessKey);
            
        txn.commit();
        return !r.empty();
    } catch (const std::exception& e) {
        std::cerr << "Error checking access: " << e.what() << std::endl;
        throw;
    }
}

void DatabaseConnection::grantAccess(const std::string& accessKey) {
    try {
        pqxx::work txn(*conn);
        
        txn.exec_params(
            "INSERT INTO permitted_access (access_key, access_granted) "
            "VALUES ($1, true) "
            "ON CONFLICT (access_key) DO UPDATE SET "
            "access_granted = true, "
            "access_time = CURRENT_TIMESTAMP",
            accessKey);
            
        txn.commit();
        std::cout << "Access granted for Access Key." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error granting access: " << e.what() << std::endl;
        throw;
    }
}

void DatabaseConnection::revokeAccess(const std::string& accessKey) {
    try {
        pqxx::work txn(*conn);
        
        txn.exec_params(
            "UPDATE permitted_access "
            "SET access_granted = false "
            "WHERE access_key = $1",
            accessKey);
            
        txn.commit();
        std::cout << "Access revoked for Access Key." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error revoking access: " << e.what() << std::endl;
        throw;
    }
}

void DatabaseConnection::updateCustomKey(const std::string& uid, const std::string& newCustomKey) {
    try {
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec_params(
            "UPDATE rfid_keys "
            "SET custom_key = $2 "
            "WHERE uid = $1 "
            "RETURNING uid",
            uid, newCustomKey);
        
        txn.commit();
        
        if (r.empty()) {
            throw std::runtime_error("UID not found: " + uid);
        }
        
        std::cout << "Custom key updated for UID: " << uid << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error updating custom key: " << e.what() << std::endl;
        throw;
    }
}

std::string DatabaseConnection::getCustomKey(const std::string& uid) {
    try {
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec_params(
            "SELECT custom_key FROM rfid_keys WHERE uid = $1",
            uid);
            
        txn.commit();
        
        if (r.empty()) {
            throw std::runtime_error("UID not found: " + uid);
        }
        
        return r[0][0].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error getting custom key: " << e.what() << std::endl;
        throw;
    }
}

std::string DatabaseConnection::getEncryptionKey(const std::string& uid) {
    try {
        pqxx::work txn(*conn);
        
        pqxx::result r = txn.exec_params(
            "SELECT encryption_key FROM rfid_keys WHERE uid = $1",
            uid);
            
        txn.commit();
        
        if (r.empty()) {
            throw std::runtime_error("UID not found: " + uid);
        }
        
        return r[0][0].as<std::string>();
    } catch (const std::exception& e) {
        std::cerr << "Error getting encryption key: " << e.what() << std::endl;
        throw;
    }
}
