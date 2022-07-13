#ifndef _FILE_LOADER
#define _FILE_LOADER

#include <stdint.h>
#include <string.h>
#include <stdio.h>

int load_file(char* file, long file_offset, uint8_t* dest, long dest_offset, long max);

#endif