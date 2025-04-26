#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

class Memory {
public:
    Memory(std::size_t mem_size) : buffer(mem_size) {};
    uint8_t read(uint16_t addr);
private:
    std::vector<uint8_t> buffer;
};

#endif