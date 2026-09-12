#pragma once

#include <cstddef>
#include <cstdint>
#include <array>

#define RAM_SIZE 65536

struct NES;

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
    NES *nes;
    /* When true, memory access never touches nes at all (used to run pure
     * 6502 opcode tests against a flat, unmapped ram array).
     * TODO: this is a temporary runtime flag; move it to a compile-time
     * macro gated on the test build instead, and drop the field entirely. */
    bool standalone { false };
    static const uint16_t stack_base = 0x100;
    uint8_t sp { 0xff };
    uint16_t pc { 0 };
    uint8_t accum { 0 };
    uint8_t reg_x { 0 };
    uint8_t reg_y { 0 };
    uint8_t flags { 0 };
    std::array<uint8_t, RAM_SIZE> ram {};

    inline void set_carry(bool cond) {flags = cond ? flags | 0x1 : flags & ~0x1;}
    inline bool get_carry() const {return flags & 0x1;}
    inline void set_zero(bool cond) {flags = cond ? flags | 0x2 : flags & ~0x2;}
    inline bool get_zero() const {return flags & 0x2;}
    inline void set_disable_interrupt(bool cond) {flags = cond ? flags | 0x4 : flags & ~0x4;}
    inline bool get_disable_interrupt() const {return flags & 0x4;}
    inline void set_overflow(bool cond) {flags = cond ? flags | 0x40 : flags & ~0x40;};
    inline void set_decimal(bool cond) {flags = cond ? flags | 0x8 : flags & ~0x8;}
    inline bool get_decimal() const {return flags & 0x8;}
    inline bool get_overflow() const {return flags & 0x40;}
    inline void set_negative(bool cond) {flags = cond ? flags | 0x80 : flags & ~0x80;};
    inline bool get_negative() const {return flags & 0x80;}
};

void cpu_reset(CPU *cpu);
void cpu_init(CPU *cpu, NES *nes);
uint8_t cpu_read_byte(CPU *cpu, uint16_t addr);
uint16_t cpu_read_two_bytes(CPU *cpu, uint16_t addr);
void cpu_write_byte(CPU *cpu, uint16_t addr, uint8_t val);
void cpu_write_two_bytes(CPU *cpu, uint16_t addr, uint16_t val);
