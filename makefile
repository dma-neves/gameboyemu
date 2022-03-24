LIBS=-lSDL2

_DEPS=main.c cpu.h memory.h timer.h ppu.h ui.h
DEPS=$(patsubst %,src/%,$(_DEPS))

CC=gcc
CFLAGS=-I src -g

ODIR=obj
_OBJ=main.o cpu.o memory.o timer.o ppu.o ui.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

emu: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(ODIR)/*.o emu
