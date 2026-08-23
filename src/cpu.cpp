#include "cpu.h"

// TODO: implement cycle penalties for page crosses and taken branches

void CPU::handle_nmi() {
    bus->nmi = false;
    ram_write_byte(bus, stack_base + sp--, sr | 0x20);
    ram_write_two_bytes(bus, stack_base + sp, pc);
    sp -= 2; 
    pc = ram_read_two_bytes(bus, 0xfffa);
}

void cpu_init(CPU *cpu, Bus *bus)
{
    cpu->bus = bus;
    cpu_reset(cpu);
}

void cpu_reset(CPU *cpu) {
    cpu->sp = 0xff;
    cpu->accum = 0;
    cpu->reg_x = 0;
    cpu->reg_y = 0;
    cpu->sr = 0;
    cpu->pc = ram_read_two_bytes(cpu->bus, 0xfffc);
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
			tmp_u16 = ram_read_two_bytes(cpu->bus, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case AddressingMode::absolute_idx_x:
			tmp_u16 = cpu->reg_x + ram_read_two_bytes(cpu->bus, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case AddressingMode::absolute_idx_y:
			tmp_u16 = cpu->reg_y + ram_read_two_bytes(cpu->bus, cpu->pc);
			cpu->pc += 2;
			return tmp_u16;
		case AddressingMode::zero_page:
			return ram_read_byte(cpu->bus, cpu->pc++);
		case AddressingMode::zero_page_idx_x:
			tmp_u8 = cpu->reg_x + ram_read_byte(cpu->bus, cpu->pc++);
			return tmp_u8;
		case AddressingMode::zero_page_idx_y:
			tmp_u8 = cpu->reg_y + ram_read_byte(cpu->bus, cpu->pc++);
			return tmp_u8;
		case AddressingMode::indirect_idx_x:
			tmp_u8 = cpu->reg_x + ram_read_byte(cpu->bus, cpu->pc++);
			tmp_u16 = ram_read_byte(cpu->bus, tmp_u8++);
			tmp_u16 += ram_read_byte(cpu->bus, tmp_u8) << 8;
			return tmp_u16;
		case AddressingMode::indirect_idx_y:
			tmp_u8 = ram_read_byte(cpu->bus, cpu->pc++);
			tmp_u16 = ram_read_byte(cpu->bus, tmp_u8++);
			tmp_u16 += (ram_read_byte(cpu->bus, tmp_u8) << 8) + cpu->reg_y;
			return tmp_u16;
        default:
            return 0;
	}
}

bool adc(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
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
	cpu->accum &= ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
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
	uint8_t operand = ram_read_byte(cpu->bus, addr);
    cpu->set_carry(operand & 0x80);
    uint8_t tmp = operand << 1;
	ram_write_byte(cpu->bus, addr, tmp);
    cpu->set_zero(tmp == 0);
    cpu->set_negative(tmp & 0x80);
}

void bcc(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if (!cpu->get_carry())
		cpu->pc += offset;
}

void bcs(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if (cpu->get_carry())
		cpu->pc += offset;
}

void beq(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if(cpu->get_zero())
		cpu->pc += offset;
}

void bit(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	uint8_t tmp = cpu->accum & operand;
	cpu->set_zero(tmp == 0);
	cpu->set_overflow(operand & 0x40);
	cpu->set_negative(operand & 0x80);
}

void bmi(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if(cpu->get_negative())
		cpu->pc += offset;
}

void bne(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if(!cpu->get_zero())
		cpu->pc += offset;
}

void bpl(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if(!cpu->get_negative())
		cpu->pc += offset;
}

void brk(CPU *cpu) {
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, ++cpu->pc >> 8);
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, cpu->pc & 0xff);
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, cpu->sr | 0x30); // break flag and extra bit (bits 4 & 5) should always be set: 0x30 = 00110000
	cpu->set_disable_interrupt(1);
	cpu->pc = ram_read_two_bytes(cpu->bus, 0xfffe); // address of irq interrupt handler
}

