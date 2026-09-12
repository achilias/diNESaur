#include "rom.h"
#include <stdlib.h>
#include <string.h>

ROM *rom_create(FILE *stream) {
    if (!stream || ferror(stream)) {
        fprintf(stderr, "Error reading file\n");
    }

    fseek(stream, 0, SEEK_END);
    size_t file_size = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    char *buf = malloc(file_size);
    fread(buf, 1, file_size, stream);

    if (memcmp(buf, SIGNATURE, 4) != 0)
        fprintf(stderr, "ROM file not of iNES type\n");

    ROM *rom = malloc(sizeof(*rom));
    rom->prg_size = buf[4] * 16384;
    rom->chr_size = buf[5] * 8192;
    rom->nt_mirror = (enum MirrorMode)(buf[6] & 0x1);
    rom->mapper = (buf[7] & 0xF0) | (buf[6] >> 4);

    rom->prg_data = malloc(rom->prg_size);
    rom->chr_data = malloc(rom->chr_size);
    for (int i = 0; i < rom->prg_size; i++) {
        rom->prg_data[i] = buf[0x10 + i];
    }
    for (int i = 0; i < rom->chr_size; i++) {
        rom->chr_data[i] = buf[0x10 + i + rom->prg_size];
    }

    free(buf);
    return rom;
}

void rom_destroy(ROM *rom) {
    free(rom->prg_data);
    free(rom->chr_data);
    free(rom);
}

uint8_t rom_read_byte_chr(const ROM *rom, uint16_t addr) {
    return rom->chr_data[addr];
}

uint8_t rom_read_byte_prg(const ROM *rom, uint16_t addr) {
    switch (rom->mapper) {
        case 0:
            return rom->prg_data[addr % 0x4000];
    }
    return rom->prg_data[addr % 0x4000];
}
