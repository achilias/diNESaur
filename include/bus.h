#pragma once

#include <cstdint>
#include <array>

#define RAM_SIZE 65536
#define VRAM_SIZE 16383

class Bus {
public:
    Bus() {};
    uint8_t ram_read_byte(uint16_t addr) const { return ram[ram_mirror(addr)]; };
    uint16_t ram_read_two_bytes(uint16_t addr) const { return ((uint16_t) ram[ram_mirror(addr + 1)]) << 8 | ram[ram_mirror(addr)]; };
    void ram_write_byte(uint16_t addr, uint8_t val) { ram[ram_mirror(addr)] = val; };
    void ram_write_two_bytes(uint16_t addr, uint16_t val) { ram_write_byte(ram_mirror(addr), (uint8_t) (val & 0xFF)); ram_write_byte(ram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

    uint8_t vram_read_byte(uint16_t addr) const { return vram[vram_mirror(addr)]; };
    uint16_t vram_read_two_bytes(uint16_t addr) const { return ((uint16_t) vram[vram_mirror(addr + 1)]) << 8 | vram[vram_mirror(addr)]; };
    void vram_write_byte(uint16_t addr, uint8_t val) { vram[vram_mirror(addr)] = val; };
    void vram_write_two_bytes(uint16_t addr, uint16_t val) { vram_write_byte(vram_mirror(addr), (uint8_t) (val & 0xFF)); vram_write_byte(vram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

    void set_mirroring(bool mirror) {this->mirror = mirror;}


private:
    std::array<uint8_t, RAM_SIZE> ram;
    std::array<uint8_t, VRAM_SIZE> vram;
    bool mirror { true };
    uint16_t ram_mirror(uint16_t addr) const;
    uint16_t vram_mirror(uint16_t addr) const;
};