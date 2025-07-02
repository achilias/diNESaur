#include "bus.h"

uint16_t Bus::ram_mirror(uint16_t addr) const {
    if (in_range(addr, 0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(addr, 0x2000, 0x3fff))
        return addr & 0x2007;
    if (in_range(addr, 0x8000, 0xffff))
        // for 16kb (0x4000) cartridges, upper 16kb of 32kb address space must be mirrored to lower 16kb
        return  rom.prg_size == 0x4000
                ? addr % 0x4000
                : addr;
    return addr;
}

uint16_t Bus::vram_mirror(uint16_t addr) const {
    if (in_range(addr, 0x3f20, 0x3fff))
        return addr & 0x3f1f;
    return addr;
}