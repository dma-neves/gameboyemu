#ifndef OBJ_LIST
#define OBJ_LIST

#include <stdint.h>

typedef struct obj
{
        uint8_t pos_y;
        uint8_t pos_x;
        uint8_t tile_number;
        uint8_t flags;
} obj;

void obj_list_reset();
void obj_list_add(obj o);
obj obj_list_remove();
uint16_t obj_list_size();

void obj_list_print(); // Debug

#endif