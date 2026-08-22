#define OPCODE_TEST(func, op)             \
void func##_opcode_##op(void) {       \
TestCPU cpu;                 \
cpu.test_opcode(#op);       \
}

#define OPCODE_TEST_ENTRY(func, op) { #func "_opcode_" #op, func##_opcode_##op },

#define EXPAND(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) __VA_ARGS__

#define PARENS ()

#define FOR_EACH(macro, a0, ...) \
__VA_OPT__(EXPAND(FOR_EACH_HELPER(macro, a0, __VA_ARGS__)))
#define FOR_EACH_HELPER(macro, a0, a1, ...) \
macro(a0, a1)                           \
__VA_OPT__(FOR_EACH_AGAIN PARENS (macro, a0, __VA_ARGS__))
#define FOR_EACH_AGAIN() FOR_EACH_HELPER

#define MAKE_OPCODE_TESTS(func, ...) FOR_EACH(OPCODE_TEST, func, __VA_ARGS__)
#define MAKE_OPCODE_TEST_ENTRIES(func, ...) FOR_EACH(OPCODE_TEST_ENTRY, func, __VA_ARGS__)