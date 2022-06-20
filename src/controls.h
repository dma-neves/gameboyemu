#ifndef CONTROLS
#define CONTROL

#include <SDL2/SDL.h>
#include <stdint.h>

void controls_pressed(SDL_Keycode key);
void controls_released(SDL_Keycode key);

uint8_t joyp_interrupt();
void write_joyp_reg(uint8_t value);

#endif