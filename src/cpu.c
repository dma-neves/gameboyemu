#include "cpu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "ppu.h"

/* -------------- data -------------- */

long debug_counter = 1;

struct cpu cpu;
uint8_t debug_enable = 0;
static const uint16_t interrupt_vector[] = { 0x40, 0x48, 0x50, 0x58, 0x60 };

/*
    Extra cycles regarding conditional jumps that can take more 
    cycles if the condition is met
*/
uint8_t extra_cycles;

/*
    Cycles arrays coppied from https://github.com/alt-romes/gameboyemulator/blob/master/src/cpu.c
*/
static const uint8_t op_cycles[0x100] = {
    4, 12, 8, 8, 4, 4, 8, 4,     20, 8, 8, 8, 4, 4, 8, 4, // 0x0_
    4, 12, 8, 8, 4, 4, 8, 4,     12, 8, 8, 8, 4, 4, 8, 4, // 0x1_
    8, 12, 8, 8, 4, 4, 8, 4,     8, 8, 8, 8, 4, 4, 8, 4, // 0x2_
    8, 12, 8, 8, 12, 12, 12, 4,  8, 8, 8, 8, 4, 4, 8, 4, // 0x3_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0x4_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0x5_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0x6_
    8, 8, 8, 8, 8, 8, 4, 8,  4, 4, 4, 4, 4, 4, 8, 4, // 0x7_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0x8_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0x9_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0xa_
    4, 4, 4, 4, 4, 4, 8, 4,  4, 4, 4, 4, 4, 4, 8, 4, // 0xb_
    8, 12, 12, 16, 12, 16, 8, 16,  8, 16, 12, 4, 12, 24, 8, 16, // 0xc_
    8, 12, 12,  0, 12, 16, 8, 16,  8, 16, 12, 0, 12,  0, 8, 16, // 0xd_
    12, 12, 8,  0, 0, 16, 8, 16,  16,  4, 16, 0,  0,  0, 8, 16, // 0xe_
    12, 12, 8,  4, 0, 16, 8, 16,  12,  8, 16, 4,  0,  0, 8, 16  // 0xf_
};

static const uint8_t cb_op_cycles[0x100] = {
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0x0_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0x1_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0x2_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0x3_
    8, 8, 8, 8, 8,  8, 12, 8,  8, 8, 8, 8, 8, 8, 12, 8, // 0x4_
    8, 8, 8, 8, 8,  8, 12, 8,  8, 8, 8, 8, 8, 8, 12, 8, // 0x5_
    8, 8, 8, 8, 8,  8, 12, 8,  8, 8, 8, 8, 8, 8, 12, 8, // 0x6_
    8, 8, 8, 8, 8,  8, 12, 8,  8, 8, 8, 8, 8, 8, 12, 8, // 0x7_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0x8_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0x9_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0xa_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0xb_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0xc_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0xd_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8, // 0xe_
    8, 8, 8, 8, 8,  8, 16, 8,  8, 8, 8, 8, 8, 8, 16, 8  // 0xf_
};

/* -------------- flags -------------- */

uint8_t zflag() { return (cpu.f & 0x80) != 0; }
uint8_t nflag() { return (cpu.f & 0x40) != 0; }
uint8_t hflag() { return (cpu.f & 0x20) != 0; }
uint8_t cflag() { return (cpu.f & 0x10) != 0; }

void set_zflag(uint8_t value) { if(value) cpu.f |= 0x80; else cpu.f &= 0x7F; }
void set_nflag(uint8_t value) { if(value) cpu.f |= 0x40; else cpu.f &= 0xBF; }
void set_hflag(uint8_t value) { if(value) cpu.f |= 0x20; else cpu.f &= 0xDF; }
void set_cflag(uint8_t value) { if(value) cpu.f |= 0x10; else cpu.f &= 0xEF; }

/* -------------- reset -------------- */

void reset_cpu()
{
    memset(&cpu, 0, sizeof(cpu));
}

