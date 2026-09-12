#include "cpu.h"
#include "nes.h"

#define STACK_BASE 0x100

enum class AddressingMode {
    accumulator,
    immediate,
    zero_page,
    zero_page_idx_x,
    zero_page_idx_y,
    absolute,
    absolute_idx_x,
    absolute_idx_y,
    indirect,
    indirect_idx_x,
    indirect_idx_y,
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

uint8_t cpu_read_byte(CPU *cpu, uint16_t addr) {
    if (cpu->standalone)
        return cpu->ram[addr];

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
    if (cpu->standalone) {
        cpu->ram[addr] = val;
        return;
    }

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
		case AddressingMode::immediate:
			return cpu->pc++;
		case AddressingMode::indirect:
		case AddressingMode::absolute:
			tmp_u16 = cpu_read_two_bytes(cpu, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case AddressingMode::absolute_idx_x:
			tmp_u16 = cpu->reg_x + cpu_read_two_bytes(cpu, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case AddressingMode::absolute_idx_y:
			tmp_u16 = cpu->reg_y + cpu_read_two_bytes(cpu, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case AddressingMode::zero_page:
			return cpu_read_byte(cpu, cpu->pc++);
		case AddressingMode::zero_page_idx_x:
			tmp_u8 = cpu->reg_x + cpu_read_byte(cpu, cpu->pc++);
			return tmp_u8;
		case AddressingMode::zero_page_idx_y:
			tmp_u8 = cpu->reg_y + cpu_read_byte(cpu, cpu->pc++);
			return tmp_u8;
		case AddressingMode::indirect_idx_x:
			tmp_u8 = cpu->reg_x + cpu_read_byte(cpu, cpu->pc++);
			tmp_u16 = cpu_read_byte(cpu, tmp_u8++);
			tmp_u16 += cpu_read_byte(cpu, tmp_u8) << 8;
			return tmp_u16;
		case AddressingMode::indirect_idx_y:
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
	uint16_t result = cpu->accum + operand + cpu->get_carry();
	cpu->set_carry(result > 0xff);
	result = (uint8_t) result;
	cpu->set_overflow((result ^ cpu->accum) & (result ^ operand) & 0x80);
	cpu->accum = result;
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
    return false;
}

bool and_(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum &= cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
    return false;
}

void asl(CPU *cpu, AddressingMode addr_mode){
	if (addr_mode == AddressingMode::accumulator) {
		cpu->set_carry(cpu->accum & 0x80);
		cpu->accum = cpu->accum << 1;
		cpu->set_zero(cpu->accum == 0);
		cpu->set_negative(cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t operand = cpu_read_byte(cpu, addr);
    cpu->set_carry(operand & 0x80);
    uint8_t tmp = operand << 1;
	cpu_write_byte(cpu, addr, tmp);
    cpu->set_zero(tmp == 0);
    cpu->set_negative(tmp & 0x80);
}

void bcc(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if (!cpu->get_carry())
		cpu->pc += offset;
}

void bcs(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if (cpu->get_carry())
		cpu->pc += offset;
}

void beq(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(cpu->get_zero())
		cpu->pc += offset;
}

void bit(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	uint8_t tmp = cpu->accum & operand;
	cpu->set_zero(tmp == 0);
	cpu->set_overflow(operand & 0x40);
	cpu->set_negative(operand & 0x80);
}

void bmi(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(cpu->get_negative())
		cpu->pc += offset;
}

void bne(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(!cpu->get_zero())
		cpu->pc += offset;
}

void bpl(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(!cpu->get_negative())
		cpu->pc += offset;
}

void brk(CPU *cpu) {
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, ++cpu->pc >> 8);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->pc & 0xff);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->flags | 0x30); // break flag and extra bit (bits 4 & 5) should always be set: 0x30 = 00110000
	cpu->set_disable_interrupt(1);
	cpu->pc = cpu_read_two_bytes(cpu, 0xfffe); // address of irq interrupt handler
}

void bvc(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(!cpu->get_overflow())
		cpu->pc += offset;
}

void bvs(CPU *cpu) {
	auto offset = (int8_t) cpu_read_byte(cpu, cpu->pc++);
	if(cpu->get_overflow())
		cpu->pc += offset;
}

void clc(CPU *cpu) {
	cpu->set_carry(0);
}

void cld(CPU *cpu) {
	cpu->flags &= ~0x8;
}

void cli(CPU *cpu) {
	cpu->set_disable_interrupt(0);
}

void clv(CPU *cpu) {
	cpu->set_overflow(0);
}

void cmp(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_carry(cpu->accum >= operand);
	auto result = (int8_t) (cpu->accum - operand);
	cpu->set_zero(result == 0);
	cpu->set_negative(result < 0);
}

void cpx(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_carry(cpu->reg_x >= operand);
	auto result = (int8_t) (cpu->reg_x - operand);
	cpu->set_zero(result == 0);
	cpu->set_negative(result < 0);
}

void cpy(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_carry(cpu->reg_y >= operand);
	auto result = (int8_t) (cpu->reg_y - operand);
	cpu->set_zero(result == 0);
	cpu->set_negative(result < 0);
}

void dec(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr) - 1;
	cpu->set_zero(tmp == 0);
	cpu->set_negative(tmp & 0x80);
	cpu_write_byte(cpu, addr, tmp);
}

void dex(CPU *cpu) {
	cpu->set_zero(--cpu->reg_x == 0);
	cpu->set_negative(cpu->reg_x & 0x80);
}

void dey(CPU *cpu) {
	cpu->set_zero(--cpu->reg_y == 0);
	cpu->set_negative(cpu->reg_y & 0x80);
}

void eor(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum ^= cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void inc(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr) + 1;
	cpu->set_zero(tmp == 0);
	cpu->set_negative(tmp & 0x80);
	cpu_write_byte(cpu, addr, tmp);
}

void inx(CPU *cpu) {
	cpu->set_zero(++cpu->reg_x == 0);
	cpu->set_negative(cpu->reg_x & 0x80);
}

void iny(CPU *cpu) {
	cpu->set_zero(++cpu->reg_y == 0);
	cpu->set_negative(cpu->reg_y & 0x80);
}

void jmp(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
    if (addr_mode == AddressingMode::indirect && (addr & 0xff) == 0xff) {
        /* CPU quirk in nes version of 6502
         * See https://www.nesdev.org/obelisk-6502-guide/reference.html#JMP
         */
        cpu->pc = (((uint16_t) cpu_read_byte(cpu, addr & 0xff00)) << 8) | cpu_read_byte(cpu, addr);
        return;
    }
	cpu->pc = addr_mode == AddressingMode::indirect ? cpu_read_two_bytes(cpu, addr) : addr;
}

void jsr(CPU *cpu) {
	uint16_t addr = cpu_read_two_bytes(cpu, cpu->pc++);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->pc >> 8);
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->pc & 0xff);
	cpu->pc = addr;
}

