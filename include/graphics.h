#pragma once
#include <SDL3/SDL.h>

class GraphicsContext {
public:
    int* framebuffer;
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    static void display_init();
};