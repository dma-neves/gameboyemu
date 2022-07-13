#include "video/ppu.h"
#include "memory/mem.h"
#include "video/ui.h"
#include "video/obj_list.h"
#include "cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define LINE_CYCLES 456
#define NLINES 144
#define NROWS 160
#define VBLANK 10
#define OAM_SEARCH_CYCLES 80
#define MEM_READ_CYCLES 172 // TODO: Approximation (168 to 291 dots (40 to 60 µs) depending on sprite count)

#define VIEW_PORT_WIDTH NROWS
#define VIEW_PORT_HEIGH NLINES
#define TILE_MAP_WIDTH 32
#define TILE_MAP_PIXEL_WIDTH 256
#define TILE_MAP_PIXEL_HEIGHT 256

#define TILE_BYTES 16
#define TILE_WIDTH 8
#define TILE_HEIGHT 8

#define N_SPRITES 40 // Number of sprites
#define MAX_SPL 10 // Max number of sprites drawn per line

#define TRANSPARENT 0

static uint16_t line_cycle_counter = 0;

static uint8_t mode;
static uint8_t stat_int_line = 0x0; // stat interrupt line

uint8_t obj_filled_pixels[NROWS] = {0}; // Binary array indicating which pixels have already been drawn (by sprites) in the current line
uint8_t bgw_filled_pixels[NROWS] = {0}; // Binary array indicating which pixels have already been drawn (by background/window) in the current line

/* -------------- LCD Color -------------- */

typedef struct LCDColor
{
    uint8_t index;
    uint8_t color;
} LCDColor;

/* -------------- LCD Control -------------- */

uint8_t bg_window_enable()      { return (*lcdc & 0x01) != 0; }
uint8_t obj_enable()            { return (*lcdc & 0x02) != 0; }
uint8_t obj_size()              { return (*lcdc & 0x04) != 0; }
uint8_t bg_tile_map_area()      { return (*lcdc & 0x08) != 0; }
uint8_t tile_data_area()        { return (*lcdc & 0x10) != 0; }
uint8_t window_enable()         { return (*lcdc & 0x20) != 0; }
uint8_t window_tile_map_area()  { return (*lcdc & 0x40) != 0; }
uint8_t lcd_enable()            { return (*lcdc & 0x80) != 0; }

static void set_stat_bit(uint8_t bitn, uint8_t value)
{
    uint8_t b = value ? 1 : 0;

    (*lcdc_stat) &= ~(0x1 << bitn);  // clear bit
    (*lcdc_stat) |= (b << bitn);     // or bit with value
}

uint8_t get_stat_int_line()
{
    if((*lcdc_stat) & (0x1 << 0x6) && (*lcdc_stat) & (0x1 << 0x2)) // ly == lyc
        return 1;

    if((*lcdc_stat) & (0x1 << 0x5) && mode == 2) // OAM search
        return 1;

    if((*lcdc_stat) & (0x1 << 0x4) && mode == 1) // VBlank
        return 1;

    if((*lcdc_stat) & (0x1 << 0x3) && mode == 0) // HBlank
        return 1;

    return 0;
}

void set_stat()
{
    set_stat_bit(0x2, (*ly == *lyc));

    if(*ly >= NLINES)
        mode = 1;
    else if(line_cycle_counter < OAM_SEARCH_CYCLES)
        mode = 2;
    else if(line_cycle_counter < OAM_SEARCH_CYCLES + MEM_READ_CYCLES)
        mode = 3;
    else
        mode = 0;

    // Set lcdc_stat bits 0 and 1 equal to bits 0 and 1 from mode
    set_stat_bit(0x0, mode & 0x1);
    set_stat_bit(0x1, mode & 0x2);

    // Enable interrupt if there is a rising edge in the stat interrupt line
    uint8_t new_stat_int_line = get_stat_int_line();
    if(new_stat_int_line == 1 && stat_int_line == 0)
        request_interrupt(LCD_STAT_INT);
    stat_int_line = new_stat_int_line;
}

