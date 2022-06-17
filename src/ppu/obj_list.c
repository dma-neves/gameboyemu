#include "ppu/obj_list.h"

#include <stdio.h>

obj obj_list[10];
uint16_t size = 0;

void obj_list_reset()
{
    size = 0;
}

void obj_list_insert(obj o, uint16_t index)
{
    size++;
    
    for(uint16_t i = size-1; i > index; i--)
        obj_list[i] = obj_list[i-1];

    obj_list[index] = o;
}

void obj_list_add(obj o)
{
    if(size == 10)
        return;

    uint16_t index = 0;
    for(; index < size && obj_list[index].pos_x > o.pos_x; index++)
        ;

    obj_list_insert(o, index);
}

obj obj_list_remove()
{
    return obj_list[--size];
}

uint16_t obj_list_size()
{
    return size;
}

void obj_list_print()
{
    for(uint16_t i = 0; i < size; i++)
        printf("pos_x: %d pos_y: %d tile_number: %d flags: %d\n", obj_list[i].pos_x, obj_list[i].pos_y, obj_list[i].tile_number, obj_list[i].flags);

    printf("\n");
}

