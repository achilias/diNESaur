#pragma once

#include "bus.h"

class PPU;

void ppu_reset(PPU *ppu);
void ppu_init(PPU *ppu, Bus *bus);

class PPU {
public:
    bool run(size_t cycles);
    void draw_tile(uint32_t tile_n, uint32_t base_x, uint32_t base_y);
    void draw_sprite(uint8_t sprite_n);
    Bus *bus;
    std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer {};
    size_t scanline_pixel { 0 };
    size_t scanline_n { 0 };
private:
    uint8_t attr_tb_lookup(uint32_t tile_n);
};