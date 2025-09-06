#pragma once

#include <cstdint>
#include <array>

#include "rom.h"
#include "flags.h"
#include "controller.h"

#define RAM_SIZE 65536
#define VRAM_SIZE 16384
#define OAM_SIZE 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

class Bus {
public:
    Bus(ROM const *rom, Controller& ctrl) : rom(rom), ctrl(ctrl) {};
    uint8_t ram_read_byte(uint16_t addr);
    uint16_t ram_read_two_bytes(uint16_t addr);
    void ram_write_byte(uint16_t addr, uint8_t val);
    void ram_write_two_bytes(uint16_t addr, uint16_t val);

    uint8_t vram_read_byte(uint16_t addr) const;
    uint16_t vram_read_two_bytes(uint16_t addr) const;
    void vram_write_byte(uint16_t addr, uint8_t val);
    void vram_write_two_bytes(uint16_t addr, uint16_t val);

    uint8_t oam_read_byte(uint16_t addr) const;

    void set_mapping(bool map_memory_nes);

    inline void reset() {
        std::fill(ram.begin(), ram.end(), 0);
        std::fill(vram.begin(), vram.end(), 0);
        std::fill(oam.begin(), oam.end(), 0);
        nmi = false;
        catchup_cycles = 0;
        ppu_ctrl = 0;
        ppu_mask = 0;
        ppu_status = 0;
        oam_addr = 0;
        ppu_addr = 0;
        ppu_w_reg = false;
        ignore_ctrl_writes = true;
    }

    ROM const *rom;
    Controller& ctrl;

    /* Map cpu memory according to NES memory map (mirroring, mapped ppu registers, etc.).
     * Disable to run regular 6502 cpu tests */
    bool map_memory_nes { true };

    bool nmi { false };
    size_t catchup_cycles{ 0 };

    uint8_t ppu_ctrl{ 0 };
    uint8_t ppu_mask{ 0 };
    uint8_t ppu_status{ 0 };
    uint8_t oam_addr{ 0 };
    uint16_t ppu_addr{ 0 };
    bool ppu_w_reg{ false };
    bool ignore_ctrl_writes{ true };
    static uint16_t ram_mirror(uint16_t addr);

    uint16_t vram_mirror(uint16_t addr) const;
private:
    std::array<uint8_t, RAM_SIZE> ram {};
    std::array<uint8_t, VRAM_SIZE> vram {};
    std::array<uint8_t, OAM_SIZE> oam {};



    static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {return addr >= start && addr <= end;};

};