#include "rom.h"
#include <iostream>

ROM::ROM(std::ifstream &stream) {
    if (!stream.good()) {
        std::cerr << "Error reading file\n";
    }

    stream.seekg(0, stream.end);
    size_t file_size = stream.tellg();
    stream.seekg(0, stream.beg);

    std::vector<char> buf(file_size);
    stream.read(&buf[0], static_cast<std::streamsize>(file_size));
    stream.close();

    std::string magic_bytes(buf.begin(), buf.begin() + 4);
    if (magic_bytes.compare(SIGNATURE) != 0)
        std::cerr << "ROM file not of iNES type\n";

    prg_size = buf[4];
    chr_size = buf[5];
    mapper = buf[7] & 0xF0 | buf[6] >> 4;

}