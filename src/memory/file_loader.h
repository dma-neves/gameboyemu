#ifndef _FILE_LOADER
#define _FILE_LOADER

#include <stdint.h>
#include <string.h>
#include <stdio.h>

int load_file(char* file, uint16_t file_offset, uint8_t* dest, uint16_t dest_offset, uint16_t max);

#endif