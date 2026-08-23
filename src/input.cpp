#include <SDL3/SDL.h>

#include "controller.h"

bool update(Controller& ctrl) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
            return false;

        if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
            return false;
        if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) {
            int idx;
            switch (e.key.key) {
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
