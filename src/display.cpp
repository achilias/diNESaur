#include <SDL3/SDL.h>

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
uint32_t* framebuffer;
const int window_width = 1024;
const int window_height = 960;

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

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

void render_and_draw(uint32_t *buffer)
{
    render(SDL_GetTicks(), buffer);
    draw();
}