/* -------------- debug -------------- */

void set_debug(uint8_t value)
{
    debug_enable = value;
}

void print_cpu_state_wait()
{
    printf("cpu state:\n");
    printf("pc: %x\n", cpu.pc);
    printf("sp: %x\n", cpu.sp);

    printf("af: %x a: %x f: %x\n", cpu.af, cpu.a, cpu.f);
    printf("bc: %x b: %x c: %x\n", cpu.bc, cpu.b, cpu.c);
    printf("de: %x d: %x e: %x\n", cpu.de, cpu.d, cpu.e);
    printf("hl: %x h: %x l: %x\n", cpu.hl, cpu.h, cpu.l);
    printf("\n\n");
    getchar();
}

/* -------------- cpu step -------------- */

void decode_exec(uint8_t opcode);
void decode_exec_cb(uint8_t opcode);
int handle_interrupts();

uint8_t step()
{
    int cycles = 0;
    extra_cycles = 0;

    if(!cpu.hlt)
    {
        uint8_t opcode;
        mmu_read(cpu.pc++, &opcode);

        // if(debug_counter >= 8238)
        //     print_cpu_state_wait();

        if(opcode == 0xCB)
        {
            mmu_read(cpu.pc++, &opcode);
            if(debug_enable) { printf("cb %x %x\n", opcode, cpu.pc-2); debug_counter++; }
            decode_exec_cb(opcode);
            cycles = cb_op_cycles[opcode] + extra_cycles;
        }
        else
        {
            if(debug_enable) { printf("%x %x\n", opcode, cpu.pc-1); debug_counter++; }
            decode_exec(opcode);
            cycles = op_cycles[opcode] + extra_cycles;
        }
    }

    cycles += handle_interrupts();
    return cycles;
}

/* -------------- interrupts -------------- */

void call(uint16_t address);

int handle_interrupts()
{
    if(!cpu.ime)
        return 0;

    uint8_t interrupt_called = 0;
    for(int i = 0; i <= 4 && !interrupt_called; i++)
    {
        if(((*ie) >> i) & 0x1 && ((*intf) >> i) & 0x1)
        {
            if(i != 0x1 || lcdc_stat_interrupt())
            {
                interrupt_called = 1;
                cpu.ime = 0;
                cpu.hlt = 0;
                
                uint8_t bitmask = ~(0x1 << i);
                (*intf) &= bitmask;

                call(interrupt_vector[i]);
            }
        }
    }

    int cycles = interrupt_called ? 20 : 0;
    return cycles;
}

/* -------------- utils -------------- */

void opcode_not_implemented(uint8_t opcode)
{
    printf("opcode not implemented: 0x%x\n", opcode);
    exit(1);
}

void invalid(uint8_t opcode)
{
    printf("invlalid opcode: 0x%x\n", opcode);
    exit(1);
}

uint8_t read_u8_param()
{
    uint8_t param;
    mmu_read(cpu.pc++, &param);
    return param;
}

int8_t read_i8_param()
{
    return (int8_t)read_u8_param();
}

uint16_t read_u16_param()
{
    uint8_t low, high;
    mmu_read(cpu.pc++, &low);
    mmu_read(cpu.pc++, &high);
    return (uint16_t)low | ( (uint16_t)high << 0x8 );
}

uint8_t get_register(uint8_t* reg)
{
    if(reg == REG_HL)
    {
        uint8_t byte;
        mmu_read(cpu.hl, &byte);
        return byte;
    }

    return *reg;
}

void set_register(uint8_t* reg, uint8_t byte)
{
    if(reg == REG_HL)
        mmu_write(cpu.hl, byte);
    else
        *reg = byte;
}

/* 
    A lot arithmetic and load opcodes use the same operand / register
    depending on the alignment of the opcode within the opcodes table
    (this saves a lot of repetitive disassembly)
*/

