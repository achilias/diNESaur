#include <fstream>
#include <ios>

#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include "controller.h"
#include "input.h"

#include <SDL3/SDL.h>

void display_init();
void display_finish();
void draw();
void render(uint64_t ticks, uint32_t *buffer);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s rom_path\n", argv[0]);
        return 1;
    }

    Controller controller;
    controller_init(&controller);

    std::ifstream file = std::ifstream(argv[1], std::ios::binary);
    ROM *rom = new ROM(file);

    Bus bus;
    bus_init(&bus, rom, &controller);
    CPU cpu{bus};
    cpu_reset(&cpu);
    PPU ppu{bus};
    ppu_reset(&ppu);

    display_init();

    bool nmi = false;
    bool should_exit = false;
    while (!should_exit) {
        poll_for_input(&controller, &should_exit);

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

    display_finish();

    return 0;
}
