#include <SDL3/SDL.h>

#include "core/controller.h"

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

        enum Button button;
        switch (e.key.key) {
            case SDLK_ESCAPE:   *should_exit = true;         return;
            case SDLK_Z:        button = SELECT; break;
            case SDLK_X:        button = START;  break;
            case SDLK_DOWN:     button = DOWN;   break;
            case SDLK_UP:       button = UP;     break;
            case SDLK_LEFT:     button = LEFT;   break;
            case SDLK_RIGHT:    button = RIGHT;  break;
            case SDLK_A:        button = A;      break;
            case SDLK_B:        button = B;      break;
            default:            continue;
        }
        controller_set_button(controller, button, e.type == SDL_EVENT_KEY_DOWN);
    }
}
