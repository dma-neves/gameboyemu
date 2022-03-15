#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "memory.h"
#include "cpu.h"

#define WIDHT 160
#define HEIGHT 144
#define SCALE 3

/*
    gameboy's cpu runs at 4194304Hz <=> 
    (4194304) * (1/60) = 69905 cycles every 1/60 seconds
*/
#define CYCLE_THRESHOLD 69905

/* flags */
uint8_t running = 1;

/* SDL */
SDL_Window* window;
SDL_Renderer* renderer;

void render();
void handle_events();
void update();

void init_sdl()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("gameboy", SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED, WIDHT*SCALE, HEIGHT*SCALE, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    SDL_RenderSetLogicalSize(renderer, WIDHT, HEIGHT);
}

int load_rom(char* file)
{
    FILE *fp;
    int c, i, max = VRAM;

    fp = fopen(file, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open input file\n");
        return -1;
    }

    for (i = 0; i <= max && (c = getc(fp)) != EOF; i++)
        mmu_write(ROM_B00 + i, (uint8_t)c);

    if(c != EOF)
    {
        fprintf(stderr, "error: rom to big (Max size: %d bytes)\n", max);
        fclose(fp);
        return -1;
    }

    return fclose(fp);
}

void reset_system()
{
    reset_memory();
}

int main(int argc, char** argv)
{
    if(argc < 2)
    {
        printf("usage: ./emu rom.gb\n");
        return 0;
    }

    reset_system();
    load_rom(argv[1]);
    // init_sdl();

    long cycles = 0;
    double clk = 0;
    double dt;
    double timer_60;
    while(running)
    {
        handle_events();

        dt = clk;
        clk = (double)clock()/CLOCKS_PER_SEC;
        dt = clk - dt;
        timer_60 += dt;

        if(cycles < CYCLE_THRESHOLD)
            cycles += step();

        if(timer_60 >= 1.f/60.f)
        {
            // Update display at a 60Hz frequency and reset cycle counter
            cycles = 0;
            timer_60 = 0;
            // render();
        }
    }
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
    //SDL_RenderDrawLine(renderer, 0, 50, WIDHT-2, 60);
    SDL_RenderPresent(renderer);
}

void handle_events()
{
    SDL_Event event;
    SDL_PollEvent(&event);
    switch (event.type)
    {
        case SDL_QUIT:
            running = 0;
            break;

        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE)
                running = 0;
            break;
        
        default:
            break;
    }
}