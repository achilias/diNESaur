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
    mapper = (buf[7] & 0xF0) | (buf[6] >> 4);
    
    // prg_data.resize(prg_size);
    // chr_data.resize(chr_size);
    // memcpy(prg_data.data(), &buf[0x10], prg_size);
    for (int i = 0 ; i < prg_size; i++) {
        prg_data.push_back(buf[0x10 + i]);
    }
    // memcpy(chr_data.data(), &buf[0x10 + prg_size], chr_size);
    for (int i = 0 ; i < chr_size; i++) {
        chr_data.push_back(buf[0x10 + i]);
    }

    assert(prg_data.size() == prg_size);
    assert(chr_data.size() == chr_size);
    assert(canary == 0xDEADBEEF);
}

ROM::ROM(std::ifstream &stream) : prg_data(0), chr_data(0) {
    load(stream);
}


uint8_t ROM::read_byte_prg(uint16_t addr) const {
    // printf("(read_byte_prg) ROM: %p\n", this);
    assert(canary == 0xDEADBEEF);
    assert(addr >= 0);
    // printf("Prg_size: %ld, prg_data.size(): %ld\n",prg_size, prg_data.size());
    assert(prg_data.size() == prg_size);
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
