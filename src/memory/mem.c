#include "memory/mem.h"
#include "memory/file_loader.h"
#include "memory/mbc1.h"
#include "timer.h"
#include "cpu.h"
#include "controls.h"

#include <string.h>
#include <stdio.h>

uint8_t memory[MEM_SIZE];
uint8_t bootrom[BR_SIZE];

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

/* -------------- flags -------------- */

uint8_t vram_oam_locked = 1;
uint8_t mbc1_enabled = 0;

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

void memory_destroy()
{
    if(mbc1_enabled)
        mbc1_destroy();
}

int load_bootrom(char* file)
{
    return load_file(file, 0, bootrom, 0, BR_SIZE);
}

static int load_header(char* file)
{
    return load_file(file, 0, memory, ROM_B00_ADR, CART_HEADER_SIZE);
}

int load_rom(char* file)
{
    int ret = load_header(file);
    if(ret == -1) return -1;

    uint8_t cart_type = memory[0x147];
    uint8_t rom_size = memory[0x148];
    uint8_t ram_size = memory[0x149];

    printf("cartridge type code: 0x%x\n", cart_type);
    printf("rom size code: 0x%x\n", rom_size);
    printf("ram size code: 0x%x\n", ram_size);

    switch(cart_type)
    {
        case 0x0:
            ret = load_file(file, CART_HEADER_SIZE, memory, ROM_B00_ADR+CART_HEADER_SIZE, MAX_CART_SIZE);
            break;

        case 0x1:
            mbc1_enabled = 1;
            ret = mbc1_load_file(file);
            break;

        default:
            printf("error: mbc > 1 not supported\n");
            ret = -1;
            break;
    }

    return ret;
}

void load_memory_bank_0(uint8_t* data)
{
    memcpy(memory, data, MEM_BANK_SIZE);
}

void load_memory_bank_n(uint8_t* data)
{
    memcpy(memory+MEM_BANK_SIZE, data, MEM_BANK_SIZE);
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
    if(mbc1_enabled && address <= 0x7FFF)
    {
        if(address <= 0x1FFF)
        {
            // ignore
            return 1;
        }
        else if(address <= 0x3FFF)
        {
            mbc1_switch_bank(byte);
            return 1;
        }
        else
        {
            printf("error: unsupported mbc1 feaure\n");
            return 1;
        }
    }

    if(address <= 0x7FFF || restricted_memory(address))
        return 1;

    if(address == BOOT_OFF_ADR && memory[address] == 0x1)
    {
        return 1;

        #ifndef DEBUG
            printf("Boot terminated\n");
        #endif

        #ifdef DEBUG
            set_debug(1);
        #endif
    }

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

void mmu_read(uint16_t address, uint8_t* dest)
{
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

    if(address == KEY1_ADR)
        *dest = 0xFF;    
    else if(address <= 0xFF && !memory[BOOT_OFF_ADR])
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

void mmu_write_u16(uint16_t address, uint16_t byte)
{
    uint8_t lower = byte & 0xFF;
    uint8_t higher = byte >> 8;

    mmu_write(address, lower);
    mmu_write(address+1, higher);
}