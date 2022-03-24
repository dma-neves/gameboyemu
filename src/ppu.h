#ifndef _PPU
#define _PPU

#include <stdint.h>

void ppu_new_frame();
void update_ppu(uint8_t cycles);

#endif