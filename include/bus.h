#pragma once

#include <cstdint>
#include <array>

#include "rom.h"
#include "flags.h"

#define RAM_SIZE 65536
#define VRAM_SIZE 16384
#define OAM_SIZE 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

class Bus {
public:
    Bus(ROM& rom);
    uint8_t ram_read_byte(uint16_t addr);
    uint16_t ram_read_two_bytes(uint16_t addr);
    void ram_write_byte(uint16_t addr, uint8_t val);
    void ram_write_two_bytes(uint16_t addr, uint16_t val);

    uint8_t vram_read_byte(uint16_t addr) const;
    uint16_t vram_read_two_bytes(uint16_t addr) const;
    void vram_write_byte(uint16_t addr, uint8_t val);
    void vram_write_two_bytes(uint16_t addr, uint16_t val);

    void set_mapping(bool map_memory_nes);
    /* Map cpu memory according to NES memory map (mirroring, mapped ppu registers, etc.).
     * Disable to run regular 6502 cpu tests */
    bool map_memory_nes { true };

    bool nmi = false;
    size_t catchup_cycles = 0;

    // TODO: move to ppu. use friend classes
    PPUCtrl ppu_ctrl;
    PPUMask ppu_mask;
    PPUStatus ppu_status;
    uint8_t oam_addr = 0;
    uint16_t ppu_addr = 0;
    bool ppu_w_reg = 0;
    bool ignore_ctrl_writes = 1;

private:
    const ROM& rom;
    std::array<uint8_t, RAM_SIZE> ram;
    std::array<uint8_t, VRAM_SIZE> vram;
    std::array<uint8_t, OAM_SIZE> oam;



    static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {return addr >= start && addr <= end;};

    uint16_t ram_mirror(uint16_t addr) const;
    uint16_t vram_mirror(uint16_t addr) const;
};