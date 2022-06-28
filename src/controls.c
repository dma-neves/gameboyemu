#include "controls.h"
#include "mem.h"

#include <stdio.h>

uint8_t button[8] = {0};

#define UP 0
#define LEFT 1
#define DOWN 2
#define RIGHT 3
#define A 4
#define B 5
#define START 6
#define SELECT 7

static int get_button_index(SDL_Keycode key)
{
    switch (key)
    {
        case SDLK_w: return UP;
        case SDLK_a: return LEFT;
        case SDLK_s: return DOWN;
        case SDLK_d: return RIGHT;
        case SDLK_o: return B;
        case SDLK_p: return A;
        case SDLK_k: return SELECT;
        case SDLK_l: return START;
        default: break;    
    }

    return -1;
}

void controls_pressed(SDL_Keycode key)
{
    int ki = get_button_index(key);
    if(ki != -1)
    {
        uint8_t action = (((*joyp) >> 0x5) & 0x1) == 0x0;
        uint8_t direction = (((*joyp) >> 0x4) & 0x1) == 0x0;

        // Joypad interrupt is requested when button is pressed 
        // (provided that the action/direction buttons are enabled by bit 5/4, respectively)
        if( button[ki] == 0 && ( (action && ki >= A) || (direction && ki <= RIGHT) ) )
            (*intf) |= (0x1 << 0x4);

        button[ki] = 1;
    }
}

void controls_released(SDL_Keycode key)
{
    int ki = get_button_index(key);
    if(ki != -1)
        button[ki] = 0;
}

static void set_joyp_bit(uint8_t bitn, uint8_t value)
{
    uint8_t b = value ? 1 : 0;

    (*joyp) &= ~(0x1 << bitn);  // clear bit
    (*joyp) |= (b << bitn);     // or bit with value
}

void write_joyp_reg(uint8_t value)
{
    uint8_t action = ((value >> 0x5) & 0x1) == 0x0;
    uint8_t direction = ((value >> 0x4) & 0x1) == 0x0;

    *joyp = value;

    if(action && direction)
    {
        set_joyp_bit(3, !(button[START] || button[DOWN]));
        set_joyp_bit(2, !(button[SELECT] || button[UP]));   
        set_joyp_bit(1, !(button[B] || button[LEFT]));   
        set_joyp_bit(0, !(button[A] || button[RIGHT]));
    }
    else if(action)
    {
        set_joyp_bit(3, !button[START]);
        set_joyp_bit(2, !button[SELECT]);   
        set_joyp_bit(1, !button[B]);   
        set_joyp_bit(0, !button[A]);  
    }

    else if(direction)
    {
        set_joyp_bit(3, !button[DOWN]);
        set_joyp_bit(2, !button[UP]);   
        set_joyp_bit(1, !button[LEFT]);   
        set_joyp_bit(0, !button[RIGHT]);
    }
}
