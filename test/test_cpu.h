#ifndef TEST_CPU_H
#define TEST_CPU_H

#include "../src/cpu.h"
#include <vector>

class TestCPU : public CPU {
public:
    TestCPU() : CPU(*new Bus(new ROM(), *new Controller())) { bus.map_memory_nes = false; };
    void test_opcode(std::string opcode);
    void set_state(uint16_t pc, uint8_t sp, uint8_t accum, uint8_t reg_x, uint8_t reg_y, uint8_t sr);
    void test_state(uint16_t pc, uint8_t sp, uint8_t accum, uint8_t reg_x, uint8_t reg_y, uint8_t sr);
    void mem_locs_set(std::vector<std::tuple<uint16_t, uint8_t>> list);
    void mem_locs_test(std::vector<std::tuple<uint16_t, uint8_t>> list);
};

#endif
