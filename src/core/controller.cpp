#include "controller.h"

void controller_init(Controller *controller) {
    controller->state = ControllerState::A;
    controller->button_states.fill(false);
}

void controller_set_strobe(Controller *controller)
{
    controller->state = ControllerState::STROBE;
}

void controller_clear_strobe(Controller *controller)
{
    controller->state = ControllerState::A;
}

bool controller_read_serial_bit(Controller *controller) {
    if (controller->state == ControllerState::STROBE)
        return controller->button_states[static_cast<int>(ControllerState::A)];
    if (controller->state == ControllerState::END)
        return true;

    int old = static_cast<int>(controller->state);
    controller->state = static_cast<ControllerState>(old + 1);
    return controller->button_states[old];
}