uint8_t* get_aligned_register(uint8_t opcode)
{
    switch ( (opcode & 0xF) % 0x8 )
    {
        case 0: return &cpu.b;
        case 1: return &cpu.c;
        case 2: return &cpu.d;
        case 3: return &cpu.e;
        case 4: return &cpu.h;
        case 5: return &cpu.l;
        case 6: return REG_HL;
        case 7: return &cpu.a;

        default: return NULL;
    }
}

// uint8_t get_aligned_operand(uint8_t opcode)
// {
//     return get_register( get_aligned_register(opcode) );
// }

/* -------------- stack operations -------------- */

void push_u16(uint16_t value)
{
    mmu_write(--cpu.sp, (value >> 0x8)); // high
    mmu_write(--cpu.sp, value & 0xFF);   // low
}

uint16_t pop_u16()
{
    uint8_t low;
    uint8_t high;
    mmu_read(cpu.sp++, &low);
    mmu_read(cpu.sp++, &high);

    return (uint16_t)(high << 0x8) | (uint16_t)low;
}

void push_af()
{
    mmu_write(--cpu.sp, cpu.a);        // high
    mmu_write(--cpu.sp, cpu.f & 0xF0); // low
}

void pop_af()
{
    uint8_t low;
    uint8_t high;
    mmu_read(cpu.sp++, &low);
    mmu_read(cpu.sp++, &high);

    cpu.f = low & 0xF0;
    cpu.a = high;
}

void ld_hl_sp_offset(int8_t offset)
{
    set_zflag(0);
    set_nflag(0);
    set_hflag( (cpu.sp & 0xF) + (offset & 0xF) > 0xF );
    set_cflag( (cpu.sp & 0xFF) + (int16_t)offset > 0xFF ); // TODO: verify (signed offset)

    cpu.hl = cpu.sp+offset;
}

void add_sp_i8(int8_t value)
{
    set_zflag(0);
    set_nflag(0);
    set_hflag( (cpu.sp & 0xF) + (value & 0xF) > 0xF );
    set_cflag( (cpu.sp & 0xFF) + (int16_t)value > 0xFF ); // TODO: verify (signed value)

    cpu.sp += value;
}

/* -------------- 8 bit arithmetic -------------- */

void compare_u8(uint8_t b)
{
    uint8_t cp = cpu.a-b;

    set_zflag(cp == 0);
    set_nflag(1);
    set_hflag( (cp & 0xF) + (b & 0xF) > 0xF );
    set_cflag( cp > cpu.a );
}

void add_u8(uint8_t b)
{
    uint8_t result = cpu.a + b;
    set_zflag(result == 0);
    set_nflag(0);
    set_hflag( (cpu.a & 0xF) + (b & 0xF) > 0xF );
    set_cflag( (uint16_t)cpu.a + (uint16_t)b > 0xFF );

    cpu.a = result;
}

void adc_u8(uint8_t b)
{
    uint8_t b_carry = b + cflag();
    add_u8(b_carry); // TODO: Verify
}

void sub_u8(uint8_t b)
{
    compare_u8(b); // TODO: Verify
    cpu.a -= b;
}

void sbc_u8(uint8_t b)
{
    uint8_t b_carry = b + cflag(); 
    sub_u8(b_carry); // TODO: Verify
}

void and_u8(uint8_t b)
{
    cpu.a &= b;
    set_zflag(cpu.a == 0);
    set_nflag(0);
    set_hflag(1);
    set_cflag(0);
}

