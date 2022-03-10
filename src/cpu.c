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

void decode_exec(uint8_t opcode)
{
    printf("executing: %x\n", opcode);
    switch (opcode)
    {
        case 0x01: cpu.bc = read_u16_param(); break; // LD BC,u16
        case 0x02: mmu_write(cpu.bc, cpu.a); break; // LD (BC),A

        case 0x31: cpu.sp = read_u16_param(); break; // LD SP,u16

        case 0xC3: break; // JP u16

        case 0xFE: break; // CP A,u8

        default: printf("opcode not implemented\n"); break;
    }
}