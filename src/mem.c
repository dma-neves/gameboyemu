#include "mem.h"
#include "timer.h"

#include <string.h>
#include <stdio.h>

#include "cpu.h"
#include "controls.h"

uint8_t memory[MEM_SIZE];

uint8_t* tdiv = memory + DIV_ADR;
uint8_t* tima = memory + TIMA_ADR;
uint8_t* tma = memory + TMA_ADR;
uint8_t* tac = memory + TAC_ADR;
uint8_t* ly = memory + LY_ADR;
uint8_t* lyc = memory + LYC_ADR;
uint8_t* lcdc = memory + LCDC_ADR;
uint8_t* lcdc_stat = memory + LCDC_STAT_ADR;
uint8_t* scx = memory + SCX_ADR;
uint8_t* scy = memory + SCY_ADR;
uint8_t* windowx = memory + WX_ADR;
uint8_t* windowy = memory + WY_ADR;
uint8_t* bgp = memory + BGP_ADR;
uint8_t* ie = memory + IE_ADR;
uint8_t* intf = memory + IF_ADR;
uint8_t* dma = memory + DMA_ADR;
uint8_t* obp0 = memory + OBP0_ADR;
uint8_t* obp1 = memory + OBP1_ADR;
uint8_t* joyp = memory + JOYP_ADR;

const uint8_t bootrom[0x100] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x00, 0x00, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x00, 0x00, 0x3E, 0x01, 0xE0, 0x50
};


uint8_t vram_oam_locked = 1;

/* -------------- dma -------------- */

#define DMA_DURATION 160

uint16_t dma_source;
const uint16_t dma_dest = 0xFE00;
uint8_t performing_dma_transfer = 0;
uint8_t dma_transfer_counter = 0;

void init_dma_transfer(uint8_t value)
{
    dma_source = value * 0x100;
    performing_dma_transfer = 1;
    dma_transfer_counter = 0;
}

void update_dma_transfer(uint8_t cycles)
{
    if(performing_dma_transfer)
    {
        dma_transfer_counter += cycles;
        if(dma_transfer_counter >= DMA_DURATION)
        {
            performing_dma_transfer = 0;
            for(uint16_t i = 0; i < 0x100; i++)
            {
                uint8_t byte;
                mmu_read(dma_source+i, &byte);
                mmu_write(dma_dest+i, byte);
            }
        }
    }
}

void reset_memory()
{
    memset(memory, 0, MEM_SIZE);
    performing_dma_transfer = 0;
    dma_transfer_counter = 0;
}

void lock_vram_oam()
{
    vram_oam_locked = 1;
}

void free_vram_oam()
{
    vram_oam_locked = 0;
}

uint8_t restricted_memory(uint16_t address)
{
    // TODO: remove
    // if(address == JOYP_ADR)
    //     return 1;
    
    if(vram_oam_locked)
    {
        if(address >= 0x8000 && address <= 0x9FFF)
            return 1;

        if(address >= 0xFE00 && address <= 0xFE9F)
            return 1;
    }

    if(address >= 0xFEA0 && address <= 0xFEFF)
        return 1;

    return 0;
}


int mmu_write(uint16_t address, uint8_t byte)
{
    if(address == BOOT_OFF_ADR)
    {
        #ifndef DEBUG
            printf("Boot terminated\n");
        #endif

        #ifdef DEBUG
            set_debug(1);
        #endif
    }

    if(restricted_memory(address))
        return 1;

    if(address == BOOT_OFF_ADR && memory[address] == 0x1)
        return 1;

    if(address == DIV_ADR)
    {
        reset_div();
        return 1;
    }

    if(address == DMA_ADR)
    {
        init_dma_transfer(byte);
        return 1;
    }

    if(address == JOYP_ADR)
    {
        write_joyp_reg(byte);
        return 1;
    }   

    memory[address] = byte;
    return 1;
}

void mmu_write_u16(uint16_t address, uint16_t byte)
{
    uint8_t lower = byte & 0xFF;
    uint8_t higher = byte >> 8;

    mmu_write(address, lower);
    mmu_write(address+1, higher);
}

void mmu_read(uint16_t address, uint8_t* dest)
{
    // TODO: When the PPU is accessing some video-related memory, that memory is inaccessible to the CPU: writes are ignored, and reads return garbage values (usually $FF).
    // TODO: When performing dma transfer, cpu can only read from HRAM 

    #ifdef DEBUG
        if(address == LY_ADR)
        {
            *dest =0x90;
            return;
        }
    #endif

    if(restricted_memory(address))
    {
        *dest = 0xFF;
        return;
    }
    
    if(address <= 0x00FF && !memory[BOOT_OFF_ADR])
        *dest = bootrom[address];
    else
        *dest = memory[address];
}

void mmu_read_u16(uint16_t address, uint16_t* dest)
{
    uint8_t lower, higher;
    mmu_read(address, &higher); // TODO: first lower?
    mmu_read(address, &lower);

    *dest = lower | (higher << 8);
}