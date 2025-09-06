#include <vector>
#include <cstdint>
#include <fstream>

#define SIGNATURE "NES\x1A" // start of file magic bytes, ASCII string "NES" + "^Z" (msdos EOF)

enum class MirrorMode {
    HORIZONTAL,
    VERTICAL
};

class ROM {
public:
    ROM() = default; // for testing with dummy empty roms
    explicit ROM(std::ifstream &stream); 
    uint32_t prg_size;
    uint32_t chr_size;
    uint8_t mapper;
    MirrorMode nt_mirror;

    uint8_t read_byte_prg(uint16_t addr) const;
    uint8_t read_byte_chr(uint16_t addr) const;

private:
    std::vector<uint8_t> prg_data {};
    std::vector<uint8_t> chr_data {};
};