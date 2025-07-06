#pragma once

#include "bus.h"

class PPU {
public:
    PPU(Bus& bus) : bus(bus) {
        for (size_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
            framebuffer[i] = 0xff000000;
    };
    bool run(size_t cycles);
    void draw_tile(uint32_t i, uint32_t loc_x, uint32_t loc_y);
    void draw_sprite(uint8_t sprite_n);
    uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
private:
    Bus& bus;
    size_t scanline_pixel = 0;
    size_t scanline_n = 0;
    uint8_t attr_tb_lookup(uint32_t tile_n);
};