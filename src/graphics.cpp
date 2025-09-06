#include "graphics.h"
#include "L2DFileDialog.h"
#include <stdio.h>

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

using namespace GraphicsContext;

uint32_t*     GraphicsContext::framebuffer;
SDL_Window*   GraphicsContext::window;
SDL_Renderer* GraphicsContext::renderer;
SDL_Texture*  GraphicsContext::texture;
ImVec4        GraphicsContext::clear_colour;
bool          GraphicsContext::rom_changed;
char          GraphicsContext::rom_path[1024] = "";

void GraphicsContext::display_init() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    clear_colour = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    framebuffer = new uint32_t[window_width * window_height];
    window = SDL_CreateWindow("NES emulator", window_width, window_height, 0);
    renderer = SDL_CreateRenderer(window, nullptr);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);
}

void GraphicsContext::finish() {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void GraphicsContext::draw() {
    bool flag;
    char *pixels;
    int row_sz;
    SDL_LockTexture(texture, nullptr, (void**) &pixels, &row_sz);

    for (int i = 0, sp = 0, dp = 0; i < window_height; i++, dp += window_width, sp += row_sz)
        memcpy(pixels + sp, framebuffer + dp, window_width * 4); // 4 bytes per pixel
    
    SDL_UnlockTexture(texture);
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    SDL_RenderTexture(renderer, texture, nullptr, nullptr);
    ImGui::NewFrame();

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("Emulator", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::Image((ImTextureID)texture, ImVec2(window_width, window_height));
    ImGui::End();

    if (FileDialog::file_dialog_open) {
        FileDialog::ShowFileDialog(&FileDialog::file_dialog_open, rom_path, sizeof(rom_path), FileDialog::file_dialog_open_type);
        if (strlen(rom_path))   // "Cancel" writes "" to the buffer
            rom_changed = true;
    }

    ImGui::PopStyleVar(2);

    ImGui::Render();
    SDL_SetRenderDrawColorFloat(renderer, clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

bool GraphicsContext::update(Controller& ctrl) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL3_ProcessEvent(&e);
        if (e.type == SDL_EVENT_QUIT)
            return false;
        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
            return false;
    
        if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) {
            int idx;
            switch (e.key.key) {
                case SDLK_F1:
                    FileDialog::file_dialog_open = !FileDialog::file_dialog_open;
                    return true;
                case SDLK_Z:
                    idx = static_cast<int>(Ctrl_State::SELECT);
                    break;
                case SDLK_X:
                    idx = static_cast<int>(Ctrl_State::START);
                    break;
                case SDLK_DOWN:
                    idx = static_cast<int>(Ctrl_State::DOWN);
                    break;
                case SDLK_UP:
                    idx = static_cast<int>(Ctrl_State::UP);
                    break;
                case SDLK_LEFT:
                    idx = static_cast<int>(Ctrl_State::LEFT);
                    break;
                case SDLK_RIGHT:
                    idx = static_cast<int>(Ctrl_State::RIGHT);
                    break;
                case SDLK_A:
                    idx = static_cast<int>(Ctrl_State::A);
                    break;
                case SDLK_B:
                    idx = static_cast<int>(Ctrl_State::B);
                    break;
                default:
                    idx = static_cast<int>(Ctrl_State::START);
            }
            ctrl.button_states[idx] = e.type == SDL_EVENT_KEY_DOWN;
        }
    }

    return true;
}

static void draw_rect(int x, int y, int rect_width, int rect_height, uint32_t colour, uint32_t *buf) {
    for (int x_ = x; x_ < x + rect_width; x_++)
        for (int y_ = y; y_ < y + rect_height; y_++)
            buf[y_ * window_width + x_] = colour;
}

void GraphicsContext::render(uint64_t ticks, uint32_t *buffer) {
    int rect_width = window_width / SCREEN_WIDTH;
    int rect_height = window_height / SCREEN_HEIGHT;
    for (int i = 0; i < SCREEN_WIDTH; i ++)
        for (int j = 0; j < SCREEN_HEIGHT; j++)
            draw_rect(i * rect_width, j * rect_height, rect_width, rect_height, buffer[j * SCREEN_WIDTH + i], framebuffer);
}