#pragma once

#include <array>

struct NES;

#define RAM_SIZE 65536

struct CPU {
    /*
     * Back pointer to the parent NES instance. Used as a bus in order to access other components.
     * CPU operations can:
     * - Read from ROM
     * - Read and modify PPU registers and memory
     * - Read and modify controller states
     */
    NES *nes;
    /* When true, memory access never touches nes at all (used to run pure
     * 6502 opcode tests against a flat, unmapped ram array).
     * TODO: this is a temporary runtime flag; move it to a compile-time
     * macro gated on the test build instead, and drop the field entirely. */
    bool standalone { false };
    uint8_t sp { 0xff };
    uint16_t pc { 0 };
    uint8_t accum { 0 };
    uint8_t reg_x { 0 };
    uint8_t reg_y { 0 };
    uint8_t flags { 0 };
    uint8_t ram[RAM_SIZE];

    void set_carry(bool cond) {flags = cond ? flags | 0x1 : flags & ~0x1;}
    bool get_carry() const {return flags & 0x1;}
    void set_zero(bool cond) {flags = cond ? flags | 0x2 : flags & ~0x2;}
    bool get_zero() const {return flags & 0x2;}
    void set_disable_interrupt(bool cond) {flags = cond ? flags | 0x4 : flags & ~0x4;}
    bool get_disable_interrupt() const {return flags & 0x4;}
    void set_overflow(bool cond) {flags = cond ? flags | 0x40 : flags & ~0x40;};
    void set_decimal(bool cond) {flags = cond ? flags | 0x8 : flags & ~0x8;}
    bool get_decimal() const {return flags & 0x8;}
    bool get_overflow() const {return flags & 0x40;}
    void set_negative(bool cond) {flags = cond ? flags | 0x80 : flags & ~0x80;};
    bool get_negative() const {return flags & 0x80;}
};

void cpu_reset(CPU *cpu);
void cpu_init(CPU *cpu, NES *nes);
size_t cpu_execute_instruction(CPU *cpu);
void cpu_handle_nmi(CPU *cpu);
