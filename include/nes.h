#pragma once

#include "cpu.h"
#include "ppu.h"
#include "bus.h"

class NES {
public:
    NES() : cpu(bus), ppu(bus), bus() {
    };
    void run() {};

private:
    CPU cpu;
    PPU ppu;
    Bus bus;
};