void xor_u8(uint8_t b)
{
    cpu.a ^= b;
    set_zflag(cpu.a == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(0);
}

void or_u8(uint8_t b)
{
    cpu.a |= b;
    set_zflag(cpu.a == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(0);
}

void inc_u8_register(uint8_t* r)
{
    uint8_t result = get_register(r) + 1;
    set_register(r, result);
    set_zflag(result == 0);
    set_nflag(0);
    set_hflag( (result & 0xF) == 0);
    // set_cflag(); not affected
}

void dec_u8_register(uint8_t* r)
{
    uint8_t result = get_register(r) - 1;
    set_register(r, result);
    set_zflag(result == 0);
    set_nflag(1);
    set_hflag( (result & 0x0F) == 0x0F); // TODO: why is this correct?
    // set_cflag(); not affected
}

/* -------------- 16 bit arithmetic -------------- */

void inc_u16_register(uint16_t* r)
{
    // Flags not affected
    (*r)++;
}

void dec_u16_register(uint16_t* r)
{
    // Flags not affected
    (*r)--;
}

void add_hl_u16(uint16_t w)
{
    // set_zflag(); Not affected
    set_nflag(0);
    set_hflag( (cpu.hl & 0x7FF) + (w & 0x7FF) > 0x7FF ); // Set if overflow from bit 11
    set_cflag( (uint32_t)cpu.hl + (uint32_t)w > 0xFFFF );

    cpu.hl += w;
}

/* -------------- Single-bit Operation instructions -------------- */

// Test bit bitn in register
void test_bit(uint8_t* reg, uint8_t bitn)
{
    uint8_t bit = ( get_register(reg) >> bitn) & 0x1;
    set_zflag(bit == 0);
    set_nflag(0);
    set_hflag(1);
}

// Reset bit bitn in register
void res_bit(uint8_t* reg, uint8_t bitn)
{
    uint8_t byte = get_register(reg) & ~(0x1 << bitn);
    set_register(reg, byte);
}

// Set bit bitn in register
void set_bit(uint8_t* reg, uint8_t bitn)
{
    uint8_t byte = get_register(reg) | (0x1 << bitn);
    set_register(reg, byte);
}


/* -------------- Rotate and Shift instructions -------------- */

// Rotate left circular
void rlc(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = (oldbyte << 0x1) | (oldbyte >> 0x7);
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x80);
}

// Rotate right circular
void rrc(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = (oldbyte >> 0x1) | (oldbyte << 0x7);
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x1);
}

// Rotate left through carry
void rl(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = (oldbyte << 0x1) | cflag();
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x80);
}

// Rotate right through carry
void rr(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = (oldbyte >> 0x1) | (cflag() << 0x7);
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x0);
}


// Rotate A left through carry
// Note: Contrary to RL, RLA does not affect the zero flag
void rla()
{
    // TODO: Verify flags

    uint8_t oldbyte = cpu.a;
    uint8_t byte = (oldbyte << 0x1) | cflag();
    cpu.a = byte;

    set_zflag(0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x80);
}

// Rotate A left circular
void rlca()
{
    uint8_t oldbyte = cpu.a;
    uint8_t byte = (oldbyte << 0x1) | (oldbyte >> 0x7);
    cpu.a = byte;

    set_zflag(0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x80);
}

// Rotate A right circular
void rrca()
{
    uint8_t oldbyte = cpu.a;
    uint8_t byte = (oldbyte >> 0x1) | (oldbyte << 0x7);
    cpu.a = byte;

    set_zflag(0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x1);
}

// Rotate A right through carry
void rra()
{
    uint8_t oldbyte = cpu.a;
    uint8_t byte = (oldbyte >> 0x1) | (cflag() << 0x7);
    cpu.a = byte;

    set_zflag(0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x0);
}

// Shift left into carry LSB set to 0
void sla(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = oldbyte << 0x1;
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x80);
}

// Shift right into carry MSB doent change
void sra(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = (oldbyte & 0x80) | (oldbyte >> 0x1);
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x1);
}

// Shift right into carry MSB set to 0
void srl(uint8_t* reg)
{
    uint8_t oldbyte = get_register(reg);
    uint8_t byte = oldbyte >> 0x1;
    set_register(reg, byte);

    set_zflag(byte == 0);
    set_nflag(0);
    set_hflag(0);
    set_cflag(oldbyte & 0x1);
}

