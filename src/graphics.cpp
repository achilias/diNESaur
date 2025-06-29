#include "graphics.h"
#include "ppu.h"

using namespace GraphicsContext;

uint32_t *GraphicsContext::framebuffer;
bool GraphicsContext::done;
SDL_Window* GraphicsContext::window;
SDL_Renderer* GraphicsContext::renderer;
SDL_Texture* GraphicsContext::texture;

void GraphicsContext::display_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    framebuffer = new uint32_t[window_width * window_height];
    window = SDL_CreateWindow("NES emulator", window_width, window_height, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
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
    int row_sz;
    SDL_LockTexture(texture, nullptr, (void**) &pixels, &row_sz);

    for (int i = 0, sp = 0, dp = 0; i < window_height; i++, dp += window_width, sp += row_sz)
        memcpy(pixels + sp, framebuffer + dp, window_width * 4); // 4 bytes per pixel
    
    SDL_UnlockTexture(texture);
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
    SDL_Delay(1);
    return true;
}

static void draw_rect(int x, int y, int rect_width, int rect_height, uint32_t colour, uint32_t *buf) {
    for (int y_ = y; y_ < y + rect_height; y_++)
        memset(&buf[y_ * window_width + x], colour, rect_width * 4);
}

void GraphicsContext::render(uint64_t ticks, uint32_t *buffer) {
    int rect_width = window_width / SCREEN_WIDTH;
    int rect_height = window_height / SCREEN_HEIGHT;
    for (int i = 0; i < SCREEN_WIDTH; i ++)
        for (int j = 0; j < SCREEN_HEIGHT; j++)
            draw_rect(i * rect_width, j * rect_height, rect_width, rect_height, buffer[j * SCREEN_WIDTH + i], framebuffer);
}