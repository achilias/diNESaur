#include <SDL3/SDL_main.h>
#include <fstream>
#include <ios>

#include "nes.h"
#include "graphics.h"

int main(int argc, char *argv[]) {
    std::ifstream file(argv[1], std::ios::binary);

    NES nes(file);

    nes.run();

    return 0;
}