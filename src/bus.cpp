#include "bus.h"


Bus::Bus(ROM& rom) : rom(rom) {
    ppu_ctrl.raw = 0;
    ppu_mask.raw = 0;
    ppu_status.raw = 0;
    ppu_ctrl.vblank_enable = 1;
};

uint16_t Bus::ram_mirror(uint16_t addr) const {
    if (in_range(addr, 0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(addr, 0x2000, 0x3fff))
        return addr & 0x2007;
    if (in_range(addr, 0x8000, 0xffff)) {
        // for 16kb (0x4000) cartridges, upper 16kb of 32kb address space must be mirrored to lower 16kb
        return addr % rom.prg_size;
    }
    return addr;
}

uint16_t Bus::vram_mirror(uint16_t addr) const {
    if (in_range(addr, 0x0, 0x3eff))
        return rom.chr_size == 0x2000 ? addr % 0x2000 : addr;
    if (in_range(addr, 0x3f20, 0x3fff))
        return addr & 0x3f1f;
    return addr % rom.chr_size;
}


uint8_t Bus::ram_read_byte(uint16_t addr) {
    if (!map_memory_nes)
        return ram[addr];

    switch (ram_mirror(addr)) {
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

    /* TODO: properly handle mapper mirroring / bank switching. 
       TODO: for now, find solution that works for NROM-128 
    */
    if(in_range(addr, 0x8000, 0xbfff)) {
        return rom.read_byte_prg(addr - 0x8000);
    }
    if (in_range(addr, 0xc000, 0xffff))
        return rom.read_byte_prg(addr - 0xc000);

    return ram[ram_mirror(addr)];
};

uint16_t Bus::ram_read_two_bytes(uint16_t addr) { return (((uint16_t) ram_read_byte(addr + 1)) << 8) | ram_read_byte(addr); };

void Bus::ram_write_byte(uint16_t addr, uint8_t val) {
    if (!map_memory_nes) {
        ram[addr] = val;
        return;
    }

    switch (ram_mirror(addr)) {
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

void Bus::ram_write_two_bytes(uint16_t addr, uint16_t val) { ram_write_byte(ram_mirror(addr), (uint8_t) (val & 0xFF)); ram_write_byte(ram_mirror(addr) + 1, (uint8_t) (val >> 8)); };

uint8_t Bus::vram_read_byte(uint16_t addr) const {
    if (in_range(addr, 0x0, 0x3eff))
        return rom.read_byte_chr(vram_mirror(addr));
    return vram[vram_mirror(addr)];
};


uint16_t Bus::vram_read_two_bytes(uint16_t addr) const { return ((uint16_t) vram[vram_mirror(addr + 1)]) << 8 | vram[vram_mirror(addr)]; };


void Bus::vram_write_byte(uint16_t addr, uint8_t val) { 
    addr = vram_mirror(addr);
    if (addr > vram.size() || addr < 0)
        printf("Out of bounds vram %p\n", addr);
    vram[vram_mirror(addr)] = val;
};


void Bus::vram_write_two_bytes(uint16_t addr, uint16_t val) { vram_write_byte(vram_mirror(addr), (uint8_t) (val & 0xFF)); vram_write_byte(vram_mirror(addr) + 1, (uint8_t) (val >> 8)); };


void Bus::set_mapping(bool map_memory_nes) {this->map_memory_nes = map_memory_nes;}