LIBS=-lSDL2

_DEPS=main.c
DEPS=$(patsubst %,src/%,$(_DEPS))

CC=gcc
CFLAGS=-I src -g

ODIR=obj
_OBJ=main.o cpu.o memory.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))

emu: $(OBJ)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

$(ODIR)/%.o: src/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm $(ODIR)/*.o emu
