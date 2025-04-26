#include "address.h"

uint16_t Address::get_mapped_cpu() const {
    if (in_range(0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(0x2000, 0x3fff))
        return addr & 0x2007;
    return addr;
}