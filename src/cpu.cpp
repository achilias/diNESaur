#include "cpu.h"
#include <tuple>

uint16_t CPUMem::mirror(uint16_t addr) const {
    if (!enable_mirroring)
        return addr;
    if (in_range(addr, 0x0, 0x1fff))
        return addr & 0x7ff;
    if (in_range(addr, 0x2000, 0x3fff))
        return addr & 0x2007;
    return addr;
}

auto CPU::mem_fetch(AddressingMode mode) -> std::tuple<uint16_t, uint8_t> {
	switch (mode) {
		uint16_t tmp_u16;

		// used for zero page & indirect addressing to utilize default unsigned wraparound overflow behavior
		uint8_t tmp_u8;
		case AddressingMode::immediate:
			return {0, mem.read_byte(pc++)};
		case AddressingMode::indirect:
		case AddressingMode::absolute:
			tmp_u16 = mem.read_two_bytes(pc);
			pc += 2;
			return {tmp_u16, mem.read_byte(tmp_u16)};
		case AddressingMode::absolute_idx_x:
			tmp_u16 = reg_x + mem.read_two_bytes(pc);
			pc += 2;
			return {tmp_u16, mem.read_byte(tmp_u16)};
		case AddressingMode::absolute_idx_y:
			tmp_u16 = reg_y + mem.read_two_bytes(pc);
			pc += 2;
			return {tmp_u16, mem.read_byte(tmp_u16)};
		case AddressingMode::zero_page:
			tmp_u8 = mem.read_byte(pc++);
			return {tmp_u8, mem.read_byte(tmp_u8)};
		case AddressingMode::zero_page_idx_x:
			tmp_u8 = reg_x + mem.read_byte(pc++);
			return {tmp_u8, mem.read_byte(tmp_u8)};
		case AddressingMode::zero_page_idx_y:
			tmp_u8 = reg_y + mem.read_byte(pc++);
			return {tmp_u8, mem.read_byte(tmp_u8)};
		case AddressingMode::indirect_idx_x:
			tmp_u8 = reg_x + mem.read_byte(pc++);
			tmp_u16 = mem.read_byte(tmp_u8++);
			tmp_u16 += mem.read_byte(tmp_u8) << 8;
			return {tmp_u16, mem.read_byte(tmp_u16)};
		case AddressingMode::indirect_idx_y:
			tmp_u8 = mem.read_byte(pc++);
			tmp_u16 = mem.read_byte(tmp_u8++);
			tmp_u16 += (mem.read_byte(tmp_u8) << 8) + reg_y;
			return {tmp_u16, mem.read_byte(tmp_u16)};
	}
}

