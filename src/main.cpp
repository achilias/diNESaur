#include <fstream>
#include <ios>

#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include "controller.h"
#include "input.h"

#include "nes.h"

#include <SDL3/SDL.h>

void display_init();
void display_finish( );
void render_and_draw(uint32_t *buffer);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s rom_path\n", argv[0]);
        return 1;
    }

    display_init();

    NES nes;
    std::ifstream file = std::ifstream(argv[1], std::ios::binary);
    nes.rom = new ROM(file);
    nes_init(&nes);

    bool nmi = false;
    bool should_exit = false;
    while (!should_exit) {
        poll_for_input(nes.controller, &should_exit);

        size_t cycles = 0;
        if (nmi) {
            nes.cpu->handle_nmi();
            cycles = 2;
        }
        cycles += nes.cpu->execute_instr();
        bool before = nes.bus->nmi;
        if (nes.ppu->run(3 * cycles)) {
            render_and_draw(nes.ppu->framebuffer.data());
        }
        bool after = nes.bus->nmi;
        nmi = PPUSTATUS_VBLANK(nes.bus->ppu_status) && !before && after;
    }

    display_finish();

    return 0;
}
