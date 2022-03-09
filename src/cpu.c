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
    mmu_read(cpu.pc, &opcode);
    // cpu.pc += ?
    cpu.pc++;
    decode_exec(opcode);
}

void decode_exec(uint8_t opcode)
{
    printf("executing: %x\n", opcode);
    switch (opcode)
    {
        case 0x1: // LD BC,d16
            break; 

        case 0x2: // LD (BC),A
            break; 

        default: printf("opcode not implemented\n"); break;
    }
}