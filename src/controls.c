#include "controls.h"
#include "mem.h"

#include <stdio.h>

uint8_t keys[8] = {0};

#define UP 0
#define LEFT 1
#define DOWN 2
#define RIGHT 3
#define A 4
#define B 5
#define START 6
#define SELECT 7

static int get_key_index(SDL_Keycode key)
{
    switch (key)
    {
        case SDLK_w: return UP;
        case SDLK_a: return LEFT;
        case SDLK_s: return DOWN;
        case SDLK_d: return RIGHT;
        case SDLK_j: return A;
        case SDLK_k: return B;
        case SDLK_n: return START;
        case SDLK_m: return SELECT;
        default: break;    
    }

    return -1;
}

void controls_pressed(SDL_Keycode key)
{
    int ki = get_key_index(key);
    if(ki != -1)
    {
        uint8_t action = (((*joyp) >> 0x5) & 0x1) == 0x0;
        uint8_t direction = (((*joyp) >> 0x4) & 0x1) == 0x0;

        if( (action && ki >= A) || (direction && ki <= RIGHT) )
            ; // Interrupt


        keys[get_key_index(key)] = 1;
    }
}

void controls_released(SDL_Keycode key)
{
    int ki = get_key_index(key);
    if(ki != -1)
        keys[get_key_index(key)] = 0;
}

static void set_joyp_bit(uint8_t bitn, uint8_t value)
{
    uint8_t value_bit = value ? 1 : 0;

    (*joyp) &= ~(0x1 << bitn);
    (*joyp) |= (value_bit << bitn);
}

void write_joyp_reg(uint8_t value)
{
    uint8_t action = (((*joyp) >> 0x5) & 0x1) == 0x0;
    uint8_t direction = (((*joyp) >> 0x4) & 0x1) == 0x0;

    if(action && direction)
    {
        set_joyp_bit(3, !(keys[START] || keys[DOWN]));
        set_joyp_bit(2, !(keys[SELECT] || keys[UP]));   
        set_joyp_bit(1, !(keys[B] || keys[LEFT]));   
        set_joyp_bit(0, !(keys[A] || keys[RIGHT]));
    }
    else if(action)
    {
        set_joyp_bit(3, !keys[START]);
        set_joyp_bit(2, !keys[SELECT]);   
        set_joyp_bit(1, !keys[B]);   
        set_joyp_bit(0, !keys[A]);    
    }

    else if(direction)
    {
        set_joyp_bit(3, !keys[DOWN]);
        set_joyp_bit(2, !keys[UP]);   
        set_joyp_bit(1, !keys[LEFT]);   
        set_joyp_bit(0, !keys[RIGHT]);   
    }
}