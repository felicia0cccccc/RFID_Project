#ifndef KEY_SCHEDULE_H
#define KEY_SCHEDULE_H

#include <cstdint>

void keyExpansion(uint8_t* key, uint32_t* expandedKey);

#endif