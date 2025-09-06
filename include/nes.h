#pragma once

#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include "controller.h"

class NES {
public:
    NES(ROM const *rom, Controller& ctrl) : ctrl(ctrl), rom(rom), bus(rom, ctrl), cpu(bus), ppu(bus) {};
    void run();

private:
    Controller& ctrl;
    ROM const *rom;
    Bus bus;
    CPU cpu;
    PPU ppu;
};