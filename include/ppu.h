#pragma once

#include "bus.h"

class PPU {
public:
    PPU(Bus& bus) : bus(bus) {};
private:
    Bus& bus;
};