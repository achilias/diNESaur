#include <vector>
#include <cstdint>
#include <fstream>
#include <ios>

#define SIGNATURE "NES\x1A" // start of file magic bytes, ASCII string "NES" + "^Z" (msdos EOF)

class ROM {
public:
    ROM() {}; // for testing with dummy empty roms
    ROM(std::ifstream &stream); 
    uint8_t prg_size;
    uint8_t chr_size;
    uint8_t mapper;

    void load(std::ifstream &stream);
    uint8_t read_byte_prg(uint16_t addr) const { return prg_data[addr]; };

private:
    std::vector<uint8_t> prg_data;
    std::vector<uint8_t> chr_data;

};