/*

Limitations:
    - No support for >= 1 Mbyte roms (special wiring)
    - No support for MBC1M
    - Currently no support for RAM banking
*/

#ifndef _MBC1
#define _MBC1

#include <stdint.h>

#define MAX_CART_SIZE_MBC1 0x80000

int mbc1_load_file(char* file);
void mbc1_destroy();

void mbc1_switch_bank(uint8_t n);

#endif