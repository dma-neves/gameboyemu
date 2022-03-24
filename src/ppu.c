#include "ppu.h"
#include "memory.h"

#include <stdint.h>

#define TILE_BYTES 16
// #define FRAME_CYCLES 70224
#define LINE_CYCLES 456
#define NLINES 144
// #define SEARCH_CYCLES 80

uint8_t background[TILE_BYTES * 32 * 32];
uint8_t window[TILE_BYTES * 32 * 32];

uint16_t line_cycle_counter = 0;

// Scan line buffering

void ppu_new_frame()
{
    (*ly) = 0;
    line_cycle_counter = 0;
}

void draw_tiles()
{

}

void draw_sprites()
{

}

void draw_line()
{
    draw_tiles();
    draw_sprites();
}

void update_ppu(uint8_t cycles)
{
    line_cycle_counter += cycles;
    if(line_cycle_counter >= LINE_CYCLES)
    {
        line_cycle_counter -= LINE_CYCLES;

        if(*ly < NLINES)
        {
            draw_line();
        }

        (*ly)++;
    }
}