#pragma once

#include "bus.h"

class PPU {
public:
    explicit PPU(Bus& bus) : bus(bus) {
        framebuffer.fill(0xff000000);
    };
    bool run(size_t cycles);
    void draw_tile(uint32_t tile_n, uint32_t base_x, uint32_t base_y);
    void draw_sprite(uint8_t sprite_n);
    void reset();
    std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer {};
private:
    Bus& bus;
    size_t scanline_pixel = 0;
    size_t scanline_n = 0;
    uint8_t attr_tb_lookup(uint32_t tile_n);
};