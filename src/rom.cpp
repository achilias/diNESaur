#include "rom.h"
#include <iostream>
#include <cstring>
#include <assert.h>

void ROM::load(std::ifstream &stream) {
    if (!stream.good()) {
        std::cerr << "Error reading file\n";
    }

    stream.seekg(0, stream.end);
    size_t file_size = stream.tellg();
    stream.seekg(0, stream.beg);

    std::vector<char> buf(file_size);
    stream.read(&buf[0], static_cast<std::streamsize>(file_size));

    std::string magic_bytes(buf.begin(), buf.begin() + 4);
    if (magic_bytes.compare(SIGNATURE) != 0)
        std::cerr << "ROM file not of iNES type\n";

    prg_size = buf[4] * 16384;
    chr_size = buf[5] * 8192;
    nt_mirror = static_cast<MirrorMode>(buf[6] & 0x1);
    mapper = (buf[7] & 0xF0) | (buf[6] >> 4);
    
    for (int i = 0 ; i < prg_size; i++) {
        prg_data.push_back(buf[0x10 + i]);
    }
    for (int i = 0 ; i < chr_size; i++) {
        chr_data.push_back(buf[0x10 + i + prg_size]);
    }

    assert(prg_data.size() == prg_size);
    assert(chr_data.size() == chr_size);
}

ROM::ROM(std::ifstream &stream) : prg_data(0), chr_data(0) {
    load(stream);
}


uint8_t ROM::read_byte_prg(uint16_t addr) const {
    assert(addr < prg_size);
    return prg_data[addr];
};

uint8_t ROM::read_byte_chr(uint16_t addr) const {
    // printf("(read_byte_chr) ROM: %p\n", this);
    assert(addr >= 0);
    assert(chr_data.size() == chr_size);
    assert(addr < chr_size);
    return chr_data[addr];
};
