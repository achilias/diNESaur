#include "bus.h"
#include "nes.h"

#include <cassert>


void bus_init(Bus *bus, ROM const *rom, Controller *controller)
{
    bus->controller = controller;
    bus->rom = rom;

    std::fill(bus->ram.begin(), bus->ram.end(), 0);
    bus->catchup_cycles = 0;
};

bool in_range(uint16_t addr, uint16_t start, uint16_t end)
{
    return addr >= start && addr <= end;
};

uint16_t ram_mirror(uint16_t addr) {
    if (in_range(addr, 0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(addr, 0x2000, 0x3fff))
        return addr & 0x2007;

    return addr;
}

uint8_t ram_read_byte(Bus *bus, uint16_t addr) {
    if (!bus->map_memory_nes)
        return bus->ram[addr];

    // address belongs to cartridge-mapped address space
    if (in_range(addr, 0x4020, 0xffff))
        return bus->rom->read_byte_prg(addr);

    switch (ram_mirror(addr)) {
        case 0x2002:
        case 0x2004:
        case 0x2007:
            return ppu_read_register(bus->nes->ppu, ram_mirror(addr));
        case 0x4016:
            return controller_read_serial_bit(bus->controller) ? 1 : 0;
    }

    return bus->ram[ram_mirror(addr)];
};

uint16_t ram_read_two_bytes(Bus *bus, uint16_t addr) {
    return (((uint16_t) ram_read_byte(bus, addr + 1)) << 8) | ram_read_byte(bus, addr);
};

void ram_write_byte(Bus *bus, uint16_t addr, uint8_t val) {
    if (!bus->map_memory_nes) {
        bus->ram[addr] = val;
        return;
    }

    if (in_range(addr, 0x8000, 0xffff)) {
        printf("Error! Attempt to write %p to rom at %p\n",val, addr);
        exit(0);
    }

    switch (ram_mirror(addr)) {
        case 0x2000:
        case 0x2001:
        case 0x2003:
        case 0x2004:
        case 0x2005:
        case 0x2006:
        case 0x2007:
            ppu_write_register(bus->nes->ppu, ram_mirror(addr), val);
            return;
        case 0x4014:
            // TODO: cycle penalty
            {
                uint16_t page_start = ((uint16_t) val) << 8;
                for (int i = 0; i < 256; i++)
                    bus->nes->ppu->oam[bus->nes->ppu->oam_addr++] = ram_read_byte(bus, page_start + i);
                return;
            }
        case 0x4016:
            if (val & 0x80)
                controller_set_strobe(bus->controller);
            else
                controller_clear_strobe(bus->controller);
            return;
    }
    bus->ram[ram_mirror(addr)] = val;
};

void ram_write_two_bytes(Bus *bus, uint16_t addr, uint16_t val) { ram_write_byte(bus, ram_mirror(addr), (uint8_t) (val & 0xFF)); ram_write_byte(bus, ram_mirror(addr) + 1, (uint8_t) (val >> 8)); };