static void clear_filled_pixels()
{
    memset(obj_filled_pixels, 0, sizeof(uint8_t)*NROWS);
    memset(bgw_filled_pixels, 0, sizeof(uint8_t)*NROWS);
}

static LCDColor get_tile_color(uint8_t tile_row_0, uint8_t tile_row_1, uint8_t tile_x, uint8_t palette)
{
    // bit 7 represents the leftmost pixel, and bit 0 the rightmost
    uint8_t color_index_bit_0, color_index_bit_1, color_index;
    color_index_bit_0 = (tile_row_0 >> (7-tile_x)) & 0x1;
    color_index_bit_1 = (tile_row_1 >> (7-tile_x)) & 0x1;
    color_index = color_index_bit_0 | (color_index_bit_1 << 0x1);

    /* Get color*/

    // Color depends on palette
    uint8_t color_bit_0, color_bit_1, color;
    color_bit_0 = ( palette >> (color_index*2) ) & 0x1;
    color_bit_1 = ( palette >> (color_index*2+1) ) & 0x1;
    color = color_bit_0 | (color_bit_1 << 0x1);

    return (LCDColor){color_index, color};
}

void draw_tiles(uint8_t scroll_x, uint8_t scroll_y, uint8_t offset_x, uint8_t offset_y, uint8_t tile_map_area_flag)
{
    // Tile map address depends on bg_tile_map_area or window_tile_map_area flag
    uint16_t tile_map_base_address = tile_map_area_flag ? 0x9C00 : 0x9800;

    // Calculate pixel position taking into account the scx and scy offsets (viewport's position)
    uint16_t viewport_y = ( (*ly) + scroll_y ) % TILE_MAP_PIXEL_HEIGHT;
    
    // TODO: Instead of iterating over every pixel, load chunks of 8 pixels
    for(uint16_t i = 0; i < VIEW_PORT_WIDTH; i++)
    {
        int16_t lcd_x = i+offset_x;
        int16_t lcd_y = (*ly)+offset_y;

        if(lcd_x < 0 || lcd_x >= VIEW_PORT_WIDTH || lcd_y < 0 || lcd_y >= VIEW_PORT_HEIGH)
            continue;

        uint16_t viewport_x = (i + scroll_x) % TILE_MAP_PIXEL_WIDTH;

        /* Select tile in tile map */

        uint8_t tile_map_x = viewport_x / TILE_WIDTH;
        uint8_t tile_map_y = viewport_y / TILE_HEIGHT;

        /* Select pixel whthin tile */

        uint8_t tile_x = viewport_x % TILE_WIDTH;
        uint8_t tile_y = viewport_y % TILE_HEIGHT;

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
        mmu_read(tile_data_address+1, &tile_row_0);
        mmu_read(tile_data_address+0, &tile_row_1);

        uint8_t color = get_tile_color(tile_row_0, tile_row_1, tile_x, *bgp).color;
        set_pixel(lcd_x, lcd_y, color);
        if(color != 0)
            bgw_filled_pixels[lcd_x] = 1;
    }
}

void draw_bg_window_tiles()
{
    // Draw background
    draw_tiles(*scx, *scy, 0, 0, bg_tile_map_area());

    // Draw window
    if(window_enable())
        draw_tiles(0, 0, (*windowx)-7, *windowy, window_tile_map_area());
}

static void select_sprites()
{
    obj_list_reset();

    uint8_t obj_height = obj_size() ? 16 : 8;
    uint16_t oam_adr = 0xFE00;
    // Iterate over all 40 sprites that might be rendered on this line without exceeding the max 10 sprites per line
    for(uint8_t spriten = 0; spriten < N_SPRITES && obj_list_size() < MAX_SPL; spriten++)
    {
        obj object;

        mmu_read(oam_adr++, &object.pos_y);
        mmu_read(oam_adr++, &object.pos_x);
        mmu_read(oam_adr++, &object.tile_number);
        mmu_read(oam_adr++, &object.flags);

        object.pos_y -= 16; // Pos y offsetted 16 pixels in OAM
        object.pos_x -= 8;  // Pos x offsetted 8 pixels in OAM

        if(*ly >= object.pos_y && *ly < object.pos_y + obj_height) // Check if a row of pixels of the sprite lays on line ly
            obj_list_add(object);
    }
}

