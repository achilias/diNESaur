#include <SDL3/SDL_main.h>
#include <fstream>
#include <ios>

#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include "controller.h"
#include "graphics.h"

int main(int argc, char *argv[]) {
    std::ifstream file(argv[1], std::ios::binary);

    Controller ctrl;
    ROM const *rom = new ROM(file);
    Bus bus(rom, ctrl);
    CPU cpu(bus);
    PPU ppu(bus);

    using namespace GraphicsContext;
    display_init();
    cpu.reset();

    bool nmi = false;
    rom_changed = false;
    while (update(ctrl)) {
        if (rom_changed) {
            file = std::ifstream(rom_path, std::ios::binary);
            bus.rom = new ROM(file);
            cpu.reset();
            bus.reset();
            ppu.reset();
            rom_changed = false;
        }

        size_t cycles = 0;
        if (nmi) {
            cpu.handle_nmi();
            cycles = 2;
        }
        cycles += cpu.execute_instr();
        bool before = bus.nmi;
        if (ppu.run(3 * cycles)) {
            draw();
            render(SDL_GetTicks(), ppu.framebuffer.data());
        }
        bool after = bus.nmi;
        nmi = PPUSTATUS_VBLANK(bus.ppu_status) && !before && after;
    }

    finish();

    return 0;
}