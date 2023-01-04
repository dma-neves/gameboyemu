#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <time.h>

#include "memory/mem.h"
#include "cpu.h"
#include "timer.h"
#include "video/ppu.h"
#include "video/ui.h"
#include "controls.h"

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

void reset_system()
{
    reset_memory();
    reset_cpu();
}

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("usage: ./emu bootrom.gb rom.gb\n");
        return 0;
    }

    reset_system();

    #ifndef DEBUG
        printf("Loading bootrom\n");
    #endif

    if(load_bootrom(argv[1]) == -1)
        exit(1);

    #ifndef DEBUG
        printf("Loading rom\n");
    #endif

    if(load_rom(argv[2]) == -1)
        exit(1);

    #ifndef DEBUG
        printf("Initializing UI\n");
    #endif

    init_ui();

    #ifndef DEBUG
        printf("Starting boot sequence\n");
    #endif

    long cycles = 0;
    double clk = 0;
    double dt;
    double timer_60;
    double timer_1;
    int frames = 0;
    while(running)
    {
        handle_events();

        dt = clk;
        clk = (double)clock()/CLOCKS_PER_SEC;
        dt = clk - dt;
        timer_60 += dt;
        timer_1 += dt;

        if(cycles < CYCLE_THRESHOLD)
        {
            uint8_t step_cycles = step();
            update_timers(step_cycles);
            update_ppu(step_cycles);
            update_dma_transfer(step_cycles);
            cycles += step_cycles;
        }

        if(timer_60 >= 1.f/59.73f)
        {
            // Update display at a ~ 60Hz frequency and reset cycle counter
            cycles = 0;
            timer_60 = 0;
            render_ui();
            frames++;
        }

        if(timer_1 >= 1.f)
        {
            timer_1 = 0;
            printf("fps: %d\n", frames);
            frames = 0;
        }
    }

    memory_destroy();
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

            else
                controls_pressed(event.key.keysym.sym);

            break;

        case SDL_KEYUP:
            controls_released(event.key.keysym.sym);
        
        default:
            break;
    }
}