void swap(uint8_t* reg)
{
    uint8_t low = get_register(reg) & 0xF;
    uint8_t high = get_register(reg) & 0xF0;
    uint16_t byte = (low << 0x4) | (high >> 0x4);
    set_register(reg, byte);
}

/* -------------- jumps -------------- */

void restart(uint16_t address)
{
    push_u16(cpu.pc); // Push address of next instruction
    cpu.pc = address;
}

void jump_relative(uint8_t cond, int8_t offset)
{
    if(cond)
    {
        cpu.pc += offset;
        extra_cycles += 4;
    }
}

void jump(uint16_t address)
{
    // TODO: check
    cpu.pc = address;
}

void cond_jump(uint8_t cond, uint16_t address)
{
    if(cond)
    {
        extra_cycles += 4;
        jump(address);
    }
}

void call(uint16_t address)
{
    push_u16(cpu.pc); // Push address of next instruction
    cpu.pc = address;
}

void cond_call(uint8_t cond, uint16_t address)
{
    if(cond)
    {
        push_u16(cpu.pc); // Push address of next instruction
        cpu.pc = address;
        extra_cycles += 12;
    }
}

void ret()
{
    cpu.pc = pop_u16();
}

void cond_ret(uint8_t cond)
{
    if(cond)
    {
        ret();
        extra_cycles += 12; // TODO: verify if 12 is correct
    }
}

void reti()
{
    cpu.ime = 1;
    ret();
}

/* -------------- miscellaneous -------------- */

void scf()
{
    // set_zflag(); Not affected
    set_nflag(0);
    set_hflag(0);
    set_cflag(1);
}

void cpl()
{
    cpu.a = ~cpu.a;
    // set_zflag(); Not affected
    set_nflag(1);
    set_hflag(1);
    // set_cflag(); Not affected
}

void rst(uint16_t address)
{
    call(address);
}

void ccf()
{
    // set_zflag(); Not affected
    set_nflag(0);
    set_hflag(0);
    set_cflag(!cflag());
}

void daa()
{
    // DAA implemetation based on jgilchrist's: https://github.com/jgilchrist/gbemu

    uint8_t reg = cpu.a;

    uint16_t correction = cflag() ? 0x60 : 0x00;

    if(hflag() || (!nflag() && ((reg & 0x0F) > 9)))
        correction |= 0x06;

    if(cflag() || (!nflag() && (reg > 0x99)))
        correction |= 0x60;

    if(nflag())
        reg = (uint8_t)(reg - correction);
    else
        reg = (uint8_t)(reg + correction);

    if(((correction << 2) & 0x100) != 0)
        set_cflag(1);

    set_hflag(0);
    set_zflag(0);
    cpu.a = reg;
}

/* -------------- decode & execute -------------- */

