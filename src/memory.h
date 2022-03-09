#ifndef _MEMORY
#define _MEMORY

#include <stdint.h>

#define MEM_SIZE 0x10000

uint16_t rom_b00();
uint16_t rom_bnn();
uint16_t vram();
uint16_t external_ram();
uint16_t wram();
uint16_t echo_ram();
uint16_t oam();
uint16_t unusable();
uint16_t io_registers();
uint16_t hram();
uint16_t ie();

int mmu_write(uint16_t address, uint8_t byte);
void mmu_read(uint16_t address, uint8_t* dest);

#endif