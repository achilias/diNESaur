#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include "test_cpu.h"
#include "macros.h"

using json = nlohmann::json;

void TestCPU::mem_locs_set(std::vector<std::tuple<uint16_t, uint8_t>> list) {
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        mem.write_byte(std::get<0>(*iter), std::get<1>(*iter));
    }
}

void TestCPU::mem_locs_test(std::vector<std::tuple<uint16_t, uint8_t>> list) {
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        uint8_t byte = mem.read_byte(std::get<0>(*iter));
        EXPECT_EQ(byte, std::get<1>(*iter));
    }
}

void TestCPU::set_state(uint16_t pc, uint8_t sp, uint8_t accum, uint8_t reg_x, uint8_t reg_y, uint8_t sr) {
    this->pc = pc; this->sp = sp; this->accum = accum; this->reg_x = reg_x; this->reg_y = reg_y; this->sr = sr;
};

void TestCPU::test_state(uint16_t pc, uint8_t sp, uint8_t accum, uint8_t reg_x, uint8_t reg_y, uint8_t sr) {
    EXPECT_EQ(this->pc, pc);
    EXPECT_EQ(this->sp, sp);
    EXPECT_EQ(this->accum, accum);
    EXPECT_EQ(this->reg_x, reg_x);
    EXPECT_EQ(this->reg_y, reg_y);
    EXPECT_EQ(this->sr, sr);
};

void TestCPU::test_opcode(std::string opcode) {
    std::ifstream f("../test/json_tests/" + opcode + "_test.json");
    json data = json::parse(f);
    for(auto test_case = data.begin(); test_case != data.end(); test_case++) {
        auto init = (*test_case)["initial"];
        set_state(init["pc"], init["s"], init["a"], init["x"], init["y"], init["p"]);
        mem_locs_set(init["ram"]);

        execute_instr();

        auto final = (*test_case)["final"];
        test_state(final["pc"], final["s"], final["a"], final["x"], final["y"], final["p"]);
        mem_locs_test(final["ram"]);

        std::cout << (*test_case)["name"] << std::endl;
    }
}