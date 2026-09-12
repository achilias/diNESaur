#include <stdio.h>

#include "core/nes.h"
#include "display.h"
#include "input.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Usage: %s rom_path\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "rb");

    display_init();

    NES nes;
    nes_init(&nes, file);
    fclose(file);

    nes_run(&nes, &render_and_draw, &poll_for_input);

    display_finish();

    return 0;
}
