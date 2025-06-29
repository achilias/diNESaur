#include "nes.h"

#include "graphics.h"
#include <iostream>

void NES::run() {
    using namespace GraphicsContext;
    display_init();

	for (int i = 0; i < 200; i++)
		ppu.draw_tile(i, i * 8, (i / 30) * 8);

    while (update())
        render(SDL_GetTicks(), ppu.framebuffer);

    finish();
}