void decode_exec(uint8_t opcode)
{
    if(opcode == 0x76)
    {
        cpu.hlt = 1; // HLT
    }
    if(opcode < 0x40)
    {
        switch (opcode)
        {
            case 0x00: break; // NOP
            case 0x01: cpu.bc = read_u16_param(); break; // LD BC,u16
            case 0x02: mmu_write(cpu.bc, cpu.a); break; // LD (BC),A
            case 0x03: inc_u16_register(&cpu.bc); break; // INC BC
            case 0x04: inc_u8_register(&cpu.b); break; // INC B
            case 0x05: dec_u8_register(&cpu.b); break; // DEC B
            case 0x06: cpu.b = read_u8_param(); break; // LD B,u8
            case 0x07: rlca(); break; // RLCA
            case 0x08: mmu_write_u16(read_u16_param(), cpu.sp); break; // LD (u16),SP
            case 0x09: add_hl_u16(cpu.bc); break; // ADD HL,BC
            case 0x0A: mmu_read(cpu.bc, &cpu.a); break; // LD A,(BC)
            case 0x0B: dec_u16_register(&cpu.bc); break; // DEC BC
            case 0x0C: inc_u8_register(&cpu.c); break; // INC C
            case 0x0D: dec_u8_register(&cpu.c); break; // DEC C
            case 0x0E: cpu.c = read_u8_param(); break; // LD C,u8
            case 0x0F: rrca(); break; // RRCA
            // TODO: case 0x10: break; // STOP
            case 0x11: cpu.de = read_u16_param(); break; // LD DE,u16
            case 0x12: mmu_write(cpu.de, cpu.a); break; // LD (DE),A
            case 0x13: inc_u16_register(&cpu.de); break; // INC DE
            case 0x14: inc_u8_register(&cpu.d); break; // INC D
            case 0x15: dec_u8_register(&cpu.d); break; // DEC D
            case 0x16: cpu.d = read_u8_param(); break; // LD D,u8
            case 0x17: rla(); break; // RLA
            case 0x18: jump_relative(1, read_i8_param()); break; // JR i8
            case 0x19: add_hl_u16(cpu.de); break; // ADD HL,DE
            case 0x1A: mmu_read(cpu.de, &cpu.a); break; // LD A,(DE)
            case 0x1B: dec_u16_register(&cpu.de); break; // DEC DE
            case 0x1C: inc_u8_register(&cpu.e); break; // INC E
            case 0x1D: dec_u8_register(&cpu.e); break; // DEC E
            case 0x1E: cpu.e = read_u8_param(); break; // LD E,u8
            case 0x1F: rra(); break; // RRA
            case 0x20: jump_relative(!zflag(), read_i8_param()); break; // JR NZ,i8
            case 0x21: cpu.hl = read_u16_param(); break; // LD HL,u16
            case 0x22: mmu_write(cpu.hl++, cpu.a); break; // LD (HL+),A
            case 0x23: inc_u16_register(&cpu.hl); break; // INC HL
            case 0x24: inc_u8_register(&cpu.h); break; // INC H
            case 0x25: dec_u8_register(&cpu.h); break; // DEC H
            case 0x26: cpu.h = read_u8_param(); break; // LD H,u8
            case 0x27: daa(); break; // DAA
            case 0x28: jump_relative(zflag(), read_i8_param()); break; // JR Z,i8
            case 0x29: add_hl_u16(cpu.hl); break; // ADD HL,HL
            case 0x2A: mmu_read(cpu.hl++, &cpu.a); break; // LD A,(HL+)
            case 0x2B: dec_u16_register(&cpu.hl); break; // DEC HL
            case 0x2C: inc_u8_register(&cpu.l); break; // INC L
            case 0x2D: dec_u8_register(&cpu.l); break; // DEC L
            case 0x2E: cpu.l = read_u8_param(); break; // LD L,u8
            case 0x2F: cpl(); break; // CPL
            case 0x30: jump_relative(!cflag(), read_i8_param()); break; // JR NC,i8
            case 0x31: cpu.sp = read_u16_param(); break; // LD SP,u16
            case 0x32: mmu_write(cpu.hl--, cpu.a); break; // LD (HL-),A
            case 0x33: inc_u16_register(&cpu.sp); break; // INC SP
            case 0x34: inc_u8_register(REG_HL); break; // INC (HL)
            case 0x35: dec_u8_register(REG_HL); break; // DEC (HL)
            case 0x36: mmu_write(cpu.hl, read_u8_param()); break; // LD (HL),u8
            case 0x37: scf(); break; // SCF
            case 0x38: jump_relative(cflag(), read_i8_param()); break; // JR C,i8
            case 0x39: add_hl_u16(cpu.sp); break; // ADD HL,SP
            case 0x3A: mmu_read(cpu.hl--, &cpu.a); break; // LD A,(HL-)
            case 0x3B: dec_u16_register(&cpu.sp); break; // DEC SP
            case 0x3C: inc_u8_register(&cpu.a); break; // INC A
            case 0x3D: dec_u8_register(&cpu.a); break; // DEC A
            case 0x3E: cpu.a = read_u8_param(); break; // LD A,u8
            case 0x3F: ccf();  break; // CCF

            default: opcode_not_implemented(opcode); break;
        }   
    }
    else if(opcode < 0xC0)
    {
        uint8_t* reg = get_aligned_register(opcode);
        uint8_t operand = get_register(reg);
        
        if(opcode < 0x80)
        {
            // LD - Load instruction

            // Registers are contiguously aligned within opcodes table
            switch ( (opcode-0x40) / 0x8 )
            {
                case 0x0: cpu.b = operand; break;
                case 0x1: cpu.c = operand; break;
                case 0x2: cpu.d = operand; break;
                case 0x3: cpu.e = operand; break;
                case 0x4: cpu.h = operand; break;
                case 0x5: cpu.l = operand; break;
                case 0x6: mmu_write(cpu.hl, operand); break;
                case 0x7: cpu.a = operand; break;

                default: printf("error: invalid 8bit load opcode\n"); break;
            }
        }
        else
        {
            // 8-bit arithmetic with register A

            // Arithmetic operators are contiguously aligned within opcodes table
            switch ( (opcode-0x80) / 0x8 )
            {
                case 0x0: add_u8(operand); break;
                case 0x1: adc_u8(operand); break;
                case 0x2: sub_u8(operand); break;
                case 0x3: sbc_u8(operand); break;
                case 0x4: and_u8(operand); break;
                case 0x5: xor_u8(operand); break;
                case 0x6: or_u8(operand); break;
                case 0x7: compare_u8(operand); break;

                default: printf("error: invalid 8bit arithmetic opcode\n"); break;
            }
        }
                
    }
    else
    {
        switch (opcode)
        {
            case 0xC0: cond_ret(!zflag()); break; // RET NZ
            case 0xC1: cpu.bc = pop_u16(); break; // POP BC
            case 0xC2: cond_jump(!zflag(), read_u16_param()); break; // JP NZ,u16
            case 0xC3: jump(read_u16_param()); break; // JP u16
            case 0xC4: cond_call(!zflag(), read_u16_param()); break; // CALL NZ,u16
            case 0xC5: push_u16(cpu.bc); break; // PUSH BC
            case 0xC6: add_u8(read_u8_param()); break; // ADD A,u8
            case 0xC7: rst(0x00); break; // RST 00h
            case 0xC8: cond_ret(zflag()); break; // RET Z
            case 0xC9: ret(); break; // RET
            case 0xCA: cond_jump(zflag(), read_u16_param()); break; // JP Z,u16
            case 0xCB: /* handled before */; break; // PREFIX CB
            case 0xCC: cond_call(zflag(), read_u16_param()); break; // CALL Z,u16
            case 0xCD: call( read_u16_param() ); break; // CALL u16
            case 0xCE: adc_u8(read_u8_param()); break; // ADC A,u8
            case 0xCF: rst(0x08); break; // RST 08h
            case 0xD0: cond_ret(!cflag()); break; // RET NC
            case 0xD1: cpu.de = pop_u16(); break; // POP DE
            case 0xD2: cond_jump(!cflag(), read_u16_param()); break; // JP NC,u16
            case 0xD3: invalid(opcode); break; // invalid
            case 0xD4: cond_call(!cflag(), read_u16_param()); break; // CALL NC,u16
            case 0xD5: push_u16(cpu.de); break; // PUSH DE
            case 0xD6: sub_u8(read_u8_param()); break; // SUB A,u8
            case 0xD7: rst(0x10); break; // RST 10h
            case 0xD8: cond_ret(cflag()); break; // RET C
            case 0xD9: reti(); break; // RETI
            case 0xDA: cond_jump(cflag(), read_u16_param()); break; // JP C,u16
            case 0xDB: invalid(opcode); break; // invalid
            case 0xDC: cond_call(cflag(),read_u16_param()); break; // CALL C,u16
            case 0xDD: invalid(opcode); break; // invalid
            case 0xDE: sbc_u8(read_u8_param()); break; // SBC A,u8
            case 0xDF: rst(0x18); break; // RST 18h
            case 0xE0: mmu_write(0xFF00 + read_u8_param(), cpu.a); break; // LD (FF00+u8),A
            case 0xE1: cpu.hl = pop_u16(); break; // POP HL
            case 0xE2: mmu_write(0xFF00 + cpu.c, cpu.a); break; // LD (FF00+C),A
            case 0xE3: invalid(opcode); break; // invalid
            case 0xE4: invalid(opcode); break; // invalid
            case 0xE5: push_u16(cpu.hl); break; // PUSH HL
            case 0xE6: and_u8(read_u8_param()); break; // AND A,u8
            case 0xE7: rst(0x20); break; // RST 20h
            case 0xE8: add_sp_i8(read_i8_param()); break; // ADD SP,i8
            case 0xE9: jump(cpu.hl); break; // JP HL
            case 0xEA: mmu_write(read_u16_param(), cpu.a); break; // LD (u16),A
            case 0xEB: invalid(opcode); break; // invalid
            case 0xEC: invalid(opcode); break; // invalid
            case 0xED: invalid(opcode); break; // invalid
            case 0xEE: xor_u8(read_u8_param()); break; // XOR A,u8
            case 0xEF: rst(0x28); break; // RST 28h
            case 0xF0: mmu_read(0xFF00 + read_u8_param(), &cpu.a); break; // LD A,(FF00+u8)
            case 0xF1: pop_af(); break; // POP AF
            case 0xF2: mmu_write(0xFF00 + cflag(), cpu.a); break; // LD A,(FF00+C)
            case 0xF3: cpu.ime = 0; break; // DI
            case 0xF4: invalid(opcode); break; // invalid
            case 0xF5: push_af(); break; // PUSH AF
            case 0xF6: or_u8(read_u8_param()); break; // OR A,u8
            case 0xF7: rst(0x30); break; // RST 30h
            case 0xF8: ld_hl_sp_offset(read_i8_param()); break; // LD HL,SP+i8
            case 0xF9: cpu.sp = cpu.hl; break; // LD SP,HL
            case 0xFA: mmu_read(read_u16_param(), &cpu.a); break; // LD A,(u16)    
            case 0xFB: cpu.ime = 1; break; // EI TODO: Should be delayed by one instruction
            case 0xFC: invalid(opcode); break; // invalid
            case 0xFD: invalid(opcode); break; // invalid
            case 0xFE: compare_u8(read_u8_param()); break; // CP A,u8
            case 0xFF: restart(0x38); break; // RST 38h

            default: opcode_not_implemented(opcode); break;
        }
    }
}

void decode_exec_cb(uint8_t opcode)
{
    uint8_t* reg = get_aligned_register(opcode);

    if(opcode < 0x40)
    {
        // Rotate and Shift instructions

        // Rotate and shift operators are contiguously aligned within opcodes table
        switch ( (opcode-0x0) / 0x8 )
        {
            case 0x0: rlc(reg); break;
            case 0x1: rrc(reg); break;
            case 0x2: rl(reg); break;
            case 0x3: rr(reg); break;
            case 0x4: sla(reg); break;
            case 0x5: sra(reg); break;
            case 0x6: swap(reg); break;
            case 0x7: srl(reg); break;

            default: printf("error: invalid rotate/shift opcode\n"); break;
        }
    }
    else if(opcode < 0x80)
    {
        // BIT b,r
        uint8_t bitn = (opcode-0x40) / 0x8; // TODO: verify this
        test_bit(reg, bitn);
    }
    else if(opcode < 0xC0)
    {
        // RES b,r
        uint8_t bitn = (opcode-0x80) / 0x8;
        res_bit(reg, bitn);
    }
    else
    {
        // SET b,r
        uint8_t bitn = (opcode-0xC0) / 0x8;
        set_bit(reg, bitn);
    }
}