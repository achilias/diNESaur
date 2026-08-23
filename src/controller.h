#pragma once

#include <array>

enum class ControllerState : int {
    A,
    B,
    SELECT,
    START,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    END,
    STROBE
};

struct Controller {
    ControllerState state;
    std::array<bool, 8> button_states {};
};

void controller_init(Controller *controller);
void controller_set_strobe(Controller *controller);
void controller_clear_strobe(Controller *controller);
bool controller_(Controller *controller);

// TODO: write explanatory comment
bool controller_read_serial_bit(Controller *controller);