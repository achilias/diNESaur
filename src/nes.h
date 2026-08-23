#pragma once

#include "ppu.h"
#include "rom.h"
#include "controller.h"
#include "cpu.h"

#include <cstdint>
#include <array>

struct NES
{
    CPU *cpu;
    PPU *ppu;
    ROM *rom;
    Bus *bus;
    Controller *controller;
};

void nes_init(NES *nes);