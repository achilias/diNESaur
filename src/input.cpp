#include <SDL3/SDL.h>

#include "controller.h"

void poll_for_input(Controller *controller, bool* should_exit) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_EVENT_QUIT)
        {
            *should_exit = true;
            return;
        }

        if (e.type != SDL_EVENT_KEY_DOWN && e.type != SDL_EVENT_KEY_UP)
            continue;

        ControllerState button;
        switch (e.key.key) {
            case SDLK_ESCAPE:   *should_exit = true;         return;
            case SDLK_Z:        button = ControllerState::SELECT; break;
            case SDLK_X:        button = ControllerState::START;  break;
            case SDLK_DOWN:     button = ControllerState::DOWN;   break;
            case SDLK_UP:       button = ControllerState::UP;     break;
            case SDLK_LEFT:     button = ControllerState::LEFT;   break;
            case SDLK_RIGHT:    button = ControllerState::RIGHT;  break;
            case SDLK_A:        button = ControllerState::A;      break;
            case SDLK_B:        button = ControllerState::B;      break;
            default:            continue;
        }
        auto index = static_cast<int>(button);
        controller->button_states[index] = e.type == SDL_EVENT_KEY_DOWN;
    }
}
