#include <SDL3/SDL.h>

#include "controller.h"

void poll_for_input(Controller& controller, bool* should_exit) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
        {
            *should_exit = true;
            return;
        }

        if (e.type != SDL_EVENT_KEY_DOWN && e.type != SDL_EVENT_KEY_UP)
            continue;

        Ctrl_State button;
        switch (e.key.key) {
            case SDLK_ESCAPE:   *should_exit = true;         return;
            case SDLK_Z:        button = Ctrl_State::SELECT; break;
            case SDLK_X:        button = Ctrl_State::START;  break;
            case SDLK_DOWN:     button = Ctrl_State::DOWN;   break;
            case SDLK_UP:       button = Ctrl_State::UP;     break;
            case SDLK_LEFT:     button = Ctrl_State::LEFT;   break;
            case SDLK_RIGHT:    button = Ctrl_State::RIGHT;  break;
            case SDLK_A:        button = Ctrl_State::A;      break;
            case SDLK_B:        button = Ctrl_State::B;      break;
            default:            continue;
        }
        auto index = static_cast<int>(button);
        controller.button_states[index] = e.type == SDL_EVENT_KEY_DOWN;
    }
}
