LIBS=-lSDL2

_DEPS=main.c cpu.h memory/mem.h timer.h controls.h video/ppu.h video/ui.h video/obj_list.h
DEPS=$(patsubst %,src/%,$(_DEPS))

CC=gcc
CFLAGS=-I src -g -Wall -Wformat

ODIR=obj
_OBJ=main.o cpu.o memory/mem.o timer.o controls.o video/ppu.o video/ui.o video/obj_list.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

emu: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(ODIR)/*.o $(ODIR)/video/*.o $(ODIR)/memory/*.o emu