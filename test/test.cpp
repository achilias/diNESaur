#include <gtest/gtest.h>

#include <fstream>
#include <nlohmann/json.hpp>
#include "test_cpu.h"
#include "macros.h"

using json = nlohmann::json;

void TestCPU::mem_locs_set(std::vector<std::tuple<uint16_t, uint8_t>> list) {
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        bus.ram_write_byte(std::get<0>(*iter), std::get<1>(*iter));
    }
}

void TestCPU::mem_locs_test(std::vector<std::tuple<uint16_t, uint8_t>> list) {
    for (auto iter = list.begin(); iter != list.end(); iter++) {
        uint8_t byte = bus.ram_read_byte(std::get<0>(*iter));
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
    std::ifstream f("../test/65x02/nes6502/v1/" + opcode + ".json");
    json data = json::parse(f);
    for(auto test_case = data.begin(); test_case != data.end(); test_case++) {
        std::cout << (*test_case)["name"] << std::endl;

        auto init = (*test_case)["initial"];

        set_state(init["pc"], init["s"], init["a"], init["x"], init["y"], init["p"]);

        mem_locs_set(init["ram"]);

        execute_instr();

        auto final = (*test_case)["final"];

        test_state(final["pc"], final["s"], final["a"], final["x"], final["y"], final["p"]);
        mem_locs_test(final["ram"]);

    }
}

MAKE_OPCODE_TESTS(ADC, 69, 65, 75, 6d, 7d, 79, 61, 71);

MAKE_OPCODE_TESTS(AND, 29, 25, 35, 2d, 3d, 39, 21, 31);

MAKE_OPCODE_TESTS(ASL, 0a, 06, 16, 0e, 1e);

MAKE_OPCODE_TESTS(BCC, 90);

MAKE_OPCODE_TESTS(BCS, b0);

MAKE_OPCODE_TESTS(BEQ, f0);

MAKE_OPCODE_TESTS(BIT, 24, 2c);

MAKE_OPCODE_TESTS(BMI, 30);

MAKE_OPCODE_TESTS(BNE, d0);

MAKE_OPCODE_TESTS(BPL, 10);

MAKE_OPCODE_TESTS(BRK, 00);

MAKE_OPCODE_TESTS(BVC, 50);

MAKE_OPCODE_TESTS(BVS, 70);

MAKE_OPCODE_TESTS(CLC, 18);

MAKE_OPCODE_TESTS(CLI, 58);

MAKE_OPCODE_TESTS(CLV, b8);

MAKE_OPCODE_TESTS(CMP, c9, c5, d5, cd, dd, d9, c1, d1);

MAKE_OPCODE_TESTS(CPX, e0, e4, ec);

MAKE_OPCODE_TESTS(CPY, c0, c4, cc);

MAKE_OPCODE_TESTS(DEC, c6, d6, ce, de);

MAKE_OPCODE_TESTS(DEX, ca);

MAKE_OPCODE_TESTS(DEY, 88);

MAKE_OPCODE_TESTS(EOR, 49, 45, 55, 4d, 5d, 59, 41, 51);

MAKE_OPCODE_TESTS(INC, e6, f6, ee, fe);

MAKE_OPCODE_TESTS(INX, e8);

MAKE_OPCODE_TESTS(INY, c8);

MAKE_OPCODE_TESTS(JMP, 4c, 6c);

MAKE_OPCODE_TESTS(JSR, 20);

MAKE_OPCODE_TESTS(LDA, a9, a5, b5, ad, bd, b9, a1, b1);

MAKE_OPCODE_TESTS(LDX, a2, a6, b6, ae, be);

MAKE_OPCODE_TESTS(LDY, a0, a4, b4, ac, bc);

MAKE_OPCODE_TESTS(LSR, 4a, 46, 56, 4e, 5e);

MAKE_OPCODE_TESTS(NOP, ea);

MAKE_OPCODE_TESTS(ORA, 09, 05, 15, 0d, 1d, 19, 01, 11);

MAKE_OPCODE_TESTS(PHA, 48);

MAKE_OPCODE_TESTS(PHP, 08);

MAKE_OPCODE_TESTS(PLA, 68);

MAKE_OPCODE_TESTS(PLP, 28);

MAKE_OPCODE_TESTS(ROL, 2a, 26, 36, 2e, 3e);

MAKE_OPCODE_TESTS(ROR, 6a, 66, 76, 6e, 7e);

MAKE_OPCODE_TESTS(RTI, 40);

MAKE_OPCODE_TESTS(RTS, 60);

MAKE_OPCODE_TESTS(SBC, e9, e5, f5, ed, fd, f9, e1, f1);

MAKE_OPCODE_TESTS(SEC, 38);

MAKE_OPCODE_TESTS(SED, f8);

MAKE_OPCODE_TESTS(SEI, 78);

MAKE_OPCODE_TESTS(STA, 85, 95, 8d, 9d, 99, 81, 91);

MAKE_OPCODE_TESTS(STX, 86, 96, 8e);

MAKE_OPCODE_TESTS(STY, 84, 94, 8c);

MAKE_OPCODE_TESTS(TAX, aa);

MAKE_OPCODE_TESTS(TAY, a8);

MAKE_OPCODE_TESTS(TSX, ba);

MAKE_OPCODE_TESTS(TXA, 8a);

MAKE_OPCODE_TESTS(TXS, 9a);

MAKE_OPCODE_TESTS(TYA, 98);