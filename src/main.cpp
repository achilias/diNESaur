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
bool update(Controller& ctrl);
void render(uint64_t ticks, uint32_t *buffer);

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


bool update(Controller& ctrl) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            return false;

        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
            return false;
        if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) {
            int idx;
            switch (e.key.key) {
                case SDLK_Z:
                    idx = static_cast<int>(Ctrl_State::SELECT);
                    break;
                case SDLK_X:
                    idx = static_cast<int>(Ctrl_State::START);
                    break;
                case SDLK_DOWN:
                    idx = static_cast<int>(Ctrl_State::DOWN);
                    break;
                case SDLK_UP:
                    idx = static_cast<int>(Ctrl_State::UP);
                    break;
                case SDLK_LEFT:
                    idx = static_cast<int>(Ctrl_State::LEFT);
                    break;
                case SDLK_RIGHT:
                    idx = static_cast<int>(Ctrl_State::RIGHT);
                    break;
                case SDLK_A:
                    idx = static_cast<int>(Ctrl_State::A);
                    break;
                case SDLK_B:
                    idx = static_cast<int>(Ctrl_State::B);
                    break;
                default:
                    idx = static_cast<int>(Ctrl_State::START);
            }
            ctrl.button_states[idx] = e.type == SDL_EVENT_KEY_DOWN;
        }
    }

    return true;
}
