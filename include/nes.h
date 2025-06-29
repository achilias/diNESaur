#pragma once

#include "cpu.h"
#include "ppu.h"
#include "bus.h"

class NES {
public:
    NES(std::ifstream &rom_stream) : rom(rom_stream), bus(rom), cpu(bus), ppu(bus) {
        
    };
    void run();

private:
    ROM rom;
    Bus bus;
    CPU cpu;
    PPU ppu;
};