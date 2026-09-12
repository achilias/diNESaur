#include "nes.h"
#include "cpu.h"
#include "../input.h"

void nes_init(NES *nes, FILE *file)
{
    nes->nmi = false;
    nes->rom = rom_create(file);
    nes->controller = controller_create();

    nes->cpu = new CPU();
    cpu_init(nes->cpu, nes);

    nes->ppu = ppu_create(nes);
}

void nes_run(NES *nes, DrawingCallback draw, InputPollingCallback poll_for_input)
{
    // this is done in order to decouple the game loop and display / input code from the internal NES representation and logic
    // TODO: better comment

    bool nmi = false;
    bool should_exit = false;
    while (!should_exit) {
        poll_for_input(nes->controller, &should_exit);

        size_t cycles = 0;
        if (nmi) {
            cpu_handle_nmi(nes->cpu);
            cycles = 2;
        }
        cycles += cpu_execute_instruction(nes->cpu);
        bool before = nes->nmi;
        if (ppu_run(nes->ppu, 3 * cycles)) {
            draw(nes->ppu->framebuffer);
        }
        bool after = nes->nmi;
        nmi = PPUSTATUS_VBLANK(nes->ppu->ppu_status) && !before && after;
    }
}
