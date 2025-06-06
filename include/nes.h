#pragma once

#include "cpu.h"
#include "ppu.h"
#include "bus.h"

class NES {
public:
    NES(std::ifstream &rom_stream) : cpu(bus), ppu(bus), bus(rom), rom(rom_stream) {
    };
    void run() {};

private:
    CPU cpu;
    PPU ppu;
    Bus bus;
    ROM rom;
};