void bvc(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if(!cpu->get_overflow())
		cpu->pc += offset;
}

void bvs(CPU *cpu) {
	auto offset = (int8_t) ram_read_byte(cpu->bus, cpu->pc++);
	if(cpu->get_overflow())
		cpu->pc += offset;
}

void clc(CPU *cpu) {
	cpu->set_carry(0);
}

void cld(CPU *cpu) {
	cpu->sr &= ~0x8;
}

void cli(CPU *cpu) {
	cpu->set_disable_interrupt(0);
}

void clv(CPU *cpu) {
	cpu->set_overflow(0);
}

void cmp(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_carry(cpu->accum >= operand);
	auto result = (int8_t) (cpu->accum - operand);
	cpu->set_zero(result == 0);
	cpu->set_negative(result < 0);
}

void cpx(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_carry(cpu->reg_x >= operand);
	auto result = (int8_t) (cpu->reg_x - operand);
	cpu->set_zero(result == 0);
	cpu->set_negative(result < 0);
}

void cpy(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_carry(cpu->reg_y >= operand);
	auto result = (int8_t) (cpu->reg_y - operand);
	cpu->set_zero(result == 0);
	cpu->set_negative(result < 0);
}

void dec(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = ram_read_byte(cpu->bus, addr) - 1;
	cpu->set_zero(tmp == 0);
	cpu->set_negative(tmp & 0x80);
	ram_write_byte(cpu->bus, addr, tmp);
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
	cpu->accum ^= ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void inc(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	uint8_t tmp = ram_read_byte(cpu->bus, addr) + 1;
	cpu->set_zero(tmp == 0);
	cpu->set_negative(tmp & 0x80);
	ram_write_byte(cpu->bus, addr, tmp);
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
        cpu->pc = (((uint16_t) ram_read_byte(cpu->bus, addr & 0xff00)) << 8) | ram_read_byte(cpu->bus, addr);
        return;
    }
	cpu->pc = addr_mode == AddressingMode::indirect ? ram_read_two_bytes(cpu->bus, addr) : addr;
}

void jsr(CPU *cpu) {
	uint16_t addr = ram_read_two_bytes(cpu->bus, cpu->pc++);
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, cpu->pc >> 8);
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, cpu->pc & 0xff);
	cpu->pc = addr;
}

void lda(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void ldx(CPU *cpu, AddressingMode addr_mode) {
	cpu->reg_x = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->reg_x == 0);
	cpu->set_negative(cpu->reg_x & 0x80);
}

void ldy(CPU *cpu, AddressingMode addr_mode) {
	cpu->reg_y = ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
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
	uint8_t tmp = ram_read_byte(cpu->bus, addr);
	cpu->set_carry(tmp & 0x1);
	tmp = tmp >> 1;
	cpu->set_zero(tmp == 0);
	cpu->set_negative(tmp & 0x80);
	ram_write_byte(cpu->bus, addr, tmp);
}

void ora(CPU *cpu, AddressingMode addr_mode) {
	cpu->accum |= ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void pha(CPU *cpu) {
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, cpu->accum);
}

void php(CPU *cpu) {
	ram_write_byte(cpu->bus, cpu->stack_base + cpu->sp--, cpu->sr | 0x30);
}

void pla(CPU *cpu) {
	cpu->accum = ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp);
	cpu->set_zero(cpu->accum == 0);
	cpu->set_negative(cpu->accum & 0x80);
}

void plp(CPU *cpu) {
	// ignore break flag (bit 4): 0xef = 11101111 and set extra bit (bit 5): 0x20 = 00100000
	cpu->sr = 0x20 | (ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp) & 0xef);
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
	uint8_t tmp = ram_read_byte(cpu->bus, addr);
	uint8_t tmp_ = (tmp << 1) | cpu->get_carry();
	cpu->set_carry(tmp & 0x80);
	cpu->set_zero(tmp_ == 0);
	cpu->set_negative(tmp_ & 0x80);
	ram_write_byte(cpu->bus, addr, tmp_);
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
	uint8_t tmp = ram_read_byte(cpu->bus, addr);
	uint8_t tmp_ = (tmp >> 1) | (((uint8_t) cpu->get_carry()) << 7);
	cpu->set_carry(tmp & 0x1);
	cpu->set_zero(tmp_ == 0);
	cpu->set_negative(tmp_ & 0x80);
	ram_write_byte(cpu->bus, addr, tmp_);
}

