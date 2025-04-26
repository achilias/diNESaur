#ifndef ADDRESS_H
#define ADDRESS_H

#include "common.h"

class Address {
public:
    Address(uint16_t addr) : addr(addr) {};
    inline bool in_range(uint16_t start, uint16_t end) const {return addr >= start && addr <= end;};
    uint16_t get_mapped_cpu() const;
private:
    uint16_t addr;
};

#endif