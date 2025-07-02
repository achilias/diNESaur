#pragma once

#include <cstdint>
#include <array>

#include "rom.h"
#include "flags.h"

#define RAM_SIZE 65536
#define VRAM_SIZE 16383
#define OAM_SIZE 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

class Bus {
public:
    Bus(ROM& rom) : rom(rom) {
        ppu_ctrl.raw = 0;
        ppu_mask.raw = 0;
        ppu_status.raw = 0;
    };
    uint8_t ram_read_byte(uint16_t addr) {
        if (!map_memory_nes)
            return ram[addr];

        switch (addr) {
            case 0x2002: {
                uint8_t tmp = ppu_status.raw;
                ppu_status.vblank = false;
                return tmp;
            }
            case 0x2004:
                /* TODO: reads to OAMDATA should have different behavior during rendering
                 * (i.e. not during vblanks), few games use this, but should be handled for accuracy. */
                return oam[oam_addr];
            case 0x2007: {
                uint16_t old = ppu_addr;
                ppu_addr += ppu_ctrl.incr ? 32 : 1;
                return vram_read_byte(old);
            }
        }

        if(in_range(addr, 0x8000, 0xffff))
            return rom.read_byte_prg(ram_mirror(addr) - 0x8000);

        return ram[ram_mirror(addr)];
    };
    uint16_t ram_read_two_bytes(uint16_t addr) { return (((uint16_t) ram_read_byte(addr + 1)) << 8) | ram_read_byte(addr); };
    void ram_write_byte(uint16_t addr, uint8_t val) {
        if (!map_memory_nes) {
            ram[addr] = val;
            return;
        }
        
        switch (addr) {
            case 0x2000:
                ppu_ctrl.raw = val;
                return;
            case 0x2001:
                ppu_mask.raw = val;
                return;
            case 0x2003:
                oam_addr = val;
                return;
            case 0x2004:
                /* TODO: writes to OAMDATA should have different behavior during rendering
                * (i.e. not during vblanks), probably completely ignored. */
                oam[oam_addr++] = val;
                return;
            case 0x2005:
                /* TODO: implement scrolling */
                return;
            case 0x2006:
                if (!ppu_w_reg)
                    ppu_addr = ((uint16_t) val) << 8;
                else
                    ppu_addr |= val;

                ppu_w_reg = !ppu_w_reg;
                return;
            case 0x2007:
                uint16_t old = ppu_addr;
                ppu_addr += ppu_ctrl.incr ? 32 : 1;
                vram_write_byte(old, val);
                return;
        }
        ram[ram_mirror(addr)] = val;
    };
    void ram_write_two_bytes(uint16_t addr, uint16_t val) { ram_write_byte(ram_mirror(addr), (uint8_t) (val & 0xFF)); ram_write_byte(ram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

    uint8_t vram_read_byte(uint16_t addr) const {
        if (in_range(addr, 0x0, 0x2000))
            return rom.read_byte_chr(vram_mirror(addr));
        return vram[vram_mirror(addr)];
    };
    uint16_t vram_read_two_bytes(uint16_t addr) const { return ((uint16_t) vram[vram_mirror(addr + 1)]) << 8 | vram[vram_mirror(addr)]; };
    void vram_write_byte(uint16_t addr, uint8_t val) { vram[vram_mirror(addr)] = val; };
    void vram_write_two_bytes(uint16_t addr, uint16_t val) { vram_write_byte(vram_mirror(addr), (uint8_t) (val & 0xFF)); vram_write_byte(vram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

    void set_mapping(bool map_memory_nes) {this->map_memory_nes = map_memory_nes;}
    /* Map cpu memory according to NES memory map (mirroring, mapped ppu registers, etc.).
     * Disable to run regular 6502 cpu tests */
    bool map_memory_nes { true };
private:
    std::array<uint8_t, RAM_SIZE> ram;
    std::array<uint8_t, VRAM_SIZE> vram;
    std::array<uint8_t, OAM_SIZE> oam;
    PPUCtrl ppu_ctrl;
    PPUMask ppu_mask;
    PPUStatus ppu_status;
    uint8_t oam_addr = 0;
    uint16_t ppu_addr = 0;
    bool ppu_w_reg = 0;

    ROM& rom;


    static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {return addr >= start && addr <= end;};

    uint16_t ram_mirror(uint16_t addr) const;
    uint16_t vram_mirror(uint16_t addr) const;
};