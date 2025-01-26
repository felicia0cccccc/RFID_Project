#ifndef DB_CONNECTION_H
#define DB_CONNECTION_H

#include <pqxx/pqxx>
#include <string>
#include <cstdint>  // For uint8_t

class DatabaseConnection {
private:
    pqxx::connection* conn;

public:
    DatabaseConnection(const std::string& conninfo);
    ~DatabaseConnection();

    void storeRFIDData(const std::string& uid, 
                       const std::string& customKey,
                       const std::string& accessKey,
                       const std::string& encryptionKey);

    bool hasAccess(const std::string& accessKey);
    void grantAccess(const std::string& accessKey); // Uncommented
    void revokeAccess(const std::string& accessKey); // Uncommented
    void updateCustomKey(const std::string& uid, const std::string& newCustomKey); // Uncommented
    std::string getCustomKey(const std::string& uid);
    std::string getEncryptionKey(const std::string& uid);
};

#endif // DB_CONNECTION_H