void rti(CPU *cpu) {
	cpu->sr = 0x20 | (ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp) & 0xef);
    uint8_t pcl = ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp);
    uint8_t pch = ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp);
	cpu->pc = (((uint16_t) pch) << 8) | pcl;
}

void rts(CPU *cpu) {
    uint8_t pcl = ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp);
    uint8_t pch = ram_read_byte(cpu->bus, cpu->stack_base + ++cpu->sp);
    cpu->pc = ((((uint16_t) pch) << 8) | pcl) + 1;
}

void sbc(CPU *cpu, AddressingMode addr_mode) {
	uint8_t operand = ~ram_read_byte(cpu->bus, get_addr(cpu, addr_mode));
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
	ram_write_byte(cpu->bus, addr, cpu->accum);
}

void stx(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	ram_write_byte(cpu->bus, addr, cpu->reg_x);
}

void sty(CPU *cpu, AddressingMode addr_mode) {
	uint16_t addr = get_addr(cpu, addr_mode);
	ram_write_byte(cpu->bus, addr, cpu->reg_y);
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

size_t CPU::execute_instr() {
    // TODO: Implement checks to set these flags and return correct number of cpu cycles
    bool pg_cross = false, branch_taken = false, new_page = false;
    switch (uint8_t opcode = ram_read_byte(bus, pc++); opcode) {
        case 0x69:
            adc(this, AddressingMode::immediate);
            return 2;
        case 0x65:
            adc(this, AddressingMode::zero_page);
            return 3;
        case 0x75:
            adc(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x6d:
            adc(this, AddressingMode::absolute);
            return 4;
        case 0x7d:
            // TODO: implement check for page crossing
            pg_cross = adc(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x79:
            pg_cross = adc(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x61:
            adc(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0x71:
            pg_cross = adc(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x29:
            and_(this, AddressingMode::immediate);
            return 2;
        case 0x25:
            and_(this, AddressingMode::zero_page);
            return 3;
        case 0x35:
            and_(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x2d:
            and_(this, AddressingMode::absolute);
            return 4;
        case 0x3d:
            pg_cross = and_(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x39:
            pg_cross = and_(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x21:
            and_(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0x31:
            and_(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xa:
            asl(this, AddressingMode::accumulator);
            return 2;
        case 0x6:
            asl(this, AddressingMode::zero_page);
            return 5;
        case 0x16:
            asl(this, AddressingMode::zero_page_idx_x);
            return 6;
        case 0xe:
            asl(this, AddressingMode::absolute);
            return 6;
        case 0x1e:
            asl(this, AddressingMode::absolute_idx_x);
            return 7;
        case 0x90:
            bcc(this);
            return 2 + branch_taken + new_page;
        case 0xb0:
            bcs(this);
            return 2 + branch_taken + new_page;
        case 0xf0:
            beq(this);
            return 2 + branch_taken + new_page;
        case 0x24:
            bit(this, AddressingMode::zero_page);
            return 3;
        case 0x2c:
            bit(this, AddressingMode::absolute);
            return 4;
        case 0x30:
            bmi(this);
            return 2 + branch_taken + new_page;
        case 0xd0:
            bne(this);
            return 2 + branch_taken + new_page;
        case 0x10:
            bpl(this);
            return 2 + branch_taken + new_page;
        case 0x00:
			brk(this);
            return 7;
        case 0x50:
            bvc(this);
            return 2 + branch_taken + new_page;
        case 0x70:
            bvs(this);
            return 2 + branch_taken + new_page;
        case 0x18:
            clc(this);
            return 2;
        case 0xd8:
            cld(this);
            return 2;
        case 0x58:
            cli(this);
            return 2;
        case 0xb8:
            clv(this);
            return 2;
        case 0xc9:
            cmp(this, AddressingMode::immediate);
            return 2;
        case 0xc5:
            cmp(this, AddressingMode::zero_page);
            return 3;
        case 0xd5:
            cmp(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xcd:
            cmp(this, AddressingMode::absolute);
            return 4;
        case 0xdd:
            cmp(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xd9:
            cmp(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xc1:
            cmp(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0xd1:
            cmp(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xe0:
            cpx(this, AddressingMode::immediate);
            return 2;
        case 0xe4:
            cpx(this, AddressingMode::zero_page);
            return 3;
        case 0xec:
            cpx(this, AddressingMode::absolute);
            return 4;
        case 0xc0:
            cpy(this, AddressingMode::immediate);
            return 2;
        case 0xc4:
            cpy(this, AddressingMode::zero_page);
            return 3;
        case 0xcc:
            cpy(this, AddressingMode::absolute);
            return 4;
        case 0xc6:
            dec(this, AddressingMode::zero_page);
            return 5;
        case 0xd6:
            dec(this, AddressingMode::zero_page_idx_x);
            return 6;
        case 0xce:
            dec(this, AddressingMode::absolute);
            return 6;
        case 0xde:
            dec(this, AddressingMode::absolute_idx_x);
            return 7;
        case 0xca:
            dex(this);
            return 2;
        case 0x88:
            dey(this);
            return 2;
        case 0x49:
            eor(this, AddressingMode::immediate);
            return 2;
        case 0x45:
            eor(this, AddressingMode::zero_page);
            return 3;
        case 0x55:
            eor(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x4d:
            eor(this, AddressingMode::absolute);
            return 4;
        case 0x5d:
            eor(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x59:
            eor(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x41:
            eor(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0x51:
            eor(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xe6:
            inc(this, AddressingMode::zero_page);
            return 5;
        case 0xf6:
            inc(this, AddressingMode::zero_page_idx_x);
            return 6;
        case 0xee:
            inc(this, AddressingMode::absolute);
            return 6;
        case 0xfe:
            inc(this, AddressingMode::absolute_idx_x);
            return 7;
        case 0xe8:
            inx(this);
            return 2;
        case 0xc8:
            iny(this);
            return 2;
        case 0x4c:
            jmp(this, AddressingMode::absolute);
            return 3;
        case 0x6c:
            jmp(this, AddressingMode::indirect);
            return 5;
        case 0x20:
            jsr(this);
            return 6;
        case 0xa9:
            lda(this, AddressingMode::immediate);
            return 2;
        case 0xa5:
            lda(this, AddressingMode::zero_page);
            return 3;
        case 0xb5:
            lda(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xad:
            lda(this, AddressingMode::absolute);
            return 4;
        case 0xbd:
            lda(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xb9:
            lda(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xa1:
            lda(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0xb1:
            lda(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xa2:
            ldx(this, AddressingMode::immediate);
            return 2;
        case 0xa6:
            ldx(this, AddressingMode::zero_page);
            return 3;
        case 0xb6:
            ldx(this, AddressingMode::zero_page_idx_y);
            return 4;
        case 0xae:
            ldx(this, AddressingMode::absolute);
            return 4;
        case 0xbe:
            ldx(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xa0:
            ldy(this, AddressingMode::immediate);
            return 2;
        case 0xa4:
            ldy(this, AddressingMode::zero_page);
            return 3;
        case 0xb4:
            ldy(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xac:
            ldy(this, AddressingMode::absolute);
            return 4;
        case 0xbc:
            ldy(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x4a:
            lsr(this, AddressingMode::accumulator);
            return 2;
        case 0x46:
            lsr(this, AddressingMode::zero_page);
            return 5;
        case 0x56:
            lsr(this, AddressingMode::zero_page_idx_x);
            return 6;
        case 0x4e:
            lsr(this, AddressingMode::absolute);
            return 6;
        case 0x5e:
            lsr(this, AddressingMode::absolute_idx_x);
            return 7;
        case 0xea: // NOP
            return 2;
        case 0x9:
            ora(this, AddressingMode::immediate);
            return 2;
        case 0x5:
            ora(this, AddressingMode::zero_page);
            return 3;
        case 0x15:
            ora(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xd:
            ora(this, AddressingMode::absolute);
            return 4;
        case 0x1d:
            ora(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x19:
            ora(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x1:
            ora(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0x11:
            ora(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x48:
            pha(this);
            return 3;
        case 0x8:
            php(this);
            return 3;
        case 0x68:
            pla(this);
            return 4;
        case 0x28:
            plp(this);
            return 4;
        case 0x2a:
            rol(this, AddressingMode::accumulator);
            return 2;
        case 0x26:
            rol(this, AddressingMode::zero_page);
            return 5;
        case 0x36:
            rol(this, AddressingMode::zero_page_idx_x);
            return 6;
        case 0x2e:
            rol(this, AddressingMode::absolute);
            return 6;
        case 0x3e:
            rol(this, AddressingMode::absolute_idx_x);
            return 7;
        case 0x6a:
            ror(this, AddressingMode::accumulator);
            return 2;
        case 0x66:
            ror(this, AddressingMode::zero_page);
            return 5;
        case 0x76:
            ror(this, AddressingMode::zero_page_idx_x);
            return 6;
        case 0x6e:
            ror(this, AddressingMode::absolute);
            return 6;
        case 0x7e:
            ror(this, AddressingMode::absolute_idx_x);
            return 7;
        case 0x40:
            rti(this);
            return 6;
        case 0x60:
            rts(this);
            return 6;
        case 0xe9:
            sbc(this, AddressingMode::immediate);
            return 2;
        case 0xe5:
            sbc(this, AddressingMode::zero_page);
            return 3;
        case 0xf5:
            sbc(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0xed:
            sbc(this, AddressingMode::absolute);
            return 4;
        case 0xfd:
            sbc(this, AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xf9:
            sbc(this, AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xe1:
            sbc(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0xf1:
            sbc(this, AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x38:
            sec(this);
            return 2;
        case 0xf8:
            sed(this);
            return 2;
        case 0x78:
            sei(this);
            return 2;
        case 0x85:
            sta(this, AddressingMode::zero_page);
            return 3;
        case 0x95:
            sta(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x8d:
            sta(this, AddressingMode::absolute);
            return 4;
        case 0x9d:
            sta(this, AddressingMode::absolute_idx_x);
            return 5;
        case 0x99:
            sta(this, AddressingMode::absolute_idx_y);
            return 5;
        case 0x81:
            sta(this, AddressingMode::indirect_idx_x);
            return 6;
        case 0x91:
            sta(this, AddressingMode::indirect_idx_y);
            return 6;
        case 0x86:
            stx(this, AddressingMode::zero_page);
            return 3;
        case 0x96:
            stx(this, AddressingMode::zero_page_idx_y);
            return 4;
        case 0x8e:
            stx(this, AddressingMode::absolute);
            return 4;
        case 0x84:
            sty(this, AddressingMode::zero_page);
            return 3;
        case 0x94:
            sty(this, AddressingMode::zero_page_idx_x);
            return 4;
        case 0x8c:
            sty(this, AddressingMode::absolute);
            return 4;
        case 0xaa:
            tax(this);
            return 2;
        case 0xa8:
            tay(this);
            return 2;
        case 0xba:
            tsx(this);
            return 2;
        case 0x8a:
            txa(this);
            return 2;
        case 0x9a:
            txs(this);
            return 2;
        case 0x98:
            tya(this);
            return 2;
        default:
            printf("Unimplemented opcode %hhu!\n", opcode);
            exit(0);
            return 0;
    }
}