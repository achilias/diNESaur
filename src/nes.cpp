#include "nes.h"

#include "graphics.h"

void NES::run() {
    using namespace GraphicsContext;
    display_init();

    while (update())
        render(SDL_GetTicks(), bus.framebuffer);

    finish();
}