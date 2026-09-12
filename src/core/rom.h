#pragma once

#include <stdint.h>
#include <stdio.h>

#define SIGNATURE "NES\x1A" // start of file magic bytes, ASCII string "NES" + "^Z" (msdos EOF)

enum MirrorMode {
    HORIZONTAL,
    VERTICAL
};

typedef struct ROM {
    uint32_t prg_size;
    uint32_t chr_size;
    uint8_t mapper;
    enum MirrorMode nt_mirror;
    uint8_t *prg_data;
    uint8_t *chr_data;
} ROM;

ROM *rom_create(FILE *stream);
void rom_destroy(ROM *rom);
uint8_t rom_read_byte_prg(const ROM *rom, uint16_t addr);
uint8_t rom_read_byte_chr(const ROM *rom, uint16_t addr);