size_t CPU::execute_instr() {
    // TODO: Implement checks to set these flags and return correct number of cpu cycles
    bool pg_cross = false, branch_taken = false, new_page = false;
    switch (uint8_t opcode = mem.read_byte(pc++); opcode) {
        case 0x69:
            adc(AddressingMode::immediate);
            return 2;
        case 0x65:
            adc(AddressingMode::zero_page);
            return 3;
        case 0x75:
            adc(AddressingMode::zero_page_idx_x);
            return 4;
        case 0x6d:
            adc(AddressingMode::absolute);
            return 4;
        case 0x7d:
            // TODO: implement check for page crossing
            pg_cross = adc(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x79:
            pg_cross = adc(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x61:
            adc(AddressingMode::indirect_idx_x);
            return 6;
        case 0x71:
            pg_cross = adc(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x29:
            and_(AddressingMode::immediate);
            return 2;
        case 0x25:
            and_(AddressingMode::zero_page);
            return 3;
        case 0x35:
            and_(AddressingMode::zero_page_idx_x);
            return 4;
        case 0x2d:
            and_(AddressingMode::absolute);
            return 4;
        case 0x3d:
            pg_cross = and_(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x39:
            pg_cross = and_(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x21:
            and_(AddressingMode::indirect_idx_x);
            return 6;
        case 0x31:
            and_(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xa:
            asl(AddressingMode::accumulator);
            return 2;
        case 0x6:
            asl(AddressingMode::zero_page);
            return 5;
        case 0x16:
            asl(AddressingMode::zero_page_idx_x);
            return 6;
        case 0xe:
            asl(AddressingMode::absolute);
            return 6;
        case 0x1e:
            asl(AddressingMode::absolute_idx_x);
            return 7;
        case 0x90:
            bcc();
            return 2 + branch_taken + new_page;
        case 0xb0:
            bcs();
            return 2 + branch_taken + new_page;
        case 0xf0:
            beq();
            return 2 + branch_taken + new_page;
        case 0x24:
            bit(AddressingMode::zero_page);
            return 3;
        case 0x2c:
            bit(AddressingMode::absolute);
            return 4;
        case 0x30:
            bmi();
            return 2 + branch_taken + new_page;
        case 0xd0:
            bne();
            return 2 + branch_taken + new_page;
        case 0x10:
            bpl();
            return 2 + branch_taken + new_page;
        case 0x00: // BRK
            return 7;
        case 0x50:
            bvc();
            return 2 + branch_taken + new_page;
        case 0x70:
            bvs();
            return 2 + branch_taken + new_page;
        case 0x18:
            clc();
            return 2;
        case 0xd8:
            cld();
            return 2;
        case 0x58:
            cli();
            return 2;
        case 0xb8:
            clv();
            return 2;
        case 0xc9:
            cmp(AddressingMode::immediate);
            return 2;
        case 0xc5:
            cmp(AddressingMode::zero_page);
            return 3;
        case 0xd5:
            cmp(AddressingMode::zero_page_idx_x);
            return 4;
        case 0xcd:
            cmp(AddressingMode::absolute);
            return 4;
        case 0xdd:
            cmp(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xd9:
            cmp(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xc1:
            cmp(AddressingMode::indirect_idx_x);
            return 6;
        case 0xd1:
            cmp(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xe0:
            cpx(AddressingMode::immediate);
            return 2;
        case 0xe4:
            cpx(AddressingMode::zero_page);
            return 3;
        case 0xec:
            cpx(AddressingMode::absolute);
            return 4;
        case 0xc0:
            cpy(AddressingMode::immediate);
            return 2;
        case 0xc4:
            cpy(AddressingMode::zero_page);
            return 3;
        case 0xcc:
            cpy(AddressingMode::absolute);
            return 4;
        case 0xc6:
            dec(AddressingMode::zero_page);
            return 5;
        case 0xd6:
            dec(AddressingMode::zero_page_idx_x);
            return 6;
        case 0xce:
            dec(AddressingMode::absolute);
            return 6;
        case 0xde:
            dec(AddressingMode::absolute_idx_x);
            return 7;
        case 0xca:
            dex();
            return 2;
        case 0x88:
            dey();
            return 2;
        case 0x49:
            eor(AddressingMode::immediate);
            return 2;
        case 0x45:
            eor(AddressingMode::zero_page);
            return 3;
        case 0x55:
            eor(AddressingMode::zero_page_idx_x);
            return 4;
        case 0x4d:
            eor(AddressingMode::absolute);
            return 4;
        case 0x5d:
            eor(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x59:
            eor(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x41:
            eor(AddressingMode::indirect_idx_x);
            return 6;
        case 0x51:
            eor(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xe6:
            inc(AddressingMode::zero_page);
            return 5;
        case 0xf6:
            inc(AddressingMode::zero_page_idx_x);
            return 6;
        case 0xee:
            inc(AddressingMode::absolute);
            return 6;
        case 0xfe:
            inc(AddressingMode::absolute_idx_x);
            return 7;
        case 0xe8:
            inx();
            return 2;
        case 0xc8:
            iny();
            return 2;
        case 0x4c:
            jmp(AddressingMode::absolute);
            return 3;
        case 0x6c:
            jmp(AddressingMode::indirect);
            return 5;
        case 0x20:
            jsr();
            return 6;
        case 0xa9:
            lda(AddressingMode::immediate);
            return 2;
        case 0xa5:
            lda(AddressingMode::zero_page);
            return 3;
        case 0xb5:
            lda(AddressingMode::zero_page_idx_x);
            return 4;
        case 0xad:
            lda(AddressingMode::absolute);
            return 4;
        case 0xbd:
            lda(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xb9:
            lda(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xa1:
            lda(AddressingMode::indirect_idx_x);
            return 6;
        case 0xb1:
            lda(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0xa2:
            ldx(AddressingMode::immediate);
            return 2;
        case 0xa6:
            ldx(AddressingMode::zero_page);
            return 3;
        case 0xb6:
            ldx(AddressingMode::zero_page_idx_y);
            return 4;
        case 0xae:
            ldx(AddressingMode::absolute);
            return 4;
        case 0xbe:
            ldx(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xa0:
            ldy(AddressingMode::immediate);
            return 2;
        case 0xa4:
            ldy(AddressingMode::zero_page);
            return 3;
        case 0xb4:
            ldy(AddressingMode::zero_page_idx_x);
            return 4;
        case 0xac:
            ldy(AddressingMode::absolute);
            return 4;
        case 0xbc:
            ldy(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x4a:
            lsr(AddressingMode::accumulator);
            return 2;
        case 0x46:
            lsr(AddressingMode::zero_page);
            return 5;
        case 0x56:
            lsr(AddressingMode::zero_page_idx_x);
            return 6;
        case 0x4e:
            lsr(AddressingMode::absolute);
            return 6;
        case 0x5e:
            lsr(AddressingMode::absolute_idx_x);
            return 7;
        case 0xea: // NOP
            return 2;
        case 0x9:
            ora(AddressingMode::immediate);
            return 2;
        case 0x5:
            ora(AddressingMode::zero_page);
            return 3;
        case 0x15:
            ora(AddressingMode::zero_page_idx_x);
            return 4;
        case 0xd:
            ora(AddressingMode::absolute);
            return 4;
        case 0x1d:
            ora(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0x19:
            ora(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0x1:
            ora(AddressingMode::indirect_idx_x);
            return 6;
        case 0x11:
            ora(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x48:
            pha();
            return 3;
        case 0x8:
            php();
            return 3;
        case 0x68:
            pla();
            return 4;
        case 0x28:
            plp();
            return 4;
        case 0x2a:
            rol(AddressingMode::accumulator);
            return 2;
        case 0x26:
            rol(AddressingMode::zero_page);
            return 5;
        case 0x36:
            rol(AddressingMode::zero_page_idx_x);
            return 6;
        case 0x2e:
            rol(AddressingMode::absolute);
            return 6;
        case 0x3e:
            rol(AddressingMode::absolute_idx_x);
            return 7;
        case 0x6a:
            ror(AddressingMode::accumulator);
            return 2;
        case 0x66:
            ror(AddressingMode::zero_page);
            return 5;
        case 0x76:
            ror(AddressingMode::zero_page_idx_x);
            return 6;
        case 0x6e:
            ror(AddressingMode::absolute);
            return 6;
        case 0x7e:
            ror(AddressingMode::absolute_idx_x);
            return 7;
        case 0x40:
            rti();
            return 6;
        case 0x60:
            rts();
            return 6;
        case 0xe9:
            sbc(AddressingMode::immediate);
            return 2;
        case 0xe5:
            sbc(AddressingMode::zero_page);
            return 3;
        case 0xf5:
            sbc(AddressingMode::zero_page_idx_x);
            return 4;
        case 0xed:
            sbc(AddressingMode::absolute);
            return 4;
        case 0xfd:
            sbc(AddressingMode::absolute_idx_x);
            return 4 + pg_cross;
        case 0xf9:
            sbc(AddressingMode::absolute_idx_y);
            return 4 + pg_cross;
        case 0xe1:
            sbc(AddressingMode::indirect_idx_x);
            return 6;
        case 0xf1:
            sbc(AddressingMode::indirect_idx_y);
            return 5 + pg_cross;
        case 0x38:
            sec();
            return 2;
        case 0xf8:
            sed();
            return 2;
        case 0x78:
            sei();
            return 2;
        case 0x85:
            sta(AddressingMode::zero_page);
            return 3;
        case 0x95:
            sta(AddressingMode::zero_page_idx_x);
            return 4;
        case 0x8d:
            sta(AddressingMode::absolute);
            return 4;
        case 0x9d:
            sta(AddressingMode::absolute_idx_x);
            return 5;
        case 0x99:
            sta(AddressingMode::absolute_idx_y);
            return 5;
        case 0x81:
            sta(AddressingMode::indirect_idx_x);
            return 6;
        case 0x91:
            sta(AddressingMode::indirect_idx_y);
            return 6;
        case 0x86:
            stx(AddressingMode::zero_page);
            return 3;
        case 0x96:
            stx(AddressingMode::zero_page_idx_y);
            return 4;
        case 0x8e:
            stx(AddressingMode::absolute);
            return 4;
        case 0x84:
            sty(AddressingMode::zero_page);
            return 3;
        case 0x94:
            sty(AddressingMode::zero_page_idx_x);
            return 4;
        case 0x8c:
            sty(AddressingMode::absolute);
            return 4;
        case 0xaa:
            tax();
            return 2;
        case 0xa8:
            tay();
            return 2;
        case 0xba:
            tsx();
            return 2;
        case 0x8a:
            txa();
            return 2;
        case 0x9a:
            txs();
            return 2;
        case 0x98:
            tya();
            return 2;
    }
}

bool CPU::adc(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);

	uint16_t result = accum + get<1>(mem_loc) + get_carry();
	set_carry(result > 0xff);
	result = (uint8_t) result;
	set_overflow((result ^ accum) & (result ^ get<1>(mem_loc)) & 0x80);
	accum = result;
	set_zero(accum == 0);
	set_negative(accum & 0x80);
}

bool CPU::and_(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	accum &= get<1>(mem_loc);
	set_zero(accum == 0);
	set_negative(accum & 0x80);
}

void CPU::asl(AddressingMode addr_mode){
	if (addr_mode == AddressingMode::accumulator) {
		set_carry(accum & 0x80);
		accum = accum << 1;
		set_zero(accum == 0);
		set_negative(accum & 0x80);
		return;
	}
    auto mem_loc = mem_fetch(addr_mode);
    set_carry(get<1>(mem_loc) & 0x80);
    uint8_t tmp = get<1>(mem_loc) << 1;
	mem.write_byte(get<0>(mem_loc), tmp);
    set_zero(tmp == 0);
    set_negative(tmp & 0x80);
}

void CPU::bcc() {
	int8_t displacement = mem.read_byte(pc++);
	if (!get_carry())
		pc += displacement;
}

void CPU::bcs() {
	int8_t displacement = mem.read_byte(pc++);
	if (get_carry())
		pc += displacement;
}

void CPU::beq() {
	int8_t displacement = mem.read_byte(pc++);
	if(get_zero())
		pc += displacement;
}

void CPU::bit(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	uint8_t tmp = accum & get<1>(mem_loc);
	set_zero(tmp == 0);
	set_overflow(get<1>(mem_loc) & 0x40);
	set_negative(get<1>(mem_loc) & 0x80);
}

void CPU::bmi() {
	int8_t displacement = mem.read_byte(pc++);
	if(get_negative())
		pc += displacement;
}

void CPU::bne() {
	int8_t displacement = mem.read_byte(pc++);
	if(!get_zero())
		pc += displacement;
}

void CPU::bpl() {
	int8_t displacement = mem.read_byte(pc++);
	if(!get_negative())
		pc += displacement;
}

void CPU::brk() {
	//TODO: Implement interrupts
}

void CPU::bvc() {
	int8_t displacement = mem.read_byte(pc++);
	if(!get_overflow())
		pc += displacement;
}

void CPU::bvs() {
	int8_t displacement = mem.read_byte(pc++);
	if(get_overflow())
		pc += displacement;
}

void CPU::clc() {
	set_carry(0);
}

void CPU::cld() {
	sr &= ~0x8;
}

void CPU::cli() {
	set_disable_interrupt(0);
}

void CPU::clv() {
	set_overflow(0);
}

void CPU::cmp(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	set_carry(accum >= get<1>(mem_loc));
	int8_t result = accum - get<1>(mem_loc);
	set_zero(result == 0);
	set_negative(result < 0);
}

void CPU::cpx(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	set_carry(reg_x >= get<1>(mem_loc));
	int8_t result = reg_x - get<1>(mem_loc);
	set_zero(result == 0);
	set_negative(result < 0);
}

void CPU::cpy(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	set_carry(reg_y >= get<1>(mem_loc));
	int8_t result = reg_y - get<1>(mem_loc);
	set_zero(result == 0);
	set_negative(result < 0);
}

void CPU::dec(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	uint8_t tmp = get<1>(mem_loc) - 1;
	set_zero(tmp == 0);
	set_negative(tmp & 0x80);
	mem.write_byte(get<0>(mem_loc), tmp);
}

void CPU::dex() {
	set_zero(--reg_x == 0);
	set_negative(reg_x & 0x80);
}

void CPU::dey() {
	set_zero(--reg_y == 0);
	set_negative(reg_y & 0x80);
}

void CPU::eor(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	accum ^= get<1>(mem_loc);
	set_zero(accum == 0);
	set_negative(accum & 0x80);
}

void CPU::inc(AddressingMode addr_mode) {
	auto mem_loc = mem_fetch(addr_mode);
	uint8_t tmp = get<1>(mem_loc) + 1;
	set_zero(tmp == 0);
	set_negative(tmp & 0x80);
	mem.write_byte(get<0>(mem_loc), tmp);
}

void CPU::inx() {
	set_zero(++reg_x == 0);
	set_negative(reg_x & 0x80);
}

void CPU::iny() {
	set_zero(++reg_y == 0);
	set_negative(reg_y & 0x80);
}

void CPU::jmp(AddressingMode addr_mode) {

}

void CPU::jsr() {

}

void CPU::lda(AddressingMode addr_mode) {

}

void CPU::ldx(AddressingMode addr_mode) {

}

void CPU::ldy(AddressingMode addr_mode) {

}

void CPU::lsr(AddressingMode addr_mode) {

}

void CPU::ora(AddressingMode addr_mode) {

}

void CPU::pha() {

}

void CPU::php() {

}

void CPU::pla() {

}

void CPU::plp() {

}

void CPU::rol(AddressingMode addr_mode) {

}

void CPU::ror(AddressingMode addr_mode) {

}

void CPU::rti() {

}

void CPU::rts() {

}

void CPU::sbc(AddressingMode addr_mode) {

}

void CPU::sec() {

}

void CPU::sed() {

}

void CPU::sei() {

}

void CPU::sta(AddressingMode addr_mode) {

}

void CPU::stx(AddressingMode addr_mode) {

}

void CPU::sty(AddressingMode addr_mode) {

}

void CPU::tax() {

}

void CPU::tay() {

}

void CPU::tsx() {

}

void CPU::txa() {

}

void CPU::txs() {

}

void CPU::tya() {

}