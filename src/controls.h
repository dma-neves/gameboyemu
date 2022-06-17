#ifndef CONTROLS
#define CONTROL

#include <SDL2/SDL.h>

void controls_pressed(SDL_Keycode key);
void controls_released(SDL_Keycode key);

void write_joyp_reg(uint8_t value);

#endif