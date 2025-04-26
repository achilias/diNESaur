#ifndef CPU_H
#define CPU_H

#include "memory.h"

class CPU {
public:
    CPU() : mem(65535), sp(0x1ff) {};
private:
    Memory mem;
    uint16_t pc;
    uint8_t accum;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint8_t sr;
};

#endif