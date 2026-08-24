#pragma once

#include <cstdint>
#include <array>

#include "rom.h"
#include "flags.h"
#include "controller.h"

#define RAM_SIZE 65536

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

struct NES;

class Bus {
public:
    ROM const *rom;
    Controller *controller;
    NES *nes;

    /* Map cpu memory according to NES memory map (mirroring, mapped ppu registers, etc.).
     * Disable to run regular 6502 cpu tests */
    bool map_memory_nes { true };

    size_t catchup_cycles{ 0 };

    std::array<uint8_t, RAM_SIZE> ram {};
};

uint8_t ram_read_byte(Bus *bus, uint16_t addr);
uint16_t ram_read_two_bytes(Bus *bus, uint16_t addr);
void ram_write_byte(Bus *bus, uint16_t addr, uint8_t val);
void ram_write_two_bytes(Bus *bus, uint16_t addr, uint16_t val);
void bus_init(Bus *bus, ROM const *rom, Controller *controller);