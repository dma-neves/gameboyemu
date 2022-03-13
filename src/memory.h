#ifndef _MEMORY
#define _MEMORY

#include <stdint.h>

#define MEM_SIZE 0x10000

#define ROM_B00 0x0000
#define ROM_BNN 0x4000
#define VRAM 0x8000
#define EXTERNAL_RAM 0xA000
#define WRAM 0xC000
#define ECHO_RAM 0xE000
#define OAM 0xFE00
#define UNUSABLE 0xFEA0
#define IO_REGISTERS 0xFF00
#define BOOT_OFF 0xFF50
#define HRAM 0xFF80
#define IE 0xFFFF

void reset_memory();
int mmu_write(uint16_t address, uint8_t byte);
void mmu_read(uint16_t address, uint8_t* dest);

#endif