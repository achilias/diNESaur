#include "graphics.h"

void GraphicsContext::display_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
}