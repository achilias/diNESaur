#include "rom.h"
#include <iostream>
#include <cstring>

void ROM::load(std::ifstream &stream) {
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

    prg_size = buf[4] * 16384;
    chr_size = buf[5] * 8192;
    mapper = (buf[7] & 0xF0) | (buf[6] >> 4);
    
    prg_data.resize(prg_size);
    chr_data.resize(chr_size);
    memcpy(prg_data.data(), &buf[0x10], prg_size);
    memcpy(chr_data.data(), &buf[0x10 + prg_size], chr_size);


}

ROM::ROM(std::ifstream &stream) : prg_data(0), chr_data(0) {
    load(stream);
}