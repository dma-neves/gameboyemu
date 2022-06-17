#include "ppu/ui.h"
#include "mem.h"

#include <SDL2/SDL.h>

#define WIDTH 160
#define HEIGHT 144
#define SCALE 3

uint8_t colors[4][3] = {

    {255,255,255},
    {170,170,170},
    {85,85,85},
    {0,0,0}
};

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture *lcd_texture;
uint8_t pixels[WIDTH * HEIGHT * 4] = {0};

void init_sdl()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("gameboy", SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED, WIDTH*SCALE, HEIGHT*SCALE, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, WIDTH, HEIGHT);

    lcd_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
    if (lcd_texture == NULL)
    {
        SDL_Log("Unable to create texture: %s", SDL_GetError());
        return;
    }
}

void init_ui()
{
    init_sdl();
}

void update_texture()
{
    int texture_pitch = 0;
    void* texture_pixels = NULL;

    if(SDL_LockTexture(lcd_texture, NULL, &texture_pixels, &texture_pitch) != 0)
        SDL_Log("Unable to lock texture: %s", SDL_GetError());
    else
        memcpy(texture_pixels, pixels, texture_pitch * HEIGHT);

    SDL_UnlockTexture(lcd_texture);
}

void render_ui()
{
    update_texture();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, lcd_texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void set_pixel(uint8_t x, uint8_t y, uint8_t color)
{
    for(int i = 0; i < 3; i++)
        pixels[y*WIDTH*4 + x*4 + i] = colors[color][i];
}