#pragma once
#include <SDL3/SDL.h>

namespace GraphicsContext {
    const int window_width = 1024;
    const int window_height = 960;
    extern uint32_t* framebuffer;
    extern bool done;
    extern SDL_Window* window;
    extern SDL_Renderer* renderer;
    extern SDL_Texture* texture;
    void display_init();
    void finish();
    bool update();
    void render(uint64_t ticks, uint32_t *buffer);
};