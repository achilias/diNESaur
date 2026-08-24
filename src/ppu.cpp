#include "ppu.h"
#include "nes.h"

static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {
    return addr >= start && addr <= end;
}

static uint16_t vram_mirror(uint16_t addr, ROM const *rom) {
    if (in_range(addr, 0, 0x1fff))
        return addr % rom->chr_size;

    if (rom->nt_mirror == MirrorMode::HORIZONTAL) {
        if (in_range(addr, 0x2000, 0x23ff))
            return addr;
        if (in_range(addr, 0x2400, 0x27ff))
            return addr - 0x400;
        if (in_range(addr, 0x2800, 0x2bff))
            return addr;
        if (in_range(addr, 0x2c00, 0x2fff))
            return addr - 0x400;
    }

    if (rom->nt_mirror == MirrorMode::VERTICAL) {
        if (in_range(addr, 0x2000, 0x23ff))
            return addr;
        if (in_range(addr, 0x2400, 0x27ff))
            return addr;
        if (in_range(addr, 0x2800, 0x2bff))
            return addr - 0x800;
        if (in_range(addr, 0x2c00, 0x2fff))
            return addr - 0x800;
    }

    if (in_range(addr, 0x3f00, 0x3fff))
        return addr & 0x3f1f;
    return addr % 0x4000;
}

uint8_t ppu_read_vram_byte(PPU *ppu, uint16_t addr) {
    if (in_range(addr, 0x0, 0x1fff))
        return ppu->nes->rom->read_byte_chr(vram_mirror(addr, ppu->nes->rom));

    return ppu->vram[vram_mirror(addr, ppu->nes->rom)];
};

uint16_t ppu_read_vram_two_bytes(PPU *ppu, uint16_t addr) { return ((uint16_t) ppu->vram[vram_mirror(addr + 1, ppu->nes->rom)]) << 8 | ppu->vram[vram_mirror(addr, ppu->nes->rom)]; };

void ppu_write_vram_byte(PPU *ppu, uint16_t addr, uint8_t val) {
    addr = vram_mirror(addr, ppu->nes->rom);
    if (addr > ppu->vram.size()) {
        printf("Out of bounds vram %p\n", addr);

        exit(0);
    }
    ppu->vram[addr] = val;
};

void ppu_write_vram_two_bytes(PPU *ppu, uint16_t addr, uint16_t val) { ppu_write_vram_byte(ppu, vram_mirror(addr, ppu->nes->rom), (uint8_t) (val & 0xFF)); ppu_write_vram_byte(ppu, vram_mirror(addr, ppu->nes->rom) + 1, (uint8_t) (val >> 8)); };

void ppu_reset(PPU *ppu) {
	ppu->scanline_pixel = 0;
	ppu->scanline_n = 0;
	ppu->framebuffer.fill(0xff000000);
}

void ppu_init(PPU *ppu, Bus *bus, NES *nes)
{
	ppu->bus = bus;
	ppu->nes = nes;
	ppu_reset(ppu);
}

