#include "controller.h"
#include <string.h>
#include <stdlib.h>

enum ControllerState {
    READ_A,
    READ_B,
    READ_SELECT,
    READ_START,
    READ_UP,
    READ_DOWN,
    READ_LEFT,
    READ_RIGHT,
    END,
    STROBE
};

struct Controller {
    enum ControllerState state;
    bool button_states[8];
};

Controller *controller_create(void)
{
   Controller *controller = malloc(sizeof(*controller));
   controller->state = READ_A;
   memset(controller->button_states, 0, sizeof(controller->button_states));
   return controller;
}

void controller_set_strobe(Controller *controller)
{
    controller->state = STROBE;
}

void controller_clear_strobe(Controller *controller)
{
    controller->state = READ_A;
}

bool controller_read_serial_bit(Controller *controller) {
    if (controller->state == STROBE)
        return controller->button_states[A];
    if (controller->state == END)
        return true;

    return controller->button_states[controller->state++];
}

void controller_set_button(Controller *controller, enum Button button, bool pressed)
{
    controller->button_states[button] = pressed;
}
