#include "cpu.h"

size_t CPU::execute_instr() {
    switch (uint8_t opcode = mem.read_byte(pc++); opcode) {
        case 0x00:
            break;
    }

};

void CPU::adc(AddressingMode addr_mode) {

};

void CPU::and_(AddressingMode addr_mode) {

};

void CPU::asl(AddressingMode addr_mode){

};

void CPU::bcc() {

};

void CPU::bcs() {

};

void CPU::beq() {

};

void CPU::bit(AddressingMode addr_mode) {

};

void CPU::bmi() {

};

void CPU::bne() {

};

void CPU::bpl() {

};

void CPU::brk() {

};

void CPU::bvc() {

};

void CPU::bvs() {

};

void CPU::clc() {

};

void CPU::cld() {

};

void CPU::cli() {

};

void CPU::clv() {

};

void CPU::cmp(AddressingMode addr_mode) {

};

void CPU::cpx(AddressingMode addr_mode) {

};

void CPU::cpy(AddressingMode addr_mode) {

};

void CPU::dec(AddressingMode addr_mode) {

};

void CPU::dex() {

};

void CPU::dey() {

};

void CPU::eor(AddressingMode addr_mode) {

};

void CPU::inc(AddressingMode addr_mode) {

};

void CPU::inx() {

};

void CPU::iny() {

};

void CPU::jmp(AddressingMode addr_mode) {

};

void CPU::jsr() {

};

void CPU::lda(AddressingMode addr_mode) {

};

void CPU::ldx(AddressingMode addr_mode) {

};

void CPU::ldy(AddressingMode addr_mode) {

};

void CPU::lsr(AddressingMode addr_mode) {

};

void CPU::ora(AddressingMode addr_mode) {

};

void CPU::pha() {

};

void CPU::php() {

};

void CPU::pla() {

};

void CPU::plp() {

};

void CPU::rol(AddressingMode addr_mode) {

};

void CPU::ror(AddressingMode addr_mode) {

};

void CPU::rti() {

};

void CPU::rts() {

};

void CPU::sbc(AddressingMode addr_mode) {

};

void CPU::sec() {

};

void CPU::sed() {

};

void CPU::sei() {

};

void CPU::sta(AddressingMode addr_mode) {

};

void CPU::stx(AddressingMode addr_mode) {

};

void CPU::sty(AddressingMode addr_mode) {

};

void CPU::tax() {

};

void CPU::tay() {

};

void CPU::tsx() {

};

void CPU::txa() {

};

void CPU::txs() {

};

void tya() {

};