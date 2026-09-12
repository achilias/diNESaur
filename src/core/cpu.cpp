#include "cpu.h"
#include "nes.h"

#define STACK_BASE 0x100

enum AddressingMode {
    ACCUMULATOR,
    IMMEDIATE,
    ZERO_PAGE,
    ZERO_PAGE_IDX_X,
    ZERO_PAGE_IDX_Y,
    ABSOLUTE,
    ABSOLUTE_IDX_X,
    ABSOLUTE_IDX_Y,
    INDIRECT,
    INDIRECT_IDX_X,
    INDIRECT_IDX_Y,
};

// TODO: implement cycle penalties for page crosses and taken branches

static bool in_range(uint16_t addr, uint16_t start, uint16_t end) {
    return addr >= start && addr <= end;
}

static uint16_t ram_mirror(uint16_t addr) {
    if (in_range(addr, 0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(addr, 0x2000, 0x3fff))
        return addr & 0x2007;

    return addr;
}


static void set_carry(CPU *cpu, bool cond)
{
    cpu->flags = cond ? cpu->flags | 0x1 : cpu->flags & ~0x1;
}
static bool get_carry(CPU *cpu)
{
    return cpu->flags & 0x1;
}
static void set_zero(CPU *cpu, bool cond)
{
    cpu->flags = cond ? cpu->flags | 0x2 : cpu->flags & ~0x2;
}
static bool get_zero(CPU *cpu)
{
    return cpu->flags & 0x2;
}
static void set_disable_interrupt(CPU *cpu, bool cond)
{
    cpu->flags = cond ? cpu->flags | 0x4 : cpu->flags & ~0x4;
}
static void set_overflow(CPU *cpu, bool cond)
{
    cpu->flags = cond ? cpu->flags | 0x40 : cpu->flags & ~0x40;
};
static void set_decimal(CPU *cpu, bool cond)
{
    cpu->flags = cond ? cpu->flags | 0x8 : cpu->flags & ~0x8;
}
static bool get_overflow(CPU *cpu)
{
    return cpu->flags & 0x40;
}
static void set_negative(CPU *cpu, bool cond)
{
    cpu->flags = cond ? cpu->flags | 0x80 : cpu->flags & ~0x80;
};
static bool get_negative(CPU *cpu)
{
    return cpu->flags & 0x80;
}

uint8_t cpu_read_byte(CPU *cpu, uint16_t addr) {
#ifdef TEST_BUILD
    // CPU opcode tests use the entire address space.
    // NES-specific address mirroring and memory-mapped I/O are bypassed in test builds.
    return cpu->ram[addr];
#endif
    // address belongs to cartridge-mapped address space
    if (in_range(addr, 0x4020, 0xffff))
        return cpu->nes->rom->read_byte_prg(addr);

    switch (ram_mirror(addr)) {
        case 0x2002:
        case 0x2004:
        case 0x2007:
            return ppu_read_register(cpu->nes->ppu, ram_mirror(addr));
        case 0x4016:
            return controller_read_serial_bit(cpu->nes->controller) ? 1 : 0;
    }

    return cpu->ram[ram_mirror(addr)];
}

uint16_t cpu_read_two_bytes(CPU *cpu, uint16_t addr) {
    return (((uint16_t) cpu_read_byte(cpu, addr + 1)) << 8) | cpu_read_byte(cpu, addr);
}

void cpu_write_byte(CPU *cpu, uint16_t addr, uint8_t val) {
#ifdef TEST_BUILD
    // CPU opcode tests use the entire address space.
    // NES-specific address mirroring and memory-mapped I/O are bypassed in test builds.
    cpu->ram[addr] = val;
    return;
#endif

    if (in_range(addr, 0x8000, 0xffff)) {
        printf("Error! Attempt to write %p to rom at %p\n", val, addr);
        exit(0);
    }

    switch (ram_mirror(addr)) {
        case 0x2000:
        case 0x2001:
        case 0x2003:
        case 0x2004:
        case 0x2005:
        case 0x2006:
        case 0x2007:
            ppu_write_register(cpu->nes->ppu, ram_mirror(addr), val);
            return;
        case 0x4014:
            // TODO: cycle penalty
            {
                uint16_t page_start = ((uint16_t) val) << 8;
                for (int i = 0; i < 256; i++)
                    cpu->nes->ppu->oam[cpu->nes->ppu->oam_addr++] = cpu_read_byte(cpu, page_start + i);
                return;
            }
        case 0x4016:
            if (val & 0x80)
                controller_set_strobe(cpu->nes->controller);
            else
                controller_clear_strobe(cpu->nes->controller);
            return;
    }
    cpu->ram[ram_mirror(addr)] = val;
}

void cpu_write_two_bytes(CPU *cpu, uint16_t addr, uint16_t val) { cpu_write_byte(cpu, ram_mirror(addr), (uint8_t) (val & 0xFF)); cpu_write_byte(cpu, ram_mirror(addr) + 1, (uint8_t) (val >> 8)); }

void cpu_handle_nmi(CPU *cpu) {
    cpu->nes->nmi = false;
    cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->flags | 0x20);
    cpu_write_two_bytes(cpu, STACK_BASE + cpu->sp, cpu->pc);
    cpu->sp -= 2;
    cpu->pc = cpu_read_two_bytes(cpu, 0xfffa);
}

