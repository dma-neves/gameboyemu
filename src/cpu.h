#ifndef _CPU
#define _CPU

#include <stdint.h>

struct cpu
{
    union
    { 
        uint16_t af;
        struct { uint8_t a; uint8_t f; };
    };
    union
    { 
        uint16_t bc;
        struct { uint8_t b; uint8_t c; };
    };
    union
    { 
        uint16_t de;
        struct { uint8_t d; uint8_t e; };
    };
    union
    { 
        uint16_t hl;
        struct { uint8_t h; uint8_t l; };
    };

    uint16_t sp;
    uint16_t pc;
};

void tick();

#endif