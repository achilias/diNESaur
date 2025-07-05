#pragma once

#include "cpu.h"
#include "ppu.h"
#include "bus.h"

class NES {
public:
    NES(ROM &rom) : rom(rom), bus(*new Bus(rom)), cpu(*new CPU(bus)), ppu(*new PPU(bus)) {

    };
    void run();

private:
    ROM& rom;
    Bus& bus;
    CPU& cpu;
    PPU& ppu;
};