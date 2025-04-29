#ifndef CPU_H
#define CPU_H

#include "memory.h"
#include <tuple>

class CPUMem : public Memory {
public:
	CPUMem(size_t mem_size, bool enable_mirroring) : Memory(mem_size), enable_mirroring(enable_mirroring) {};
	uint16_t mirror (uint16_t addr) const;
    bool enable_mirroring;
};

class CPU {
public:
    CPU(bool enable_mirroring) : mem(65536, enable_mirroring), sp(0xff), pc(0), accum(0), reg_x(0), reg_y(0), sr(0) {};
    size_t execute_instr();
protected:
    static const uint16_t stack_base = 0x1ff;
    CPUMem mem;
    uint8_t sp;
    uint16_t pc;
    uint8_t accum;
    uint8_t reg_x;
    uint8_t reg_y;
    uint8_t sr;

	auto mem_fetch(AddressingMode mode) -> std::tuple<uint16_t, uint8_t>;
    void set_carry(bool cond) {sr = cond ? sr | 0x1 : sr & ~0x1;}
    bool get_carry() const {return sr & 0x1;}
    void set_zero(bool cond) {sr = cond ? sr | 0x2 : sr & ~0x2;}
    void set_disable_interrupt(bool cond) {sr = cond ? sr | 0x4 : sr & ~0x4;}
    void set_overflow(bool cond) {sr = cond ? sr | 0x40 : sr & ~0x40;};
    void set_negative(bool cond) {sr = cond ? sr | 0x80 : sr & ~0x80;};

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

#endif