#pragma once

#include <array>

enum class Ctrl_State : int {
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

// Ctrl_State& operator++(Ctrl_State& st) {
//     int next = static_cast<int>(st) + 1;
//     Ctrl_State ret = static_cast<Ctrl_State>(next);
//     return ret;
// }
#include <stdio.h>

struct Controller {
    Ctrl_State state;
    std::array<bool, 8> button_states;
    Controller() : state(Ctrl_State::A) {
        button_states.fill(false);
    }
    inline void strobe_set() {state = Ctrl_State::STROBE;}
    inline void strobe_clear() {state = Ctrl_State::A;}
    inline bool poll() {
        if (state == Ctrl_State::STROBE)
            return button_states[static_cast<int>(Ctrl_State::A)];
        if (state == Ctrl_State::END)
            return true;

        int old = static_cast<int>(state);
        state = static_cast<Ctrl_State>(old + 1);
        // printf("Button state : %d\n", static_cast<int>(state));
        return button_states[old];
    }

};