#include <fstream>
#include <ios>

#include "nes.h"

void display_init();
void display_finish( );
void render_and_draw(uint32_t *buffer);

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s rom_path\n", argv[0]);
        return 1;
    }

    std::ifstream file = std::ifstream(argv[1], std::ios::binary);

    display_init();

    NES nes;
    nes_init(&nes, file);

    nes_run(&nes, &render_and_draw);

    display_finish();

    return 0;
}
