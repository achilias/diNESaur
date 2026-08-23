#include <fstream>
#include <ios>

#include "cpu.h"
#include "ppu.h"
#include "bus.h"
#include "controller.h"

#include <SDL3/SDL.h>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
#define FILE_PATH_MAX_LENGTH 1024

const int window_width = 1024;
const int window_height = 960;
uint32_t* framebuffer;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
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

void draw() {
    char *pixels;
    int row_sz;
    SDL_LockTexture(texture, nullptr, (void**) &pixels, &row_sz);

    for (int i = 0, sp = 0, dp = 0; i < window_height; i++, dp += window_width, sp += row_sz)
        memcpy(pixels + sp, framebuffer + dp, window_width * 4); // 4 bytes per pixel

    SDL_UnlockTexture(texture);
    SDL_SetRenderDrawColorFloat(renderer, 0.45f, 0.55f, 0.60f, 1.00f);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
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
                case SDLK_F1:
                    // TODO: ROM selection UI
                    return true;
                // TODO: f2 for control config menu etc
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

void display_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    framebuffer = new uint32_t[window_width * window_height];
    window = SDL_CreateWindow("DiNESaur", window_width, window_height, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
}

void display_finish() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

static void draw_rect(int x, int y, int rect_width, int rect_height, uint32_t colour, uint32_t *buf) {
    for (int x_ = x; x_ < x + rect_width; x_++)
        for (int y_ = y; y_ < y + rect_height; y_++)
            buf[y_ * window_width + x_] = colour;
}

void render(uint64_t ticks, uint32_t *buffer) {
    int rect_width = window_width / SCREEN_WIDTH;
    int rect_height = window_height / SCREEN_HEIGHT;
    for (int i = 0; i < SCREEN_WIDTH; i ++)
        for (int j = 0; j < SCREEN_HEIGHT; j++)
            draw_rect(i * rect_width, j * rect_height, rect_width, rect_height, buffer[j * SCREEN_WIDTH + i], framebuffer);
}