void draw_sprites()
{
    /* Get 10 sprites to draw and order them in a list (draw/select priority) */
    select_sprites();

    /* Draw the pixels from the selected sprites */

    uint8_t obj_height = obj_size() ? TILE_HEIGHT*2 : TILE_HEIGHT;
    uint8_t obj_width = TILE_WIDTH;

    while(obj_list_size() > 0)
    {
        obj object = obj_list_remove(); // Get object with highest priority
    
        uint8_t pallet_number = object.flags & 0x10;
        uint8_t x_flip        = object.flags & 0x20;
        uint8_t y_flip        = object.flags & 0x40;
        uint8_t bg_over_obj   = object.flags & 0x80;

        uint8_t x_start = MAX(object.pos_x, 0);
        uint8_t x_end = MIN(object.pos_x + TILE_WIDTH, NROWS);

        /* Fetch pixels from tile data */

        // TODO: x,y flips

        uint16_t tile_data_address;

        uint8_t tile_y = y_flip ? obj_height - ((*ly) - object.pos_y) : (*ly) - object.pos_y; // row within tile
        if(tile_y < TILE_HEIGHT)
            tile_data_address = (0x8000 + object.tile_number*TILE_BYTES) + 2*tile_y;
        else
            // Sprite uses 2 tiles. Use second tile
            tile_data_address = (0x8000 + (object.tile_number + 1)*TILE_BYTES) + 2*(tile_y-TILE_HEIGHT);

        // Fetch the 2 bytes (2 bytes required since each pixel is represented by a 2 bit color) 
        // relative to the correct row of pixels in the tile
        uint8_t tile_row_0, tile_row_1;
        mmu_read(tile_data_address+1, &tile_row_0);
        mmu_read(tile_data_address+0, &tile_row_1);

        /* Draw pixels */

        for(uint16_t x = x_start; x < x_end; x++)
        {
            if(!obj_filled_pixels[x])
            {
                uint8_t tile_x = x_flip ? obj_width - (x - object.pos_x) :  x - object.pos_x; // X coordinate within tile row
                uint8_t palette = pallet_number ? *obp1 : *obp0; // Sellect pallete based on flag
                LCDColor lcdcolor = get_tile_color(tile_row_0, tile_row_1, tile_x, palette);

                if(lcdcolor.index != TRANSPARENT)
                {
                    obj_filled_pixels[x] = 1;
                    if(!(bg_over_obj && bgw_filled_pixels[x]))
                        set_pixel(x, *ly, lcdcolor.color);
                }
            }
        }
    }
}

void draw_line()
{
    clear_filled_pixels();

    if(bg_window_enable())
        draw_bg_window_tiles();

    if(obj_enable())
        draw_sprites();
}

void update_ppu(uint8_t cycles)
{
    if(!lcd_enable())
    {
        free_vram_oam();
        *ly = 0;
        set_stat_bit(0x0, 0x0);
        set_stat_bit(0x1, 0x0);
        return;
    }

    line_cycle_counter += cycles;
    if(line_cycle_counter >= LINE_CYCLES)
    {
        line_cycle_counter -= LINE_CYCLES;

        if(*ly < NLINES)
            draw_line();
        
        if(*ly >= NLINES+VBLANK)
        {
            (*ly) = 0;
            line_cycle_counter = 0;
        }
        else
            (*ly)++;
    }

    // Set VBLANK interrupt flag
    if(*ly == NLINES)
        request_interrupt(VBLANK_INT);

    set_stat();
}