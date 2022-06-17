#ifndef _PPU
#define _PPU

#include <stdint.h>

void ppu_new_frame();
uint8_t lcdc_stat_interrupt();
void update_ppu(uint8_t cycles);

#endif