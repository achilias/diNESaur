#include "nes.h"

#include "graphics.h"

void NES::run() {
    using namespace GraphicsContext;
    display_init();
    cpu.reset();
    std::ifstream file(rom_path, std::ios::binary);

    bool nmi = false;
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
}