#include <fstream>
#include <ios>

#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include "controller.h"

#include <SDL3/SDL.h>

void display_init();
void display_finish();
void draw();
void render(uint64_t ticks, uint32_t *buffer);
bool poll_for_input(Controller& controller, bool* should_quit);

int main(int argc, char *argv[]) {
    Controller controller;
    display_init();
    
    if (argc < 2) {
        printf("Usage: %s rom_path\n", argv[0]);
        return 1;
    }

    Bus bus(nullptr, controller);
    std::ifstream file = std::ifstream(argv[1], std::ios::binary);
    bus.rom = new ROM(file);
    CPU cpu(bus);
    PPU ppu(bus);

    bool nmi = false;
    bool should_exit = false;
    while (!should_exit) {
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

        poll_for_input(controller, &should_exit);
    }

    display_finish();

    return 0;
}
