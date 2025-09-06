#pragma once
#include "controller.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include <SDL3/SDL.h>

namespace GraphicsContext {
    const int window_width = 1024;
    const int window_height = 960;
    extern uint32_t* framebuffer;
    extern SDL_Window* window;
    extern ImVec4 clear_colour;
    extern SDL_Renderer* renderer;
    extern SDL_Texture* texture;
    extern bool rom_changed;
    extern char rom_path[1024];
    void display_init();
    void finish();
    void draw();
    bool update(Controller& ctrl);
    void render(uint64_t ticks, uint32_t *buffer);
};