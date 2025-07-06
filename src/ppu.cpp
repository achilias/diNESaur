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

	if (scanline_pixel < 340)
		return false;

	scanline_pixel %= 340;
	scanline_n++;

	if (scanline_n >= 261) {
		bus.ignore_ctrl_writes = 0;
		scanline_n = 0;
		bus.ppu_status.vblank = false;
	}
	

	if (scanline_n == 241) {
		scanline_n++;
		bus.ppu_status.vblank = true;
		if (bus.ppu_ctrl.vblank_enable) {
			bus.nmi = true;
		}

		for (int i = 0; i < 30; i++) {
			for (int j = 0; j < 32; j++) {
				draw_tile(i * 32 + j, j * 8, i * 8);
			}
		}
		return true;
	}

	return false;
}

static void set_pixel(uint32_t *buf, uint32_t x, uint32_t y, uint32_t colour) {
	buf[y * SCREEN_WIDTH + x] = colour;
}

// ARGB8888
const uint32_t palette[] = {
   0xFF808080, 0xFF003DA6, 0xFF0012B0, 0xFF440096, 0xFFA1005E,
   0xFFC70028, 0xFFBA0600, 0xFF8C1700, 0xFF5C2F00, 0xFF104500,
   0xFF054A00, 0xFF00472E, 0xFF004166, 0xFF000000, 0xFF050505,
   0xFF050505, 0xFFC7C7C7, 0xFF0077FF, 0xFF2155FF, 0xFF8237FA,
   0xFFEB2FB5, 0xFFFF2950, 0xFFFF2200, 0xFFD63200, 0xFFC46200,
   0xFF358000, 0xFF058F00, 0xFF008A55, 0xFF0099CC, 0xFF212121,
   0xFF090909, 0xFF090909, 0xFFFFFFFF, 0xFF0FD7FF, 0xFF69A2FF,
   0xFFD480FF, 0xFFFF45F3, 0xFFFF618B, 0xFFFF8833, 0xFFFF9C12,
   0xFFFABC20, 0xFF9FE30E, 0xFF2BF035, 0xFF0CF0A4, 0xFF05FBFF,
   0xFF5E5E5E, 0xFF0D0D0D, 0xFF0D0D0D, 0xFFFFFFFF, 0xFFA6FCFF,
   0xFFB3ECFF, 0xFFDAABEB, 0xFFFFA8F9, 0xFFFFABB3, 0xFFFFD2B0,
   0xFFFFEFA6, 0xFFFFF79C, 0xFFD7E895, 0xFFA6EDAF, 0xFFA2F2DA,
   0xFF99FFFC, 0xFFDDDDDD, 0xFF111111, 0xFF111111
};

#define BG_PALETTES_BASE 0x3f00
#define SPR_PALETTES_BASE 0x3f10

union palette_idx_t {
	uint8_t raw;
	struct {
		uint8_t pixel_val : 2;
		uint8_t palette   : 2;
		bool 	bg_spr    : 1;
		uint8_t dummy	  : 3;
	};
};

union attr_tb_byte_t {
	uint8_t raw;
	struct {
		uint8_t top_left 	: 2;
		uint8_t top_right 	: 2;
		uint8_t bottom_left : 2;
		uint8_t bottom_right: 2;
	};	
};

uint16_t nt_base[]	= {0x2000, 0x2400, 0x2800, 0x2c00};
uint16_t at_base[]	= {0x23c0, 0x27c0, 0x2bc0, 0x2fc0};
uint16_t pt_base[]	= {0x0, 0x1000};

// only for bg tiles . TODO: sprites
void PPU::draw_tile(uint32_t tile_n, uint32_t loc_x, uint32_t loc_y) {

	uint16_t tile_pt_idx = bus.vram_read_byte(nt_base[bus.ppu_ctrl.nt] + tile_n);

	uint16_t tile_start = tile_pt_idx * 16 + pt_base[bus.ppu_ctrl.bg_pt];

	uint16_t at_start = at_base[bus.ppu_ctrl.nt];
	uint16_t nt_y = tile_n / 32;
	uint16_t nt_x = tile_n % 32;
	uint16_t at_idx = (nt_y / 4 )* 8 + nt_x / 4;

	attr_tb_byte_t at_byte = {.raw = bus.vram_read_byte(at_start + at_idx)};
	uint8_t at_bits = 0x0;
	if (((nt_x % 4) / 2) == 0) {
		if (((nt_y % 4) / 2) == 0)
			at_bits = at_byte.top_left;
		else
			at_bits = at_byte.bottom_left;
	} else {
		if (((nt_y % 4) / 2) == 0)
			at_bits = at_byte.top_right;
		else
			at_bits = at_byte.bottom_right;
	}

	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = bus.vram_read_byte(tile_start + y) & (0x80 >> x);
			uint8_t msb = bus.vram_read_byte(tile_start + y + 8) & (0x80 >> x);

			uint8_t val = (((uint8_t) (msb != 0)) << 1) | (uint8_t)(lsb != 0);

			palette_idx_t idx = {
				.pixel_val = val,
				.palette =  at_bits,
				.bg_spr = 0,
				.dummy = 0
			};

			uint32_t colour = palette[bus.vram_read_byte(BG_PALETTES_BASE + idx.raw)];

			set_pixel(framebuffer, loc_x + x, loc_y + y, colour);
		}
}