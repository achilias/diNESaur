#include <vector>
#include <cstdint>
#include <fstream>
#include <ios>

#define SIGNATURE "NES\x1A" // start of file magic bytes, ASCII string "NES" + "^Z" (msdos EOF)

class ROM {
public:
    ROM(std::ifstream &stream); 
    uint8_t prg_size;
    uint8_t chr_size;
    uint8_t mapper;

private:
    std::vector<uint8_t> prg_data;
    std::vector<uint8_t> chr_data;

};