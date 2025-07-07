#include "nes.h"

#include "graphics.h"
#include <iostream>

void NES::run() {
    using namespace GraphicsContext;
    display_init();
    cpu.reset();

    bool nmi = false;
    while (update(ctrl)) {
        size_t cycles = 0;
        if (nmi) {
            cpu.handle_nmi();
            cycles = 2;
        }
        cycles += cpu.execute_instr();
        bool before = bus.nmi;
        if (ppu.run(3 * cycles)) {
            draw();
            render(SDL_GetTicks(), ppu.framebuffer);
        }
        bool after = bus.nmi;
        nmi = bus.ppu_status.vblank && !before && after;
    }

    finish();
}