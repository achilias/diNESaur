#pragma once

#include <stdbool.h>

enum Button {
    A,
    B,
    SELECT,
    START,
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

typedef struct Controller Controller;

#ifdef __cplusplus
extern "C" {
#endif

Controller *controller_create(void);
void controller_set_strobe(Controller *controller);
void controller_clear_strobe(Controller *controller);

// TODO: write explanatory comment
bool controller_read_serial_bit(Controller *controller);

void controller_set_button(Controller *controller, enum Button button, bool pressed);

#ifdef __cplusplus
}
#endif
