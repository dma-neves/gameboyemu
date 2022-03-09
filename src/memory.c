#include "memory.h"

uint8_t memory[MEM_SIZE];

uint16_t rom_b00()      { return 0x0000; }
uint16_t rom_bnn()      { return 0x4000; }
uint16_t vram()         { return 0x8000; }
uint16_t external_ram() { return 0xA000; }
uint16_t wram()         { return 0xC000; }
uint16_t echo_ram()     { return 0xE000; }
uint16_t oam()          { return 0xFE00; }
uint16_t unusable()     { return 0xFEA0; }
uint16_t io_registers() { return 0xFF00; }
uint16_t hram()         { return 0xFF80; }
uint16_t ie()           { return 0xFFFF; }

int mmu_write(uint16_t address, uint8_t byte)
{
    memory[address] = byte;
}

void mmu_read(uint16_t address, uint8_t* dest)
{
    *dest = memory[address];
}
