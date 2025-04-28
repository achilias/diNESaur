#define OPCODE_TEST(func, op)             \
TEST(CPU_Tests, func##_opcode_##op) {  \
TestCPU cpu;                 \
cpu.test_opcode(#op);       \
}

#define EXPAND(arg) EXPAND1(EXPAND1(EXPAND1(EXPAND1(arg))))
#define EXPAND1(arg) EXPAND2(EXPAND2(EXPAND2(EXPAND2(arg))))
#define EXPAND2(arg) arg

#define PARENS ()

#define FOR_EACH(macro, a0, ...) \
__VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, a0, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a0, a1, ...) \
macro(a0, a1)                           \
__VA_OPT__(FOR_EACH_AGAIN PARENS (macro, a0, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define MAKE_OPCODE_TESTS(func, ...) \
    FOR_EACH(OPCODE_TEST, func, __VA_ARGS__)