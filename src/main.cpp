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
bool update(Controller& ctrl);

int main(int argc, char *argv[]) {
    Controller ctrl;
    display_init();
    
    if (argc < 2) {
        printf("Usage: %s rom_path\n", argv[0]);
        return 1;
    }

    Bus bus(nullptr, ctrl);
    std::ifstream file = std::ifstream(argv[1], std::ios::binary);
    bus.rom = new ROM(file);
    CPU cpu(bus);
    PPU ppu(bus);
    cpu.reset();
    bus.reset();
    ppu.reset();

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
            render(SDL_GetTicks(), ppu.framebuffer.data());
        }
        bool after = bus.nmi;
        nmi = PPUSTATUS_VBLANK(bus.ppu_status) && !before && after;
    }

    display_finish();

    return 0;
}
