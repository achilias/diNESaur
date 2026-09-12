#pragma once

#include <stdint.h>
#include <stddef.h>

struct NES;

#define RAM_SIZE 65536

typedef struct CPU {
    /*
     * Back pointer to the parent NES instance. Used as a bus in order to access other components.
     * CPU operations can:
     * - Read from ROM
     * - Read and modify PPU registers and memory
     * - Read and modify controller states
     */
    struct NES *nes;

    uint8_t sp;
    uint16_t pc;
    uint8_t accum;
    uint8_t reg_x;
    uint8_t reg_y;
    uint8_t flags;
    uint8_t ram[RAM_SIZE];

} CPU;

#ifdef __cplusplus
extern "C" {
#endif

CPU *cpu_create(struct NES *nes);
size_t cpu_execute_instruction(CPU *cpu);
void cpu_handle_nmi(CPU *cpu);

#ifdef __cplusplus
}
#endif
