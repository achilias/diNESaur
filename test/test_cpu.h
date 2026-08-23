#ifndef TEST_CPU_H
#define TEST_CPU_H

#include "../src/cpu.h"
#include <vector>

class TestCPU : public CPU {
public:
    TestCPU() : CPU(*make_test_bus()) {};
    void test_opcode(std::string opcode);
    void set_state(uint16_t pc, uint8_t sp, uint8_t accum, uint8_t reg_x, uint8_t reg_y, uint8_t sr);
    void test_state(uint16_t pc, uint8_t sp, uint8_t accum, uint8_t reg_x, uint8_t reg_y, uint8_t sr);
    void mem_locs_set(std::vector<std::tuple<uint16_t, uint8_t>> list);
    void mem_locs_test(std::vector<std::tuple<uint16_t, uint8_t>> list);

private:
    // Constructs a Bus with map_memory_nes already disabled, before CPU's constructor
    // (which resets the CPU by reading through the bus) ever runs.
    static Bus *make_test_bus() {
        Bus *bus = new Bus();
        bus_init(bus, new ROM(), new Controller());
        bus->map_memory_nes = false;
        return bus;
    }
};

#endif
