#include "nes.h"
#include "input.h"

void nes_init(NES *nes, std::ifstream& file)
{
    nes->rom = new ROM(file);

    nes->bus = new Bus();
    nes->controller = new Controller();
    bus_init(nes->bus, nes->rom, nes->controller);
    nes->bus->nes = nes;

    nes->cpu = new CPU();
    cpu_init(nes->cpu, nes->bus, nes);

    nes->ppu = new PPU();
    ppu_init(nes->ppu, nes->bus, nes);
}

void nes_run(NES *nes, void (*drawing_callback)(uint32_t*))
{
    // this is done in order to decouple the game loop and display / input code from the internal NES representation and logic
    // TODO: better comment

    bool nmi = false;
    bool should_exit = false;
    while (!should_exit) {
        poll_for_input(nes->controller, &should_exit);

        size_t cycles = 0;
        if (nmi) {
            nes->cpu->handle_nmi();
            cycles = 2;
        }
        cycles += nes->cpu->execute_instr();
        bool before = nes->nmi;
        if (nes->ppu->run(3 * cycles)) {
            drawing_callback(nes->ppu->framebuffer.data());
        }
        bool after = nes->nmi;
        nmi = PPUSTATUS_VBLANK(nes->ppu->ppu_status) && !before && after;
    }
}
