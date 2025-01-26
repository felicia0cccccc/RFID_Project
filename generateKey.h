#ifndef GENERATE_KEY_H
#define GENERATE_KEY_H

#include <cstring>
#include "db_connection.h"

void generateKeyFromUID(const std::string& uid, DatabaseConnection& db);

#endif