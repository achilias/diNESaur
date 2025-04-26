#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

class Memory {
public:
    explicit Memory(std::size_t mem_size) : buffer(mem_size) {};
    inline uint8_t read_byte(uint16_t addr) { return buffer[addr]; };
    inline uint16_t read_two_bytes(uint16_t addr) { return ((uint16_t) buffer[addr]) << 8 | buffer[addr + 1]; };
    inline void write_byte(uint16_t addr, uint8_t val) { buffer[addr] = val; };
    inline void write_two_bytes(uint16_t addr, uint16_t val) { write_byte(addr, (uint8_t) (val & 0xFF)); write_byte(addr + 1, (uint8_t) (val >> 8)); };
private:
    std::vector<uint8_t> buffer;
};

#endif