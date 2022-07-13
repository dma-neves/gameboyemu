#include "memory/mbc1.h"
#include "memory/mem.h"
#include "memory/file_loader.h"

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>

uint8_t* rom = NULL;
uint8_t bankn = 0; // Selected ROM bank number
uint8_t nbits_bank_select; // Number of bits required so select ROM bank number

uint8_t* get_selected_bank()
{
    uint8_t selected = (bankn == 0x00) ? 0x1 : bankn;
    uint8_t bitmask = 0xFF >> (8-nbits_bank_select);
    selected &= bitmask;

    return rom + selected*MEM_BANK_SIZE;
}

int mbc1_load_file(char* file)
{
    struct stat st;
    stat(file, &st);
    long size = st.st_size;

    if(size > MAX_CART_SIZE_MBC1)
    {
        printf("error: rom file to big for mbc1\n");
        return -1;
    }

    rom = malloc(size);

    if(load_file(file, 0, rom, 0, size) == -1)
        return -1;

    /*
        rom[0x148] gives us the rom size code
        rom size code +1 gives us the number of bits
        required to select a rom bank
    */
    nbits_bank_select = rom[0x148]+1;

    load_memory_bank_0(rom);
    load_memory_bank_n( get_selected_bank() );

    return 1;
}

void mbc1_destroy()
{
    if(rom != NULL)
        free(rom);
}

void mbc1_switch_bank(uint8_t n)
{
    bankn = n;
    load_memory_bank_n( get_selected_bank() );
}