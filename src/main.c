#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "memory.h"
#include "cpu.h"
#include "timer.h"
#include "ppu.h"
#include "ui.h"

/*
    gameboy's cpu runs at 4194304Hz <=> 
    (4194304) * (1/59.73) = 70221 cycles every ~ 1/60 seconds
*/
#define CYCLE_THRESHOLD 70221

/* flags */
uint8_t running = 1;

void render();
void handle_events();
void update();

int load_rom(char* file)
{
    FILE *fp;
    int c, i, max = VRAM_ADR;

    fp = fopen(file, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "error: cannot open input file\n");
        return -1;
    }

    for (i = 0; i <= max && (c = getc(fp)) != EOF; i++)
        mmu_write(ROM_B00_ADR + i, (uint8_t)c);

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
    init_ui();

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
        {
            uint8_t step_cycles = step();
            update_timers(step_cycles);
            update_ppu(step_cycles);
            cycles += step_cycles;
        }

        if(timer_60 >= 1.f/59.73f)
        {
            // Update display at a ~ 60Hz frequency and reset cycle counter
            ppu_new_frame();
            cycles = 0;
            timer_60 = 0;
            render_ui();
        }
    }
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