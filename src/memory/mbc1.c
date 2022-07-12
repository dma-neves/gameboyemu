#include "memory/mbc1.h"
#include "memory/mem.h"
#include "memory/file_loader.h"

#include <stdio.h>
#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>

uint8_t* rom = NULL;

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

    load_memory_bank_0(rom);
    load_memory_bank_n(rom + MEM_BANK_SIZE);

    return 1;
}

void mbc1_destroy()
{
    if(rom != NULL)
        free(rom);
}