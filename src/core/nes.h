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
    Controller *controller;
    bool nmi { false };
};

typedef void (*DrawingCallback)(uint32_t*);
typedef void (*InputPollingCallback)(Controller*, bool*);

void nes_init(NES *nes, std::ifstream& file);
void nes_run(NES *nes, DrawingCallback draw, InputPollingCallback poll_for_input);