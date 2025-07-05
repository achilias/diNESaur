#include "ppu.h"

bool PPU::run(size_t cycles) {
	scanline_pixel += cycles;

	if (scanline_n == 240) {
		scanline_n++;
		bus.nmi = true;
		return false;
	}

	if (scanline_pixel >= 340) {
		scanline_pixel %= 340;
		scanline_n++;

		if (scanline_n >= 261) {
			scanline_n = 0;
			bus.nmi = false;
			
			uint16_t nt_base[] = {0x2000, 0x2400, 0x2800, 0x2c00};
			uint16_t nametable_start = nt_base[bus.ppu_ctrl.nt];
			for (int i = 0; i < 32; i++) {
				int x = i * 8;
				for (int j = 0; j < 30; j++) {
					int y = j * 8;
					uint8_t tile = bus.vram_read_byte(nametable_start + i * 30 + j);
					printf("Drawing tile : %d at x: %d, y: %d\n", tile, x , y);
					draw_tile(tile, x, y);
				}
			}
			return true;
		}
	}

	if (scanline_n >= 0 && scanline_n <= 239) {
		// TODO
	}

	return false;
}

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