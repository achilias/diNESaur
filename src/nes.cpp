#include "nes.h"

void nes_init(NES *nes)
{
    nes->bus = new Bus();
    nes->controller = new Controller();
    bus_init(nes->bus, nes->rom, nes->controller);

    nes->cpu = new CPU();
    cpu_init(nes->cpu, nes->bus);

    nes->ppu = new PPU();
    ppu_init(nes->ppu, nes->bus);
}

void nes_run(void (*drawing_callback)(uint32_t*))
{
    // TODO: move main loop here and take screen update function as callback
    // this is done in order to decouple the game loop and display / input code from the internal NES representation and logic
}
