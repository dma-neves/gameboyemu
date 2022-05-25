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
#define IF_ADR 0xFF0F
#define LCDC_ADR 0xFF40
#define LCDC_STAT_ADR 0xFF41
#define SCY_ADR 0xFF42
#define SCX_ADR 0xFF43
#define LY_ADR 0xFF44
#define LYC_ADR 0xFF45
#define BGP_ADR 0xFF47
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
extern uint8_t* lyc;
extern uint8_t* lcdc;
extern uint8_t* lcdc_stat;
extern uint8_t* scx;
extern uint8_t* scy;
extern uint8_t* windowx;
extern uint8_t* windowy;
extern uint8_t* bgp;
extern uint8_t* ie;
extern uint8_t* intf;


void reset_memory();
int mmu_write(uint16_t address, uint8_t byte);
void mmu_read(uint16_t address, uint8_t* dest);

void mmu_write_u16(uint16_t address, uint16_t byte);
void mmu_read_u16(uint16_t address, uint16_t* dest);

#endif