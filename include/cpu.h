#pragma once

#include <tuple>
#include <cstddef>
#include "bus.h"
#include "common.h"

class CPU {
public:
    CPU(Bus& bus) : bus(bus), sp(0xff), pc(0), accum(0), reg_x(0), reg_y(0), sr(0) {};
    size_t execute_instr();
protected:
    static const uint16_t stack_base = 0x100;
    Bus& bus;
    uint8_t sp;
    uint16_t pc;
    uint8_t accum;
    uint8_t reg_x;
    uint8_t reg_y;
    uint8_t sr;

	uint16_t get_addr(AddressingMode mode);
    void set_carry(bool cond) {sr = cond ? sr | 0x1 : sr & ~0x1;}
    bool get_carry() const {return sr & 0x1;}
    void set_zero(bool cond) {sr = cond ? sr | 0x2 : sr & ~0x2;}
	bool get_zero() const {return sr & 0x2;}
    void set_disable_interrupt(bool cond) {sr = cond ? sr | 0x4 : sr & ~0x4;}
    void set_overflow(bool cond) {sr = cond ? sr | 0x40 : sr & ~0x40;};
	void set_decimal(bool cond) {sr = cond ? sr | 0x8 : sr & ~0x8;}
	bool get_decimal() {return sr & 0x8;}
	bool get_overflow() const {return sr & 0x40;}
    void set_negative(bool cond) {sr = cond ? sr | 0x80 : sr & ~0x80;};
	bool get_negative() const {return sr & 0x80;}

private:
    bool adc(AddressingMode addr_mode);
    bool and_(AddressingMode addr_mode);
    void asl(AddressingMode addr_mode);
    void bcc();
    void bcs();
    void beq();
    void bit(AddressingMode addr_mode);
    void bmi();
    void bne();
    void bpl();
    void brk();
    void bvc();
    void bvs();
    void clc();
    void cld();
    void cli();
    void clv();
    void cmp(AddressingMode addr_mode);
    void cpx(AddressingMode addr_mode);
    void cpy(AddressingMode addr_mode);
    void dec(AddressingMode addr_mode);
    void dex();
    void dey();
    void eor(AddressingMode addr_mode);
    void inc(AddressingMode addr_mode);
    void inx();
    void iny();
    void jmp(AddressingMode addr_mode);
    void jsr();
    void lda(AddressingMode addr_mode);
    void ldx(AddressingMode addr_mode);
    void ldy(AddressingMode addr_mode);
    void lsr(AddressingMode addr_mode);
    void ora(AddressingMode addr_mode);
    void pha();
    void php();
    void pla();
    void plp();
    void rol(AddressingMode addr_mode);
    void ror(AddressingMode addr_mode);
    void rti();
    void rts();
    void sbc(AddressingMode addr_mode);
    void sec();
    void sed();
    void sei();
    void sta(AddressingMode addr_mode);
    void stx(AddressingMode addr_mode);
    void sty(AddressingMode addr_mode);
    void tax();
    void tay();
    void tsx();
    void txa();
    void txs();
    void tya();
};