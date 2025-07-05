#include "ppu.h"

void PPU::print_nametables() {
	uint16_t nt_base[] = {0x2000, 0x2400, 0x2800, 0x2c00};
	for (int nt = 0; nt < 4; nt++) {
		printf("Printing nametable %d: \n", nt);
		uint16_t nt_start = nt_base[nt];
		for (int i = 0; i < 32; i++) {
			for (int j = 0; j < 30; j++) {
				uint16_t tile_offset = i * 30 + j;
				uint8_t tile = bus.vram_read_byte(nt_start + tile_offset);
				printf("%d ", tile);
			}
			printf("\n");
		}
		
	}
}

bool PPU::run(size_t cycles) {
	scanline_pixel += cycles;

	if (scanline_n == 240) {
		scanline_n++;
		if (bus.ppu_ctrl.vblank_enable) {
			// bus.nmi = true;
			bus.ppu_status.vblank = true;
			bus.nmi = true;
		}

		uint16_t nt_base[] = {0x2000, 0x2400, 0x2800, 0x2c00};
		uint16_t nt_start = nt_base[bus.ppu_ctrl.nt];
		for (int i = 0; i < 30; i++) {
			for (int j = 0; j < 32; j++) {
				int x = j * 8;
				int y = i * 8;
				uint16_t tile_offset = i * 32 + j;
				uint8_t tile = bus.vram_read_byte(nt_start + tile_offset);
				draw_tile(tile, x, y);
			}
		}
		return true;
	}

	if (scanline_pixel >= 340) {
		scanline_pixel %= 340;
		scanline_n++;

		if (scanline_n >= 261) {
			bus.ignore_ctrl_writes = 0;
			scanline_n = 0;
			bus.nmi = false;
			bus.ppu_status.vblank = false;

			// printf("End of frame!\n");
			
			return false;
		}
	}

	return false;
}

static void set_pixel(uint32_t *buf, uint32_t x, uint32_t y, uint32_t colour) {
	buf[y * SCREEN_WIDTH + x] = colour;
}

void PPU::draw_tile(uint32_t i, uint32_t loc_x, uint32_t loc_y) {
	uint32_t loc = i * 16 + (bus.ppu_ctrl.bg_pt ? 0x1000 : 0x0);
	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = bus.vram_read_byte(loc + y) & (0x80 >> x);
			uint8_t msb = bus.vram_read_byte(loc + y + 8) & (0x80 >> x);
			uint8_t palette_idx = ((msb != 0) << 1) | (lsb != 0);
			#define RED 0xffff0000
			#define GREEN 0xff00ff00
			#define BLUE 0xff0000ff
			#define BLACK 0xffffffff

			uint32_t colour = palette_idx == 0 ? GREEN : palette_idx == 1 ? RED : palette_idx == 2 ? BLACK : BLUE;

			set_pixel(framebuffer, loc_x + x, loc_y + y, colour);
		}
}