LIBS=-lSDL2

_DEPS=main.c cpu.h mem.h timer.h ppu/ppu.h ppu/ui.h ppu/obj_list.h
DEPS=$(patsubst %,src/%,$(_DEPS))

CC=gcc
CFLAGS=-I src -g -Wall -Wformat

ODIR=obj
_OBJ=main.o cpu.o mem.o timer.o ppu/ppu.o ppu/ui.o ppu/obj_list.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

emu: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(ODIR)/*.o $(ODIR)/ppu/*.o emu