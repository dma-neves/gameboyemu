#include "ppu.h"
#include "memory.h"
#include "ui.h"

#include <stdint.h>
#include <stdio.h>

#define TILE_BYTES 16
#define LINE_CYCLES 456
#define NLINES 144

#define VIEW_PORT_WIDTH 160
#define TILE_MAP_WIDTH 32
#define TILE_MAP_PIXEL_WIDTH 256
#define TILE_MAP_PIXEL_HEIGHT 256

#define TILE_WIDTH 8
#define TILE_HEIGHT 8

uint8_t background[TILE_BYTES * 32 * 32];
uint8_t window[TILE_BYTES * 32 * 32];

uint16_t line_cycle_counter = 0;

uint8_t tile_data_area() { return (*lcdc & 0x10) != 0; }
uint8_t bg_tile_map_area() { return (*lcdc & 0x8) != 0; }
uint8_t bg_window_enable() { return (*lcdc & 0x1) != 0; }

// TODO: possibly remove
void ppu_new_frame()
{
    (*ly) = 0;
    line_cycle_counter = 0;
}


void draw_tiles()
{
    if(!bg_window_enable())
        return;

    // Tile map address depends on bg_tile_map_area flag
    uint16_t tile_map_base_address = bg_tile_map_area() ? 0x9C00 : 0x9800;

    // Calculate pixel position taking into account the scx and scy offsets (viewport's position)
    uint16_t pixel_y = ( (*ly) + (*scy) ) % TILE_MAP_PIXEL_HEIGHT;
    
    for(uint16_t i = 0; i < VIEW_PORT_WIDTH; i++)
    {
        uint16_t pixel_x = (i + (*scx) ) % TILE_MAP_PIXEL_WIDTH;

        /* Select tile in tile map */

        uint8_t tile_map_x = pixel_x / TILE_WIDTH;
        uint8_t tile_map_y = pixel_y / TILE_HEIGHT;

        /* Select pixel whthin tile */

        uint8_t tile_x = pixel_x % TILE_WIDTH;
        uint8_t tile_y = pixel_y % TILE_HEIGHT;

        /* Fetch the tile number from the tile map */

        uint16_t tile_map_address = tile_map_base_address + tile_map_y*TILE_MAP_WIDTH + tile_map_x;
        uint8_t tile_number;
        mmu_read(tile_map_address, &tile_number);

        /* Fetch pixels from tile data */

        uint16_t tile_data_address;

        // Tile data is addressed differently depending on tile_data_area flag
        if( tile_data_area() )
        {
            // tile_data_area == 1 => unsigned addressing & 0x8000 base address
            tile_data_address = 0x8000 + tile_number*TILE_BYTES;
        }
        else
        {
            // tile_data_area == 0 => signed addressing & 0x9000 base address
            tile_data_address = 0x9000 + ( (int8_t)tile_number )*TILE_BYTES;
        }

        // Fetch the 2 bytes (2 bytes required since each pixel is represented by a 2 bit color) 
        // relative to the correct row of pixels in the tile
        tile_data_address += 2*tile_y;
        uint8_t tile_row_0, tile_row_1;
        mmu_read(tile_data_address, &tile_row_0);
        mmu_read(tile_data_address, &tile_row_1);

        /* Get color index bits */

        // bit 7 represents the leftmost pixel, and bit 0 the rightmost
        uint8_t color_index_bit_0, color_index_bit_1, color_index;
        color_index_bit_0 = (tile_row_0 >> (7-tile_x)) & 0x1;
        color_index_bit_1 = (tile_row_1 >> (7-tile_x)) & 0x1;
        color_index = color_index_bit_0 | (color_index_bit_1 << 0x1);

        /* Get color and set pixel on lcd */

        // Color depends on palette assigned by BGP 0xFF47
        uint8_t color_bit_0, color_bit_1, color;
        color_bit_0 = ( (*bgp) >> color_index*2) & 0x1;
        color_bit_1 = ( (*bgp) >> color_index*2+1) & 0x1;
        color = color_bit_0 | (color_bit_1 << 0x1);

        set_pixel(i, *ly, color);
    }
}

void draw_sprites()
{
    // TODO: later

    // Color depends on pallete assigned by OBP0 0xFF48 and OBP1 0xFF49
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
        
        if(*ly > 153)
        {
            (*ly) = 0;
            line_cycle_counter = 0;
        }
        else
            (*ly)++;
    }
}