#include "memory/file_loader.h"

int load_file(char* file, uint16_t file_offset, uint8_t* dest, uint16_t dest_offset, uint16_t max)
{
    FILE *fp;
    int c, i;

    fp = fopen(file, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open input file\n");
        return -1;
    }

    if(file_offset)
        fseek(fp, file_offset, SEEK_SET);

    for (i = 0; i <= max && (c = getc(fp)) != EOF; i++)
        dest[dest_offset + i] = (uint8_t)c;

    // if(c != EOF)
    // {
    //     fprintf(stderr, "error: file to big (max size: %d bytes)\n", max);
    //     fclose(fp);
    //     return -1;
    // }

    return fclose(fp);
}