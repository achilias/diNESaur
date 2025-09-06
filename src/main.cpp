#include <SDL3/SDL_main.h>
#include <fstream>
#include <ios>

#include "nes.h"

int main(int argc, char *argv[]) {
    std::ifstream file(argv[1], std::ios::binary);
    Controller ctrl;

    NES nes(new ROM(file), ctrl);

    nes.run();

    return 0;
}