void cpu_init(CPU *cpu, NES *nes)
{
    cpu->nes = nes;
    cpu_reset(cpu);
}

void cpu_reset(CPU *cpu) {
    memset(cpu->ram, 0, sizeof(cpu->ram));

    cpu->sp = 0xff;
    cpu->accum = 0;
    cpu->reg_x = 0;
    cpu->reg_y = 0;
    cpu->flags = 0;
    cpu->pc = cpu_read_two_bytes(cpu, 0xfffc);
}

uint16_t get_addr(CPU *cpu, AddressingMode mode) {
	switch (mode) {
		uint16_t tmp_u16;

		// used for zero page & indirect addressing to utilize default unsigned wraparound overflow behavior
		uint8_t tmp_u8;
		case IMMEDIATE:
			return cpu->pc++;
		case INDIRECT:
		case ABSOLUTE:
			tmp_u16 = cpu_read_two_bytes(cpu, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case ABSOLUTE_IDX_X:
			tmp_u16 = cpu->reg_x + cpu_read_two_bytes(cpu, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case ABSOLUTE_IDX_Y:
			tmp_u16 = cpu->reg_y + cpu_read_two_bytes(cpu, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case ZERO_PAGE:
			return cpu_read_byte(cpu, cpu->pc++);
		case ZERO_PAGE_IDX_X:
			tmp_u8 = cpu->reg_x + cpu_read_byte(cpu, cpu->pc++);
			return tmp_u8;
		case ZERO_PAGE_IDX_Y:
			tmp_u8 = cpu->reg_y + cpu_read_byte(cpu, cpu->pc++);
			return tmp_u8;
		case INDIRECT_IDX_X:
			tmp_u8 = cpu->reg_x + cpu_read_byte(cpu, cpu->pc++);
			tmp_u16 = cpu_read_byte(cpu, tmp_u8++);
			tmp_u16 += cpu_read_byte(cpu, tmp_u8) << 8;
			return tmp_u16;
		case INDIRECT_IDX_Y:
			tmp_u8 = cpu_read_byte(cpu, cpu->pc++);
			tmp_u16 = cpu_read_byte(cpu, tmp_u8++);
			tmp_u16 += (cpu_read_byte(cpu, tmp_u8) << 8) + cpu->reg_y;
			return tmp_u16;
        default:
            return 0;
	}
}

bool adc(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	uint16_t result = cpu->accum + operand + get_carry(cpu);
	set_carry(cpu, result > 0xff);
	result = (uint8_t) result;
	set_overflow(cpu, (result ^ cpu->accum) & (result ^ operand) & 0x80);
	cpu->accum = result;
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
    return false;
}

bool and_(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum &= cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
    return false;
}

void asl(CPU *cpu, AddressingMode addr_mode){
	if (addr_mode == ACCUMULATOR) {
		set_carry(cpu, cpu->accum & 0x80);
		cpu->accum = cpu->accum << 1;
		set_zero(cpu, cpu->accum == 0);
		set_negative(cpu, cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t operand = cpu_read_byte(cpu, addr);
    set_carry(cpu, operand & 0x80);
    uint8_t tmp = operand << 1;
	cpu_write_byte(cpu, addr, tmp);
    set_zero(cpu, tmp == 0);
    set_negative(cpu, tmp & 0x80);
}

void bcc(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if (!get_carry(cpu))
		cpu->pc += offset;
}

void bcs(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if (get_carry(cpu))
		cpu->pc += offset;
}

void beq(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(get_zero(cpu))
		cpu->pc += offset;
}

void bit(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	uint8_t tmp = cpu->accum & operand;
	set_zero(cpu, tmp == 0);
	set_overflow(cpu, operand & 0x40);
	set_negative(cpu, operand & 0x80);
}

void bmi(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(get_negative(cpu))
		cpu->pc += offset;
}

void bne(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(!get_zero(cpu))
		cpu->pc += offset;
}

void bpl(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(!get_negative(cpu))
		cpu->pc += offset;
}

void brk(CPU *cpu) {
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, ++cpu->pc >> 8);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->pc & 0xff);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->flags | 0x30); // break flag and extra bit (bits 4 & 5) should always be set: 0x30 = 00110000
	set_disable_interrupt(cpu, 1);
	cpu->pc = cpu_read_two_bytes(cpu, 0xfffe); // address of irq interrupt handler
}

void bvc(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(!get_overflow(cpu))
		cpu->pc += offset;
}

void bvs(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(get_overflow(cpu))
		cpu->pc += offset;
}

void clc(CPU *cpu) {
	set_carry(cpu, 0);
}

void cld(CPU *cpu) {
	cpu->flags &= ~0x8;
}

void cli(CPU *cpu) {
	set_disable_interrupt(cpu, 0);
}

void clv(CPU *cpu) {
	set_overflow(cpu, 0);
}

void cmp(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_carry(cpu, cpu->accum >= operand);
	auto result = (int8_t) (cpu->accum - operand);
	set_zero(cpu, result == 0);
	set_negative(cpu, result < 0);
}

void cpx(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_carry(cpu, cpu->reg_x >= operand);
	auto result = (int8_t) (cpu->reg_x - operand);
	set_zero(cpu, result == 0);
	set_negative(cpu, result < 0);
}

void cpy(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_carry(cpu, cpu->reg_y >= operand);
	auto result = (int8_t) (cpu->reg_y - operand);
	set_zero(cpu, result == 0);
	set_negative(cpu, result < 0);
}

void dec(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr) - 1;
	set_zero(cpu, tmp == 0);
	set_negative(cpu, tmp & 0x80);
	cpu_write_byte(cpu, addr, tmp);
}

void dex(CPU *cpu) {
	set_zero(cpu, --cpu->reg_x == 0);
	set_negative(cpu, cpu->reg_x & 0x80);
}

void dey(CPU *cpu) {
	set_zero(cpu, --cpu->reg_y == 0);
	set_negative(cpu, cpu->reg_y & 0x80);
}

void eor(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum ^= cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

void inc(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr) + 1;
	set_zero(cpu, tmp == 0);
	set_negative(cpu, tmp & 0x80);
	cpu_write_byte(cpu, addr, tmp);
}

void inx(CPU *cpu) {
	set_zero(cpu, ++cpu->reg_x == 0);
	set_negative(cpu, cpu->reg_x & 0x80);
}

void iny(CPU *cpu) {
	set_zero(cpu, ++cpu->reg_y == 0);
	set_negative(cpu, cpu->reg_y & 0x80);
}

void jmp(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
    if (addr_mode == INDIRECT && (addr & 0xff) == 0xff) {
        /* CPU quirk in nes version of 6502
         * See https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
         */
        cpu->pc = (((uint16_t) cpu_read_byte(cpu, addr & 0xff00)) << 8) | cpu_read_byte(cpu, addr);
        return;
    }
	cpu->pc = addr_mode == INDIRECT ? cpu_read_two_bytes(cpu, addr) : addr;
}

void jsr(CPU *cpu) {
	uint16_t addr = cpu_read_two_bytes(cpu, cpu->pc++);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->pc >> 8);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->pc & 0xff);
	cpu->pc = addr;
}

