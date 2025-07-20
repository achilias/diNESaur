#include "ppu.h"

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

		for (int i = 0; i < 64; i++)
			draw_sprite(i);

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

union palette_offset_t {
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

uint8_t PPU::attr_tb_lookup(uint32_t tile_n) {
	uint16_t at_start = at_base[bus.ppu_ctrl.nt];
	uint16_t nt_y = tile_n / 32;
	uint16_t nt_x = tile_n % 32;
	uint16_t at_idx = (nt_y / 4 )* 8 + nt_x / 4;

	attr_tb_byte_t at_byte = {.raw = bus.vram_read_byte(at_start + at_idx)};
		if (((nt_x % 4) / 2) == 0) {
		if (((nt_y % 4) / 2) == 0)
			return at_byte.top_left;

		return at_byte.bottom_left;
	} else {
		if (((nt_y % 4) / 2) == 0)
			return at_byte.top_right;

		return at_byte.bottom_right;
	}
}

void PPU::draw_tile(uint32_t tile_n, uint32_t base_x, uint32_t base_y) {

	uint16_t pt_tile_offset = bus.vram_read_byte(nt_base[bus.ppu_ctrl.nt] + tile_n) * 16;

	uint16_t tile_start = pt_tile_offset + pt_base[bus.ppu_ctrl.bg_pt];

	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = 0 == (bus.vram_read_byte(tile_start + y) & (0x80 >> x))
						? 0 : 1;
			uint8_t msb = 0 == (bus.vram_read_byte(tile_start + y + 8) & (0x80 >> x))
						? 0 : 1;

			palette_offset_t offset = {
				.pixel_val = (uint8_t) (msb << 1 | lsb),
				.palette =  attr_tb_lookup(tile_n),
				.bg_spr = 0,
				.dummy = 0
			};

			uint32_t colour = palette[bus.vram_read_byte(BG_PALETTES_BASE + offset.raw)];

			set_pixel(framebuffer.data(), base_x + x, base_y + y, colour);
		}
}

void PPU::draw_sprite(uint8_t sprite_n) {
	uint8_t base_y = bus.oam_read_byte(sprite_n * 4);
	uint16_t tile_idx = bus.oam_read_byte(sprite_n * 4 + 1);
	if (bus.ppu_ctrl.sprite_sz)
		printf("Attempted to draw 8x16 sprite. This is unimplemented!\n");
	uint16_t attr = bus.oam_read_byte(sprite_n * 4 + 2);
	uint8_t base_x = bus.oam_read_byte(sprite_n * 4 + 3);

	uint16_t tile_start = 16 * tile_idx + pt_base[bus.ppu_ctrl.sprite_pt];
	bool flip_hrz = (attr & 0b01000000) != 0;
	bool flip_vrt = (attr & 0b10000000) != 0;

	uint16_t palette_base = SPR_PALETTES_BASE + (attr & 0b11) * 4;

	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = 0 == (bus.vram_read_byte(tile_start + y) & (0x80 >> x))
						? 0 : 1;
			uint8_t msb = 0 == (bus.vram_read_byte(tile_start + y + 8) & (0x80 >> x))
						? 0 : 1;

			uint16_t palette_offset = (msb << 1) | lsb;

			if (!palette_offset)
				continue;
			
			uint32_t colour = palette[bus.vram_read_byte(palette_base + palette_offset)];

			uint32_t pix_x = flip_hrz ? base_x + 7 - x : base_x + x;
			uint32_t pix_y = flip_vrt ? base_y + 7 - y : base_y + y;
			// TODO: implement proper out of bounds sprite behavior
			if (pix_x >= 256 || pix_y >= 240)
				continue;
				
			set_pixel(framebuffer.data(), pix_x, pix_y, colour);
		}

}