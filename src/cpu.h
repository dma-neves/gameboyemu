#ifndef _CPU
#define _CPU

#include <stdint.h>

#define REG_HL NULL

#define VBLANK_INT 0x1
#define LCD_STAT_INT 0x2
#define TIMER_INT 0x4
#define SERIAL_INT 0x8
#define JOYPAD_INT 0x10

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
    uint8_t ime;
    uint8_t hlt;
};

void reset_cpu();
uint8_t step();
void set_debug(uint8_t value);
void request_interrupt(uint8_t source);

#endif