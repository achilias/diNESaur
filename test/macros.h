#define OPCODE_TEST(func, op)               \
void func##_opcode_##op(void) {             \
    CPU *cpu = cpu_create(NULL);            \
    test_opcode(cpu, #op);                  \
    free(cpu);                             \
}

#define OPCODE_TEST_ENTRY(func, op) { #func "_opcode_" #op, func##_opcode_##op },

/* Each instruction has between one and eight tested opcodes. */
#define FOR_EACH_1(macro, func, a) macro(func, a)
#define FOR_EACH_2(macro, func, a, ...) macro(func, a) FOR_EACH_1(macro, func, __VA_ARGS__)
#define FOR_EACH_3(macro, func, a, ...) macro(func, a) FOR_EACH_2(macro, func, __VA_ARGS__)
#define FOR_EACH_4(macro, func, a, ...) macro(func, a) FOR_EACH_3(macro, func, __VA_ARGS__)
#define FOR_EACH_5(macro, func, a, ...) macro(func, a) FOR_EACH_4(macro, func, __VA_ARGS__)
#define FOR_EACH_6(macro, func, a, ...) macro(func, a) FOR_EACH_5(macro, func, __VA_ARGS__)
#define FOR_EACH_7(macro, func, a, ...) macro(func, a) FOR_EACH_6(macro, func, __VA_ARGS__)
#define FOR_EACH_8(macro, func, a, ...) macro(func, a) FOR_EACH_7(macro, func, __VA_ARGS__)
#define SELECT_FOR_EACH(_1, _2, _3, _4, _5, _6, _7, _8, NAME, ...) NAME
#define FOR_EACH(macro, func, ...) \
    SELECT_FOR_EACH(__VA_ARGS__, FOR_EACH_8, FOR_EACH_7, FOR_EACH_6, FOR_EACH_5, \
                    FOR_EACH_4, FOR_EACH_3, FOR_EACH_2, FOR_EACH_1, unused)(macro, func, __VA_ARGS__)

#define MAKE_OPCODE_TESTS(func, ...) FOR_EACH(OPCODE_TEST, func, __VA_ARGS__)
#define MAKE_OPCODE_TEST_ENTRIES(func, ...) FOR_EACH(OPCODE_TEST_ENTRY, func, __VA_ARGS__)
