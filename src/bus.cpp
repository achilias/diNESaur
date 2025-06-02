#include "bus.h"

static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {return addr >= start && addr <= end;};

uint16_t Bus::ram_mirror(uint16_t addr) const {
    if (!mirror)
        return addr;

    if (in_range(addr, 0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(addr, 0x2000, 0x3fff))
        return addr & 0x2007;
    return addr;
}

uint16_t Bus::vram_mirror(uint16_t addr) const {
    // TODO
    return 0x0;
}