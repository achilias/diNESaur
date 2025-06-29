#pragma once

#include "bus.h"

class PPU {
public:
    PPU(Bus& bus) : bus(bus) {};
    void draw_tile(uint32_t i, uint32_t loc_x, uint32_t loc_y);
    uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
private:
    Bus& bus;
};