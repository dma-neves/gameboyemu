#ifndef _MEMORY
#define _MEMORY

#include <stdint.h>

#define MEM_SIZE 0x10000

#define ROM_B00_ADR 0x0000
#define ROM_BNN_ADR 0x4000
#define VRAM_ADR 0x8000
#define EXTERNAL_RAM_ADR 0xA000
#define WRAM_ADR 0xC000
#define ECHO_RAM_ADR 0xE000
#define OAM_ADR 0xFE00
#define UNUSABLE_ADR 0xFEA0
#define IO_REGISTERS_ADR 0xFF00
#define DIV_ADR 0xFF04
#define TIMA_ADR 0xFF05
#define TMA_ADR 0xFF06
#define TAC_ADR 0xFF07
#define LY_ADR 0xFF44
#define WY_ADR 0xFF4A
#define WX_ADR 0xFF4B
#define BOOT_OFF_ADR 0xFF50
#define HRAM_ADR 0xFF80
#define IE_ADR 0xFFFF

extern uint8_t* tdiv;
extern uint8_t* tima;
extern uint8_t* tma;
extern uint8_t* tac;
extern uint8_t* ly;

void reset_memory();
int mmu_write(uint16_t address, uint8_t byte);
void mmu_read(uint16_t address, uint8_t* dest);

#endif