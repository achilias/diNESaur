#pragma once

#include <cstdint>
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