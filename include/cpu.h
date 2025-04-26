#ifndef CPU_H
#define CPU_H

#include "memory.h"

class CPU {
public:
    CPU() : mem(65536), sp(0x1ff) {};
    size_t run();
private:
    Memory mem;
    uint16_t pc;
    uint8_t accum;
    uint8_t x;
    uint8_t y;
    uint8_t sp;
    uint8_t sr;

    size_t execute_instr();


};

#endif