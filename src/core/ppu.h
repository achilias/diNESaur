#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "flags.h"

#define VRAM_SIZE 16384
#define OAM_SIZE 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

struct NES;
typedef struct PPU PPU;

struct PPU {
    struct NES *nes;
    uint32_t framebuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint8_t vram[VRAM_SIZE];
    uint8_t oam[OAM_SIZE];
    uint8_t oam_addr;
    uint8_t ppu_ctrl;
    uint8_t ppu_mask;
    uint8_t ppu_status;
    uint16_t ppu_addr;
    bool ppu_w_reg;
    bool ignore_ctrl_writes;
    size_t scanline_pixel;
    size_t scanline_n;
};

#ifdef __cplusplus
extern "C" {
#endif

PPU *ppu_create(struct NES *nes);
bool ppu_run(PPU *ppu, size_t cycles);
void ppu_draw_tile(PPU *ppu, uint32_t tile_n, uint32_t base_x, uint32_t base_y);
void ppu_draw_sprite(PPU *ppu, uint8_t sprite_n);
uint8_t ppu_read_vram_byte(PPU *ppu, uint16_t addr);
uint16_t ppu_read_vram_two_bytes(PPU *ppu, uint16_t addr);
void ppu_write_vram_byte(PPU *ppu, uint16_t addr, uint8_t val);
void ppu_write_vram_two_bytes(PPU *ppu, uint16_t addr, uint16_t val);
uint8_t ppu_read_register(PPU *ppu, uint16_t addr);
void ppu_write_register(PPU *ppu, uint16_t addr, uint8_t val);

#ifdef __cplusplus
}
#endif
