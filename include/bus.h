#pragma once

#include <cstdint>
#include <array>

#include "rom.h"

#define RAM_SIZE 65536
#define VRAM_SIZE 16383
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

class Bus {
public:
    Bus(ROM& rom) : rom(rom) {};
    uint8_t ram_read_byte(uint16_t addr) const {
        if(in_range(addr, 0x8000, 0xffff) && map_memory_nes)
            return rom.read_byte_prg(ram_mirror(addr) - 0x8000);
        return ram[ram_mirror(addr)];
    };
    uint16_t ram_read_two_bytes(uint16_t addr) const { return (((uint16_t) ram_read_byte(addr + 1)) << 8) | ram_read_byte(addr); };
    void ram_write_byte(uint16_t addr, uint8_t val) { ram[ram_mirror(addr)] = val; };
    void ram_write_two_bytes(uint16_t addr, uint16_t val) { ram_write_byte(ram_mirror(addr), (uint8_t) (val & 0xFF)); ram_write_byte(ram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

    uint8_t vram_read_byte(uint16_t addr) const {
        if (in_range(addr, 0x0, 0x2000))
            return rom.read_byte_chr(vram_mirror(addr));
        return vram[vram_mirror(addr)];
    };
    uint16_t vram_read_two_bytes(uint16_t addr) const { return ((uint16_t) vram[vram_mirror(addr + 1)]) << 8 | vram[vram_mirror(addr)]; };
    void vram_write_byte(uint16_t addr, uint8_t val) { vram[vram_mirror(addr)] = val; };
    void vram_write_two_bytes(uint16_t addr, uint16_t val) { vram_write_byte(vram_mirror(addr), (uint8_t) (val & 0xFF)); vram_write_byte(vram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

    void set_mapping(bool map_memory_nes) {this->map_memory_nes = map_memory_nes;}
private:
    std::array<uint8_t, RAM_SIZE> ram;
    std::array<uint8_t, VRAM_SIZE> vram;

    ROM& rom;
    bool map_memory_nes { true }; // map memory according to nes memory map. disable for cpu testing

    static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {return addr >= start && addr <= end;};

    uint16_t ram_mirror(uint16_t addr) const;
    uint16_t vram_mirror(uint16_t addr) const;
};