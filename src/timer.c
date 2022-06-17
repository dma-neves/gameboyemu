#include "timer.h"
#include "mem.h"

#include <stdint.h>

#define DIV_CYCLES 256

uint16_t div_counter = 0;
uint16_t tima_counter = 0;

// TODO: Timer Operation (https://hacktixme.ga/GBEDG/timers/)

uint16_t get_tima_cycles()
{
    switch (*tac & 0x3)
    {
        case 0: return 1024;
        case 1: return 16;
        case 2: return 64;
        case 3: return 256;

        default: return 0;
    }
}

uint8_t tima_enabled()
{
    return (*tac & 0x4) != 0;
}

void inc_tima()
{
    if(*tima == 0xFF)
    {
        *tima = *tma;
        (*intf) |= (0x1 << 0x2);
    }
    else
        (*tima)++;
}

void update_timers(uint8_t ncycles)
{
    div_counter += ncycles;
    if(div_counter >= DIV_CYCLES)
    {
        div_counter -= DIV_CYCLES;
        (*tdiv)++;
    }

    if(!tima_enabled())
        return;

    tima_counter += ncycles;
    uint16_t tima_cycles = get_tima_cycles();
    if(tima_counter >= tima_cycles)
    {
        tima_counter -= tima_cycles;
        inc_tima();
    }
}

void reset_div()
{
    *tdiv = 0;
}