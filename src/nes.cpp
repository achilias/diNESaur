#include "nes.h"

#include "graphics.h"
#include <iostream>

void NES::run() {
    using namespace GraphicsContext;
    display_init();
    cpu.reset();

    while (update(false)) {
        size_t cycles = 0;
        if (bus.nmi) {
            cpu.handle_nmi();
            cycles = 2;
        }
        cycles += cpu.execute_instr();
        if (ppu.run(3 * cycles)) {
            update(true);
            render(SDL_GetTicks(), ppu.framebuffer);
        }
    }

    finish();
}