bool PPU::run(size_t cycles) {
	scanline_pixel += cycles;

	if (scanline_pixel < 340)
		return false;

	scanline_pixel %= 340;
	scanline_n++;

	if (scanline_n >= 261) {
		bus->ignore_ctrl_writes = false;
		scanline_n = 0;
		PPUSTATUS_SET_VBLANK(bus->ppu_status, false);
	}
	

	if (scanline_n == 241) {
		scanline_n++;
		PPUSTATUS_SET_VBLANK(bus->ppu_status, true);
		if (PPUCTRL_VBLANK_ENABLE(bus->ppu_ctrl)) {
			nes->nmi = true;
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



uint16_t nt_base[]	= {0x2000, 0x2400, 0x2800, 0x2c00};
uint16_t at_base[]	= {0x23c0, 0x27c0, 0x2bc0, 0x2fc0};
uint16_t pt_base[]	= {0x0, 0x1000};

#define ATTR_TB_BYTE_TOP_LEFT(attr_tb_byte)			((attr_tb_byte) & 0b11)
#define ATTR_TB_BYTE_TOP_RIGHT(attr_tb_byte)		(((attr_tb_byte) & 0b1100) >> 2)
#define ATTR_TB_BYTE_BOTTOM_LEFT(attr_tb_byte)		(((attr_tb_byte) & 0b110000) >> 4)
#define ATTR_TB_BYTE_BOTTOM_RIGHT(attr_tb_byte)		(((attr_tb_byte) & 0b11000000) >> 6)

uint8_t PPU::attr_tb_lookup(uint32_t tile_n) {
	uint16_t at_start = at_base[PPUCTRL_NT(bus->ppu_ctrl)];
	uint16_t nt_y = tile_n / 32;
	uint16_t nt_x = tile_n % 32;
	uint16_t at_idx = (nt_y / 4 )* 8 + nt_x / 4;

	uint8_t at_byte = ppu_read_vram_byte(this, at_start + at_idx);
	if (((nt_x % 4) / 2) == 0) {
		if (((nt_y % 4) / 2) == 0)
			return ATTR_TB_BYTE_TOP_LEFT(at_byte);

		return ATTR_TB_BYTE_BOTTOM_LEFT(at_byte);
	} else {
		if (((nt_y % 4) / 2) == 0)
			return ATTR_TB_BYTE_TOP_RIGHT(at_byte);

		return ATTR_TB_BYTE_BOTTOM_RIGHT(at_byte);
	}
}
#define PAL_OFFSET_PIXEL_VAL_SET(offset, val)	{offset = ((offset) & ~0b11) | ((val) & 0b11);}
#define PAL_OFFSET_PALETTE_SET(offset, val)		{offset = ((offset) & ~0b1100) | (((val) & 0b11) << 2);}
#define PAL_OFFSET_BG_SPR_SET(offset, val)		{offset = ((offset) & ~0b10000) | (((uint8_t)(val)) << 4);}


void PPU::draw_tile(uint32_t tile_n, uint32_t base_x, uint32_t base_y) {

	uint16_t pt_tile_offset = ppu_read_vram_byte(this, nt_base[PPUCTRL_NT(bus->ppu_ctrl)] + tile_n) * 16;

	uint16_t tile_start = pt_tile_offset + pt_base[PPUCTRL_BG_PT(bus->ppu_ctrl)];

	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = 0 == (ppu_read_vram_byte(this, tile_start + y) & (0x80 >> x))
						? 0 : 1;
			uint8_t msb = 0 == (ppu_read_vram_byte(this, tile_start + y + 8) & (0x80 >> x))
						? 0 : 1;

			uint8_t offset = 0;
			PAL_OFFSET_PIXEL_VAL_SET(offset, (uint8_t)(msb << 1 | lsb));
			PAL_OFFSET_PALETTE_SET(offset, attr_tb_lookup(tile_n));
			PAL_OFFSET_BG_SPR_SET(offset, false);

			uint32_t colour = palette[ppu_read_vram_byte(this, BG_PALETTES_BASE + offset)];

			set_pixel(framebuffer.data(), base_x + x, base_y + y, colour);
		}
}

void PPU::draw_sprite(uint8_t sprite_n) {
	uint8_t base_y = bus->oam[sprite_n * 4];
	uint16_t tile_idx = bus->oam[sprite_n * 4 + 1];
	if (PPUCTRL_SPRITE_SZ(bus->ppu_ctrl))
		printf("Attempted to draw 8x16 sprite. This is unimplemented!\n");
	uint16_t attr = bus->oam[sprite_n * 4 + 2];
	uint8_t base_x = bus->oam[sprite_n * 4 + 3];

	uint16_t tile_start = 16 * tile_idx + pt_base[PPUCTRL_SPRITE_PT(bus->ppu_ctrl)];
	bool flip_hrz = (attr & 0b01000000) != 0;
	bool flip_vrt = (attr & 0b10000000) != 0;

	uint16_t palette_base = SPR_PALETTES_BASE + (attr & 0b11) * 4;

	for (int x = 0; x < 8; x++)
		for (int y = 0; y < 8; y++) {
			uint8_t lsb = 0 == (ppu_read_vram_byte(this, tile_start + y) & (0x80 >> x))
						? 0 : 1;
			uint8_t msb = 0 == (ppu_read_vram_byte(this, tile_start + y + 8) & (0x80 >> x))
						? 0 : 1;

			uint16_t palette_offset = (msb << 1) | lsb;

			if (!palette_offset)
				continue;
			
			uint32_t colour = palette[ppu_read_vram_byte(this, palette_base + palette_offset)];

			uint32_t pix_x = flip_hrz ? base_x + 7 - x : base_x + x;
			uint32_t pix_y = flip_vrt ? base_y + 7 - y : base_y + y;
			// TODO: implement proper out of bounds sprite behavior
			if (pix_x >= 256 || pix_y >= 240)
				continue;
				
			set_pixel(framebuffer.data(), pix_x, pix_y, colour);
		}

}