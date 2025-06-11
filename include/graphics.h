#pragma once
#include <SDL3/SDL.h>

namespace GraphicsContext {
    const int width = 256;
    const int height = 240;
    extern int* framebuffer;
    extern bool done;
    extern SDL_Window* window;
    extern SDL_Renderer* renderer;
    extern SDL_Texture* texture;
    void display_init();
    void finish();
    bool update();
    void render(uint64_t ticks);
};