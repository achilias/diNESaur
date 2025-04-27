#ifndef NES_H
#define NES_H

#include "cpu.h"
#include "ppu.h"

class NES {
public:
    NES() {};
    void run() {};

private:
    CPU cpu;
    PPU ppu;
};

#endif