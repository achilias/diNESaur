#pragma once

#include "bus.h"

#define VRAM_SIZE 16384
#define OAM_SIZE 256

class PPU;

void ppu_reset(PPU *ppu);
void ppu_init(PPU *ppu, Bus *bus, NES *nes);
uint8_t ppu_read_vram_byte(PPU *ppu, uint16_t addr);
uint16_t ppu_read_vram_two_bytes(PPU *ppu, uint16_t addr);
void ppu_write_vram_byte(PPU *ppu, uint16_t addr, uint8_t val);
void ppu_write_vram_two_bytes(PPU *ppu, uint16_t addr, uint16_t val);
uint8_t ppu_read_register(PPU *ppu, uint16_t addr);
void ppu_write_register(PPU *ppu, uint16_t addr, uint8_t val);

class PPU {
public:
    bool run(size_t cycles);
    void draw_tile(uint32_t tile_n, uint32_t base_x, uint32_t base_y);
    void draw_sprite(uint8_t sprite_n);
    Bus *bus;
    NES *nes;
    std::array<uint32_t, SCREEN_WIDTH * SCREEN_HEIGHT> framebuffer {};
    std::array<uint8_t, VRAM_SIZE> vram {};
    std::array<uint8_t, OAM_SIZE> oam {};
    uint8_t oam_addr { 0 };
    uint8_t ppu_ctrl { 0 };
    uint8_t ppu_mask { 0 };
    uint8_t ppu_status { 0 };
    uint16_t ppu_addr { 0 };
    bool ppu_w_reg { false };
    bool ignore_ctrl_writes { true };
    size_t scanline_pixel { 0 };
    size_t scanline_n { 0 };
private:
    uint8_t attr_tb_lookup(uint32_t tile_n);
};