void lda(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

void ldx(CPU *cpu, AddressingMode addr_mode) {
	cpu->reg_x = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_zero(cpu, cpu->reg_x == 0);
	set_negative(cpu, cpu->reg_x & 0x80);
}

void ldy(CPU *cpu, AddressingMode addr_mode) {
	cpu->reg_y = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_zero(cpu, cpu->reg_y == 0);
	set_negative(cpu, cpu->reg_y & 0x80);
}

void lsr(CPU *cpu, AddressingMode addr_mode) {
	if (addr_mode == ACCUMULATOR) {
		set_carry(cpu, cpu->accum & 0x1);
		cpu->accum = cpu->accum >> 1;
		set_zero(cpu, cpu->accum == 0);
		set_negative(cpu, cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr);
	set_carry(cpu, tmp & 0x1);
	tmp = tmp >> 1;
	set_zero(cpu, tmp == 0);
	set_negative(cpu, tmp & 0x80);
	cpu_write_byte(cpu, addr, tmp);
}

void ora(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum |= cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

void pha(CPU *cpu) {
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->accum);
}

void php(CPU *cpu) {
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->flags | 0x30);
}

void pla(CPU *cpu) {
	cpu->accum = cpu_read_byte(cpu, STACK_BASE + ++cpu->sp);
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

void plp(CPU *cpu) {
	// ignore break flag (bit 4): 0xef = 11101111 and set extra bit (bit 5): 0x20 = 00100000
	cpu->flags = 0x20 | (cpu_read_byte(cpu, STACK_BASE + ++cpu->sp) & 0xef);
}

void rol(CPU *cpu, AddressingMode addr_mode) {
	if (addr_mode == ACCUMULATOR) {
		uint8_t tmp = (cpu->accum << 1) | get_carry(cpu);
		set_carry(cpu, cpu->accum & 0x80);
		cpu->accum = tmp;
		set_zero(cpu, cpu->accum == 0);
		set_negative(cpu, cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr);
	uint8_t tmp_ = (tmp << 1) | get_carry(cpu);
	set_carry(cpu, tmp & 0x80);
	set_zero(cpu, tmp_ == 0);
	set_negative(cpu, tmp_ & 0x80);
	cpu_write_byte(cpu, addr, tmp_);
}

void ror(CPU *cpu, AddressingMode addr_mode) {
	if (addr_mode == ACCUMULATOR) {
		uint8_t tmp = (cpu->accum >> 1) | (((uint8_t) get_carry(cpu)) << 7);
		set_carry(cpu, cpu->accum & 0x1);
		cpu->accum = tmp;
		set_zero(cpu, cpu->accum == 0);
		set_negative(cpu, cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr);
	uint8_t tmp_ = (tmp >> 1) | (((uint8_t) get_carry(cpu)) << 7);
	set_carry(cpu, tmp & 0x1);
	set_zero(cpu, tmp_ == 0);
	set_negative(cpu, tmp_ & 0x80);
	cpu_write_byte(cpu, addr, tmp_);
}

void rti(CPU *cpu) {
	cpu->flags = 0x20 | (cpu_read_byte(cpu, STACK_BASE + ++cpu->sp) & 0xef);
    uint8_t pcl = cpu_read_byte(cpu, STACK_BASE + ++cpu->sp);
    uint8_t pch = cpu_read_byte(cpu, STACK_BASE + ++cpu->sp);
	cpu->pc = (((uint16_t) pch) << 8) | pcl;
}

void rts(CPU *cpu) {
    uint8_t pcl = cpu_read_byte(cpu, STACK_BASE + ++cpu->sp);
    uint8_t pch = cpu_read_byte(cpu, STACK_BASE + ++cpu->sp);
    cpu->pc = ((((uint16_t) pch) << 8) | pcl) + 1;
}

void sbc(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ~cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	uint16_t result = cpu->accum + operand + get_carry(cpu);
	set_carry(cpu, result > 0xff);
	result = (uint8_t) result;
	set_overflow(cpu, (result ^ cpu->accum) & (result ^ operand) & 0x80);
	cpu->accum = result;
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

void sec(CPU *cpu) {
	set_carry(cpu, 1);
}

void sed(CPU *cpu) {
	set_decimal(cpu, 1);
}

void sei(CPU *cpu) {
	set_disable_interrupt(cpu, 1);
}

void sta(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	cpu_write_byte(cpu, addr, cpu->accum);
}

void stx(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	cpu_write_byte(cpu, addr, cpu->reg_x);
}

void sty(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	cpu_write_byte(cpu, addr, cpu->reg_y);
}

void tax(CPU *cpu) {
	cpu->reg_x = cpu->accum;
	set_zero(cpu, cpu->reg_x == 0);
	set_negative(cpu, cpu->reg_x & 0x80);
}

void tay(CPU *cpu) {
	cpu->reg_y = cpu->accum;
	set_zero(cpu, cpu->reg_y == 0);
	set_negative(cpu, cpu->reg_y & 0x80);
}

void tsx(CPU *cpu) {
	cpu->reg_x = cpu->sp;
	set_zero(cpu, cpu->reg_x == 0);
	set_negative(cpu, cpu->reg_x & 0x80);
}

void txa(CPU *cpu) {
	cpu->accum = cpu->reg_x;
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

static void txs(CPU *cpu) {
	cpu->sp = cpu->reg_x;
}

static void tya(CPU *cpu) {
	cpu->accum = cpu->reg_y;
	set_zero(cpu, cpu->accum == 0);
	set_negative(cpu, cpu->accum & 0x80);
}

size_t cpu_execute_instruction(CPU *cpu) {
    // TODO: Implement checks to set these flags and return correct number of cpu cycles
    bool pg_cross = false, branch_taken = false, new_page = false;

    const uint8_t opcode = cpu_read_byte(cpu, cpu->pc++);

    switch (opcode) {
        case 0x69:
            adc(cpu, IMMEDIATE);
            return 2;
        case 0x65:
            adc(cpu, ZERO_PAGE);
            return 3;
        case 0x75:
            adc(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0x6d:
            adc(cpu, ABSOLUTE);
            return 4;
        case 0x7d:
            // TODO: implement check for page crossing
            pg_cross = adc(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0x79:
            pg_cross = adc(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0x61:
            adc(cpu, INDIRECT_IDX_X);
            return 6;
        case 0x71:
            pg_cross = adc(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0x29:
            and_(cpu, IMMEDIATE);
            return 2;
        case 0x25:
            and_(cpu, ZERO_PAGE);
            return 3;
        case 0x35:
            and_(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0x2d:
            and_(cpu, ABSOLUTE);
            return 4;
        case 0x3d:
            pg_cross = and_(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0x39:
            pg_cross = and_(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0x21:
            and_(cpu, INDIRECT_IDX_X);
            return 6;
        case 0x31:
            and_(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0xa:
            asl(cpu, ACCUMULATOR);
            return 2;
        case 0x6:
            asl(cpu, ZERO_PAGE);
            return 5;
        case 0x16:
            asl(cpu, ZERO_PAGE_IDX_X);
            return 6;
        case 0xe:
            asl(cpu, ABSOLUTE);
            return 6;
        case 0x1e:
            asl(cpu, ABSOLUTE_IDX_X);
            return 7;
        case 0x90:
            bcc(cpu);
            return 2 + branch_taken + new_page;
        case 0xb0:
            bcs(cpu);
            return 2 + branch_taken + new_page;
        case 0xf0:
            beq(cpu);
            return 2 + branch_taken + new_page;
        case 0x24:
            bit(cpu, ZERO_PAGE);
            return 3;
        case 0x2c:
            bit(cpu, ABSOLUTE);
            return 4;
        case 0x30:
            bmi(cpu);
            return 2 + branch_taken + new_page;
        case 0xd0:
            bne(cpu);
            return 2 + branch_taken + new_page;
        case 0x10:
            bpl(cpu);
            return 2 + branch_taken + new_page;
        case 0x00:
			brk(cpu);
            return 7;
        case 0x50:
            bvc(cpu);
            return 2 + branch_taken + new_page;
        case 0x70:
            bvs(cpu);
            return 2 + branch_taken + new_page;
        case 0x18:
            clc(cpu);
            return 2;
        case 0xd8:
            cld(cpu);
            return 2;
        case 0x58:
            cli(cpu);
            return 2;
        case 0xb8:
            clv(cpu);
            return 2;
        case 0xc9:
            cmp(cpu, IMMEDIATE);
            return 2;
        case 0xc5:
            cmp(cpu, ZERO_PAGE);
            return 3;
        case 0xd5:
            cmp(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0xcd:
            cmp(cpu, ABSOLUTE);
            return 4;
        case 0xdd:
            cmp(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0xd9:
            cmp(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0xc1:
            cmp(cpu, INDIRECT_IDX_X);
            return 6;
        case 0xd1:
            cmp(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0xe0:
            cpx(cpu, IMMEDIATE);
            return 2;
        case 0xe4:
            cpx(cpu, ZERO_PAGE);
            return 3;
        case 0xec:
            cpx(cpu, ABSOLUTE);
            return 4;
        case 0xc0:
            cpy(cpu, IMMEDIATE);
            return 2;
        case 0xc4:
            cpy(cpu, ZERO_PAGE);
            return 3;
        case 0xcc:
            cpy(cpu, ABSOLUTE);
            return 4;
        case 0xc6:
            dec(cpu, ZERO_PAGE);
            return 5;
        case 0xd6:
            dec(cpu, ZERO_PAGE_IDX_X);
            return 6;
        case 0xce:
            dec(cpu, ABSOLUTE);
            return 6;
        case 0xde:
            dec(cpu, ABSOLUTE_IDX_X);
            return 7;
        case 0xca:
            dex(cpu);
            return 2;
        case 0x88:
            dey(cpu);
            return 2;
        case 0x49:
            eor(cpu, IMMEDIATE);
            return 2;
        case 0x45:
            eor(cpu, ZERO_PAGE);
            return 3;
        case 0x55:
            eor(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0x4d:
            eor(cpu, ABSOLUTE);
            return 4;
        case 0x5d:
            eor(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0x59:
            eor(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0x41:
            eor(cpu, INDIRECT_IDX_X);
            return 6;
        case 0x51:
            eor(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0xe6:
            inc(cpu, ZERO_PAGE);
            return 5;
        case 0xf6:
            inc(cpu, ZERO_PAGE_IDX_X);
            return 6;
        case 0xee:
            inc(cpu, ABSOLUTE);
            return 6;
        case 0xfe:
            inc(cpu, ABSOLUTE_IDX_X);
            return 7;
        case 0xe8:
            inx(cpu);
            return 2;
        case 0xc8:
            iny(cpu);
            return 2;
        case 0x4c:
            jmp(cpu, ABSOLUTE);
            return 3;
        case 0x6c:
            jmp(cpu, INDIRECT);
            return 5;
        case 0x20:
            jsr(cpu);
            return 6;
        case 0xa9:
            lda(cpu, IMMEDIATE);
            return 2;
        case 0xa5:
            lda(cpu, ZERO_PAGE);
            return 3;
        case 0xb5:
            lda(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0xad:
            lda(cpu, ABSOLUTE);
            return 4;
        case 0xbd:
            lda(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0xb9:
            lda(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0xa1:
            lda(cpu, INDIRECT_IDX_X);
            return 6;
        case 0xb1:
            lda(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0xa2:
            ldx(cpu, IMMEDIATE);
            return 2;
        case 0xa6:
            ldx(cpu, ZERO_PAGE);
            return 3;
        case 0xb6:
            ldx(cpu, ZERO_PAGE_IDX_Y);
            return 4;
        case 0xae:
            ldx(cpu, ABSOLUTE);
            return 4;
        case 0xbe:
            ldx(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0xa0:
            ldy(cpu, IMMEDIATE);
            return 2;
        case 0xa4:
            ldy(cpu, ZERO_PAGE);
            return 3;
        case 0xb4:
            ldy(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0xac:
            ldy(cpu, ABSOLUTE);
            return 4;
        case 0xbc:
            ldy(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0x4a:
            lsr(cpu, ACCUMULATOR);
            return 2;
        case 0x46:
            lsr(cpu, ZERO_PAGE);
            return 5;
        case 0x56:
            lsr(cpu, ZERO_PAGE_IDX_X);
            return 6;
        case 0x4e:
            lsr(cpu, ABSOLUTE);
            return 6;
        case 0x5e:
            lsr(cpu, ABSOLUTE_IDX_X);
            return 7;
        case 0xea: // NOP
            return 2;
        case 0x9:
            ora(cpu, IMMEDIATE);
            return 2;
        case 0x5:
            ora(cpu, ZERO_PAGE);
            return 3;
        case 0x15:
            ora(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0xd:
            ora(cpu, ABSOLUTE);
            return 4;
        case 0x1d:
            ora(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0x19:
            ora(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0x1:
            ora(cpu, INDIRECT_IDX_X);
            return 6;
        case 0x11:
            ora(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0x48:
            pha(cpu);
            return 3;
        case 0x8:
            php(cpu);
            return 3;
        case 0x68:
            pla(cpu);
            return 4;
        case 0x28:
            plp(cpu);
            return 4;
        case 0x2a:
            rol(cpu, ACCUMULATOR);
            return 2;
        case 0x26:
            rol(cpu, ZERO_PAGE);
            return 5;
        case 0x36:
            rol(cpu, ZERO_PAGE_IDX_X);
            return 6;
        case 0x2e:
            rol(cpu, ABSOLUTE);
            return 6;
        case 0x3e:
            rol(cpu, ABSOLUTE_IDX_X);
            return 7;
        case 0x6a:
            ror(cpu, ACCUMULATOR);
            return 2;
        case 0x66:
            ror(cpu, ZERO_PAGE);
            return 5;
        case 0x76:
            ror(cpu, ZERO_PAGE_IDX_X);
            return 6;
        case 0x6e:
            ror(cpu, ABSOLUTE);
            return 6;
        case 0x7e:
            ror(cpu, ABSOLUTE_IDX_X);
            return 7;
        case 0x40:
            rti(cpu);
            return 6;
        case 0x60:
            rts(cpu);
            return 6;
        case 0xe9:
            sbc(cpu, IMMEDIATE);
            return 2;
        case 0xe5:
            sbc(cpu, ZERO_PAGE);
            return 3;
        case 0xf5:
            sbc(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0xed:
            sbc(cpu, ABSOLUTE);
            return 4;
        case 0xfd:
            sbc(cpu, ABSOLUTE_IDX_X);
            return 4 + pg_cross;
        case 0xf9:
            sbc(cpu, ABSOLUTE_IDX_Y);
            return 4 + pg_cross;
        case 0xe1:
            sbc(cpu, INDIRECT_IDX_X);
            return 6;
        case 0xf1:
            sbc(cpu, INDIRECT_IDX_Y);
            return 5 + pg_cross;
        case 0x38:
            sec(cpu);
            return 2;
        case 0xf8:
            sed(cpu);
            return 2;
        case 0x78:
            sei(cpu);
            return 2;
        case 0x85:
            sta(cpu, ZERO_PAGE);
            return 3;
        case 0x95:
            sta(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0x8d:
            sta(cpu, ABSOLUTE);
            return 4;
        case 0x9d:
            sta(cpu, ABSOLUTE_IDX_X);
            return 5;
        case 0x99:
            sta(cpu, ABSOLUTE_IDX_Y);
            return 5;
        case 0x81:
            sta(cpu, INDIRECT_IDX_X);
            return 6;
        case 0x91:
            sta(cpu, INDIRECT_IDX_Y);
            return 6;
        case 0x86:
            stx(cpu, ZERO_PAGE);
            return 3;
        case 0x96:
            stx(cpu, ZERO_PAGE_IDX_Y);
            return 4;
        case 0x8e:
            stx(cpu, ABSOLUTE);
            return 4;
        case 0x84:
            sty(cpu, ZERO_PAGE);
            return 3;
        case 0x94:
            sty(cpu, ZERO_PAGE_IDX_X);
            return 4;
        case 0x8c:
            sty(cpu, ABSOLUTE);
            return 4;
        case 0xaa:
            tax(cpu);
            return 2;
        case 0xa8:
            tay(cpu);
            return 2;
        case 0xba:
            tsx(cpu);
            return 2;
        case 0x8a:
            txa(cpu);
            return 2;
        case 0x9a:
            txs(cpu);
            return 2;
        case 0x98:
            tya(cpu);
            return 2;
        default:
            printf("Unimplemented opcode %hhu!\n", opcode);
            exit(0);
    }
}
