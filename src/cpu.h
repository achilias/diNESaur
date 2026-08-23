#pragma once

#include <cstddef>
#include "bus.h"

enum class AddressingMode {
    accumulator,
    immediate,
    zero_page,
    zero_page_idx_x,
    zero_page_idx_y,
    absolute,
    absolute_idx_x,
    absolute_idx_y,
    indirect,
    indirect_idx_x,
    indirect_idx_y,
};

class CPU {
public:
    size_t execute_instr();
    void handle_nmi();
    Bus *bus;
    static const uint16_t stack_base = 0x100;
    uint8_t sp { 0xff };
    uint16_t pc { 0 };
    uint8_t accum { 0 };
    uint8_t reg_x { 0 };
    uint8_t reg_y { 0 };
    uint8_t sr { 0 };

    inline void set_carry(bool cond) {sr = cond ? sr | 0x1 : sr & ~0x1;}
    inline bool get_carry() const {return sr & 0x1;}
    inline void set_zero(bool cond) {sr = cond ? sr | 0x2 : sr & ~0x2;}
    inline bool get_zero() const {return sr & 0x2;}
    inline void set_disable_interrupt(bool cond) {sr = cond ? sr | 0x4 : sr & ~0x4;}
    inline bool get_disable_interrupt() const {return sr & 0x4;}
    inline void set_overflow(bool cond) {sr = cond ? sr | 0x40 : sr & ~0x40;};
    inline void set_decimal(bool cond) {sr = cond ? sr | 0x8 : sr & ~0x8;}
    inline bool get_decimal() const {return sr & 0x8;}
    inline bool get_overflow() const {return sr & 0x40;}
    inline void set_negative(bool cond) {sr = cond ? sr | 0x80 : sr & ~0x80;};
    inline bool get_negative() const {return sr & 0x80;}
};

uint16_t get_addr(CPU *cpu, AddressingMode mode);
void cpu_reset(CPU *cpu);
void cpu_init(CPU *cpu, Bus *bus);
