#include "cpu.h"

#include <stdio.h>

#include "memory.h"

struct cpu cpu;

uint8_t zflag() { return cpu.f & 0x80; }
uint8_t nflag() { return cpu.f & 0x40; }
uint8_t hflag() { return cpu.f & 0x20; }
uint8_t cflag() { return cpu.f & 0x10; }

void set_zflag(uint8_t value) { if(value) cpu.f |= 0x80; else cpu.f &= 0x7F; }
void set_nflag(uint8_t value) { if(value) cpu.f |= 0x40; else cpu.f &= 0xBF; }
void set_hflag(uint8_t value) { if(value) cpu.f |= 0x20; else cpu.f &= 0xDF; }
void set_cflag(uint8_t value) { if(value) cpu.f |= 0x10; else cpu.f &= 0xEF; }

void decode_exec(uint8_t opcode);

void tick()
{
    uint8_t opcode;
    mmu_read(cpu.pc++, &opcode);
    decode_exec(opcode);
}

uint8_t read_u8_param()
{
    uint8_t param;
    mmu_read(cpu.pc++, &param);
    return param;
}

uint16_t read_u16_param()
{
    uint8_t low, high;
    mmu_read(cpu.pc++, &low);
    mmu_read(cpu.pc++, &high);

    return (uint16_t)low & ( (uint16_t)high << 0x8 );
}

// Get byte stored at [HL]
uint8_t get_byte_hl()
{
    uint8_t byte;
    mmu_read(cpu.hl, &byte);
    return byte;
}

/* -------------- 16 bit load instructions -------------- */

void push_u16(uint16_t value)
{
    mmu_write(cpu.sp--, value & 0xF0);
    mmu_write(cpu.sp--, value & 0xF);
}

uint8_t pop_u16()
{
    uint8_t low;
    uint8_t high;
    mmu_read(++cpu.sp, &low);
    mmu_read(++cpu.sp, &high);

    return (uint16_t)(high << 0x8) & (uint16_t)low;
}

/* -------------- 8 bit arithmetic -------------- */

void compare_u8(uint8_t a, uint8_t b)
{
    set_zflag(a == b);
    set_nflag(1);
    // set_hflag( (a&0xF) < (b&0xF) ); // TODO: Check if its correct
    set_hflag( ((a-b) & 0xF) > (a & 0xF) );
    set_cflag(a<b);
}

void xor_u8(uint8_t* a, uint8_t b)
{
    *a ^= b;
    set_zflag(*a == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(0);
}

/* -------------- Single-bit Operation instructions -------------- */

void test_bit(uint8_t byte, uint8_t bitn)
{
    uint8_t bit = byte & (0x1 << bitn);
    set_zflag(bit == 0);
    set_nflag(0);
    set_hflag(1);
}

/* -------------- jumps -------------- */

void restart(uint16_t address)
{
    push_u16(cpu.pc);
    cpu.pc = address;
}

/* -------------- decode & execute -------------- */

void decode_exec_cb(uint8_t opcode);

void decode_exec(uint8_t opcode)
{
    uint8_t cb = 0;

    printf("executing: %x\n", opcode);
    switch (opcode)
    {
        case 0x01: cpu.bc = read_u16_param(); break; // LD BC,u16
        case 0x02: mmu_write(cpu.bc, cpu.a); break; // LD (BC),A

        case 0x21: cpu.hl = read_u16_param(); break; // LD HL,u16

        case 0x31: cpu.sp = read_u16_param(); break; // LD SP,u16
        case 0x32: mmu_write(cpu.hl--, cpu.a); break; // LD (HL-),A

        case 0xAF: xor_u8(&cpu.a, cpu.a); break; // XOR A,A

        case 0xC3: break; // JP u16

        case 0xCB: cb = 1; break; // PREFIX CB

        case 0xFE: compare_u8(cpu.a, read_u8_param()); break; // CP A,u8
        case 0xFF: restart(0x38); break; // RST 38h

        default: printf("opcode not implemented\n"); break;
    }

    if(cb)
    {
        mmu_read(cpu.pc++, &opcode);
        decode_exec_cb(opcode);
    }
}

void decode_exec_cb(uint8_t opcode)
{
    if(opcode >= 0x40 && opcode <= 0x7F)
    {
        // BIT b,r
        uint8_t bitn = (opcode-0x40) / 0x8;

        switch ( (opcode-0x40) % 0x8 )
        {
            case 0: test_bit(cpu.b, bitn); break;
            case 1: test_bit(cpu.c, bitn); break;
            case 2: test_bit(cpu.d, bitn); break;
            case 3: test_bit(cpu.e, bitn); break;
            case 4: test_bit(cpu.h, bitn); break;
            case 5: test_bit(cpu.l, bitn); break;
            case 6: test_bit(get_byte_hl(), bitn); break;
            case 7: test_bit(cpu.a, bitn); break;
        }
    }
}