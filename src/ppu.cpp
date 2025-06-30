#include "ppu.h"
#include <iostream>

static void set_pixel(uint32_t *buf, uint32_t x, uint32_t y, uint32_t colour) {
	buf[y * SCREEN_WIDTH + x] = colour;
}

void PPU::draw_tile(uint32_t i, uint32_t loc_x, uint32_t loc_y) {
	uint32_t loc = i * 16;
	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = bus.vram_read_byte(loc + y) & (0x80 >> x);
			uint8_t msb = bus.vram_read_byte(loc + y + 8) & (0x80 >> x);
			uint8_t palette_idx = ((msb != 0) << 1) | (lsb != 0);
			uint32_t colour = palette_idx == 0 ? 0xffffffff : palette_idx == 1 ? 0xffff0000 : palette_idx == 2 ? 0xff00ff00 : 0xff0000ff;

			set_pixel(framebuffer, loc_x + x, loc_y + y, colour);
		}
}