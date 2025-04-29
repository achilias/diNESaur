#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

class Memory {
public:
    explicit Memory(std::size_t mem_size) : buffer(mem_size) {};
    inline uint8_t read_byte(uint16_t addr) const { return buffer[mirror(addr)]; };
    inline uint16_t read_two_bytes(uint16_t addr) const { return ((uint16_t) buffer[mirror(addr + 1)]) << 8 | buffer[mirror(addr)]; };
    inline void write_byte(uint16_t addr, uint8_t val) { buffer[mirror(addr)] = val; };
    inline void write_two_bytes(uint16_t addr, uint16_t val) { write_byte(mirror(addr), (uint8_t) (val & 0xFF)); write_byte(mirror(addr) + 1, (uint8_t) (val >> 8)); };
    inline bool in_range(uint16_t addr, uint16_t start, uint16_t end) const {return addr >= start && addr <= end;};
	virtual uint16_t mirror(uint16_t addr) const = 0;
private:
    std::vector<uint8_t> buffer;
};

#endif