void lda(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void ldx(CPU *cpu, AddressingMode addr_mode) {
	cpu->reg_x = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->reg_x == 0);
	cpu->set_negative(cpu->reg_x & 0x80);
}

void ldy(CPU *cpu, AddressingMode addr_mode) {
	cpu->reg_y = cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->reg_y == 0);
	cpu->set_negative(cpu->reg_y & 0x80);
}

void lsr(CPU *cpu, AddressingMode addr_mode) {
	if (addr_mode == AddressingMode::accumulator) {
		cpu->set_carry(cpu->accum & 0x1);
		cpu->accum = cpu->accum >> 1;
		cpu->set_zero(cpu->accum == 0);
		cpu->set_negative(cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr);
	cpu->set_carry(tmp & 0x1);
	tmp = tmp >> 1;
	cpu->set_zero(tmp == 0);
	cpu->set_negative(tmp & 0x80);
	cpu_write_byte(cpu, addr, tmp);
}

void ora(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum |= cpu_read_byte(cpu, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void pha(CPU *cpu) {
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->accum);
}

void php(CPU *cpu) {
	cpu_write_byte(cpu, STACK_BASE + cpu->sp--, cpu->flags | 0x30);
}

void pla(CPU *cpu) {
	cpu->accum = cpu_read_byte(cpu, STACK_BASE + ++cpu->sp);
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void plp(CPU *cpu) {
	// ignore break flag (bit 4): 0xef = 11101111 and set extra bit (bit 5): 0x20 = 00100000
	cpu->flags = 0x20 | (cpu_read_byte(cpu, STACK_BASE + ++cpu->sp) & 0xef);
}

void rol(CPU *cpu, AddressingMode addr_mode) {
	if (addr_mode == AddressingMode::accumulator) {
		uint8_t tmp = (cpu->accum << 1) | cpu->get_carry();
		cpu->set_carry(cpu->accum & 0x80);
		cpu->accum = tmp;
		cpu->set_zero(cpu->accum == 0);
		cpu->set_negative(cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr);
	uint8_t tmp_ = (tmp << 1) | cpu->get_carry();
	cpu->set_carry(tmp & 0x80);
	cpu->set_zero(tmp_ == 0);
	cpu->set_negative(tmp_ & 0x80);
	cpu_write_byte(cpu, addr, tmp_);
}

void ror(CPU *cpu, AddressingMode addr_mode) {
	if (addr_mode == AddressingMode::accumulator) {
		uint8_t tmp = (cpu->accum >> 1) | (((uint8_t) cpu->get_carry()) << 7);
		cpu->set_carry(cpu->accum & 0x1);
		cpu->accum = tmp;
		cpu->set_zero(cpu->accum == 0);
		cpu->set_negative(cpu->accum & 0x80);
		return;
	}
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = cpu_read_byte(cpu, addr);
	uint8_t tmp_ = (tmp >> 1) | (((uint8_t) cpu->get_carry()) << 7);
	cpu->set_carry(tmp & 0x1);
	cpu->set_zero(tmp_ == 0);
	cpu->set_negative(tmp_ & 0x80);
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
	uint16_t result = cpu->accum + operand + cpu->get_carry();
	cpu->set_carry(result > 0xff);
	result = (uint8_t) result;
	cpu->set_overflow((result ^ cpu->accum) & (result ^ operand) & 0x80);
	cpu->accum = result;
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void sec(CPU *cpu) {
	cpu->set_carry(1);
}

void sed(CPU *cpu) {
	cpu->set_decimal(1);
}

void sei(CPU *cpu) {
	cpu->set_disable_interrupt(1);
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
	cpu->set_zero(cpu->reg_x == 0);
	cpu->set_negative(cpu->reg_x & 0x80);
}

void tay(CPU *cpu) {
	cpu->reg_y = cpu->accum;
	cpu->set_zero(cpu->reg_y == 0);
	cpu->set_negative(cpu->reg_y & 0x80);
}

void tsx(CPU *cpu) {
	cpu->reg_x = cpu->sp;
	cpu->set_zero(cpu->reg_x == 0);
	cpu->set_negative(cpu->reg_x & 0x80);
}

void txa(CPU *cpu) {
	cpu->accum = cpu->reg_x;
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void txs(CPU *cpu) {
	cpu->sp = cpu->reg_x;
}

void tya(CPU *cpu) {
	cpu->accum = cpu->reg_y;
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

size_t cpu_execute_instruction(CPU *cpu) {
    // TODO: Implement checks to set these flags and return correct number of cpu cycles
    bool pg_cross = false, branch_taken = false, new_page = false;

    const uint8_t opcode = cpu_read_byte(cpu, cpu->pc++);

    switch (opcode) {
        case 0x69:
            adc(cpu, AddressingMode::immediate);
            return 2;
        case 0x65:
            adc(cpu, AddressingMode::zero_page);
            return 3;
        case 0x75:
            adc(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x6d:
            adc(cpu, AddressingMode::absolute);
            return 4;
        case 0x7d:
            // TODO: implement check for page crossing
            pg_cross = adc(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x79:
            pg_cross = adc(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x61:
            adc(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0x71:
            pg_cross = adc(cpu, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x29:
            and_(cpu, AddressingMode::immediate);
            return 2;
        case 0x25:
            and_(cpu, AddressingMode::zero_page);
            return 3;
        case 0x35:
            and_(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x2d:
            and_(cpu, AddressingMode::absolute);
            return 4;
        case 0x3d:
            pg_cross = and_(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x39:
            pg_cross = and_(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x21:
            and_(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0x31:
            and_(cpu, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xa:
            asl(cpu, AddressingMode::accumulator);
            return 2;
        case 0x6:
            asl(cpu, AddressingMode::zero_page);
            return 5;
        case 0x16:
            asl(cpu, AddressingMode::zero_page_idx_x);
            return 6;
        case 0xe:
            asl(cpu, AddressingMode::absolute);
            return 6;
        case 0x1e:
            asl(cpu, AddressingMode::absolute_idx_x);
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
            bit(cpu, AddressingMode::zero_page);
            return 3;
        case 0x2c:
            bit(cpu, AddressingMode::absolute);
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
            cmp(cpu, AddressingMode::immediate);
            return 2;
        case 0xc5:
            cmp(cpu, AddressingMode::zero_page);
            return 3;
        case 0xd5:
            cmp(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xcd:
            cmp(cpu, AddressingMode::absolute);
            return 4;
        case 0xdd:
            cmp(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xd9:
            cmp(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xc1:
            cmp(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0xd1:
            cmp(cpu, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xe0:
            cpx(cpu, AddressingMode::immediate);
            return 2;
        case 0xe4:
            cpx(cpu, AddressingMode::zero_page);
            return 3;
        case 0xec:
            cpx(cpu, AddressingMode::absolute);
            return 4;
        case 0xc0:
            cpy(cpu, AddressingMode::immediate);
            return 2;
        case 0xc4:
            cpy(cpu, AddressingMode::zero_page);
            return 3;
        case 0xcc:
            cpy(cpu, AddressingMode::absolute);
            return 4;
        case 0xc6:
            dec(cpu, AddressingMode::zero_page);
            return 5;
        case 0xd6:
            dec(cpu, AddressingMode::zero_page_idx_x);
            return 6;
        case 0xce:
            dec(cpu, AddressingMode::absolute);
            return 6;
        case 0xde:
            dec(cpu, AddressingMode::absolute_idx_x);
            return 7;
        case 0xca:
            dex(cpu);
            return 2;
        case 0x88:
            dey(cpu);
            return 2;
        case 0x49:
            eor(cpu, AddressingMode::immediate);
            return 2;
        case 0x45:
            eor(cpu, AddressingMode::zero_page);
            return 3;
        case 0x55:
            eor(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x4d:
            eor(cpu, AddressingMode::absolute);
            return 4;
        case 0x5d:
            eor(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x59:
            eor(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x41:
            eor(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0x51:
            eor(cpu, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xe6:
            inc(cpu, AddressingMode::zero_page);
            return 5;
        case 0xf6:
            inc(cpu, AddressingMode::zero_page_idx_x);
            return 6;
        case 0xee:
            inc(cpu, AddressingMode::absolute);
            return 6;
        case 0xfe:
            inc(cpu, AddressingMode::absolute_idx_x);
            return 7;
        case 0xe8:
            inx(cpu);
            return 2;
        case 0xc8:
            iny(cpu);
            return 2;
        case 0x4c:
            jmp(cpu, AddressingMode::absolute);
            return 3;
        case 0x6c:
            jmp(cpu, AddressingMode::indirect);
            return 5;
        case 0x20:
            jsr(cpu);
            return 6;
        case 0xa9:
            lda(cpu, AddressingMode::immediate);
            return 2;
        case 0xa5:
            lda(cpu, AddressingMode::zero_page);
            return 3;
        case 0xb5:
            lda(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xad:
            lda(cpu, AddressingMode::absolute);
            return 4;
        case 0xbd:
            lda(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xb9:
            lda(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xa1:
            lda(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0xb1:
            lda(cpu, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xa2:
            ldx(cpu, AddressingMode::immediate);
            return 2;
        case 0xa6:
            ldx(cpu, AddressingMode::zero_page);
            return 3;
        case 0xb6:
            ldx(cpu, AddressingMode::zero_page_idx_y);
            return 4;
        case 0xae:
            ldx(cpu, AddressingMode::absolute);
            return 4;
        case 0xbe:
            ldx(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xa0:
            ldy(cpu, AddressingMode::immediate);
            return 2;
        case 0xa4:
            ldy(cpu, AddressingMode::zero_page);
            return 3;
        case 0xb4:
            ldy(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xac:
            ldy(cpu, AddressingMode::absolute);
            return 4;
        case 0xbc:
            ldy(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x4a:
            lsr(cpu, AddressingMode::accumulator);
            return 2;
        case 0x46:
            lsr(cpu, AddressingMode::zero_page);
            return 5;
        case 0x56:
            lsr(cpu, AddressingMode::zero_page_idx_x);
            return 6;
        case 0x4e:
            lsr(cpu, AddressingMode::absolute);
            return 6;
        case 0x5e:
            lsr(cpu, AddressingMode::absolute_idx_x);
            return 7;
        case 0xea: // NOP
            return 2;
        case 0x9:
            ora(cpu, AddressingMode::immediate);
            return 2;
        case 0x5:
            ora(cpu, AddressingMode::zero_page);
            return 3;
        case 0x15:
            ora(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xd:
            ora(cpu, AddressingMode::absolute);
            return 4;
        case 0x1d:
            ora(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x19:
            ora(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x1:
            ora(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0x11:
            ora(cpu, AddressingMode::indirect_idx_y);
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
            rol(cpu, AddressingMode::accumulator);
            return 2;
        case 0x26:
            rol(cpu, AddressingMode::zero_page);
            return 5;
        case 0x36:
            rol(cpu, AddressingMode::zero_page_idx_x);
            return 6;
        case 0x2e:
            rol(cpu, AddressingMode::absolute);
            return 6;
        case 0x3e:
            rol(cpu, AddressingMode::absolute_idx_x);
            return 7;
        case 0x6a:
            ror(cpu, AddressingMode::accumulator);
            return 2;
        case 0x66:
            ror(cpu, AddressingMode::zero_page);
            return 5;
        case 0x76:
            ror(cpu, AddressingMode::zero_page_idx_x);
            return 6;
        case 0x6e:
            ror(cpu, AddressingMode::absolute);
            return 6;
        case 0x7e:
            ror(cpu, AddressingMode::absolute_idx_x);
            return 7;
        case 0x40:
            rti(cpu);
            return 6;
        case 0x60:
            rts(cpu);
            return 6;
        case 0xe9:
            sbc(cpu, AddressingMode::immediate);
            return 2;
        case 0xe5:
            sbc(cpu, AddressingMode::zero_page);
            return 3;
        case 0xf5:
            sbc(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xed:
            sbc(cpu, AddressingMode::absolute);
            return 4;
        case 0xfd:
            sbc(cpu, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xf9:
            sbc(cpu, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xe1:
            sbc(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0xf1:
            sbc(cpu, AddressingMode::indirect_idx_y);
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
            sta(cpu, AddressingMode::zero_page);
            return 3;
        case 0x95:
            sta(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x8d:
            sta(cpu, AddressingMode::absolute);
            return 4;
        case 0x9d:
            sta(cpu, AddressingMode::absolute_idx_x);
            return 5;
        case 0x99:
            sta(cpu, AddressingMode::absolute_idx_y);
            return 5;
        case 0x81:
            sta(cpu, AddressingMode::indirect_idx_x);
            return 6;
        case 0x91:
            sta(cpu, AddressingMode::indirect_idx_y);
            return 6;
        case 0x86:
            stx(cpu, AddressingMode::zero_page);
            return 3;
        case 0x96:
            stx(cpu, AddressingMode::zero_page_idx_y);
            return 4;
        case 0x8e:
            stx(cpu, AddressingMode::absolute);
            return 4;
        case 0x84:
            sty(cpu, AddressingMode::zero_page);
            return 3;
        case 0x94:
            sty(cpu, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x8c:
            sty(cpu, AddressingMode::absolute);
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