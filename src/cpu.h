#ifndef _CPU
#define _CPU

#include <stdint.h>

#define REG_HL NULL

struct cpu
{
    union
    { 
        uint16_t af;
        struct { uint8_t f; uint8_t a; };
    };
    union
    { 
        uint16_t bc;
        struct { uint8_t c; uint8_t b; };
    };
    union
    { 
        uint16_t de;
        struct { uint8_t e; uint8_t d; };
    };
    union
    { 
        uint16_t hl;
        struct { uint8_t l; uint8_t h; };
    };

    uint16_t sp;
    uint16_t pc;
};

uint8_t step();

#endif