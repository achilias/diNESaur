#include "graphics.h"

using namespace GraphicsContext;

int *GraphicsContext::framebuffer;
bool GraphicsContext::done;
SDL_Window* GraphicsContext::window;
SDL_Renderer* GraphicsContext::renderer;
SDL_Texture* GraphicsContext::texture;

void GraphicsContext::display_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    framebuffer = new int[width * height];
    window = SDL_CreateWindow("", width, height, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, width, height);
}

void GraphicsContext::finish() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool GraphicsContext::update() {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            return false;
        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
            return false;
    }

    char *pixels;
    int pitch;
    SDL_LockTexture(texture, nullptr, (void**) &pixels, &pitch);

    for (int i = 0, sp = 0, dp = 0; i < height; i++, dp += width, sp += pitch)
        // TODO: will probably need change when implementing rendering
        memcpy(pixels + sp, framebuffer + dp, width * 4);
    
    SDL_UnlockTexture(texture);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_Delay(1);
    return true;

}
void GraphicsContext::render(uint64_t ticks) {
    // TODO: write ppu framebuffer to sdl framebuffer
}