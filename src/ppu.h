#pragma once

#include "bus.h"

#define VRAM_SIZE 16384

class PPU;

void ppu_reset(PPU *ppu);
void ppu_init(PPU *ppu, Bus *bus, NES *nes);
uint8_t ppu_read_vram_byte(PPU *ppu, uint16_t addr);
uint16_t ppu_read_vram_two_bytes(PPU *ppu, uint16_t addr);
void ppu_write_vram_byte(PPU *ppu, uint16_t addr, uint8_t val);
void ppu_write_vram_two_bytes(PPU *ppu, uint16_t addr, uint16_t val);

class PPU {
public:
    bool run(size_t cycles);
    void draw_tile(uint32_t tile_n, uint32_t base_x, uint32_t base_y);
    void draw_sprite(uint8_t sprite_n);
    Bus *bus;
    NES *nes;
    std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer {};
    std::array<uint8_t, VRAM_SIZE> vram {};
    size_t scanline_pixel { 0 };
    size_t scanline_n { 0 };
private:
    uint8_t attr_tb_lookup(uint32_t tile_n);
};