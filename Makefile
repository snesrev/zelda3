# adapted from original README.md:
# `clang -I/usr/include/SDL2 -lSDL2 -O2 -ozelda3 *.c snes/*.c`

ifneq (,$(shell which clang))
	CC = clang
else ifneq (,$(shell which gcc))
	CC = gcc
endif

ifneq (,$(findstring clang,$(CC)))
	LTO = -flto=thin
else ifneq (,$(findstring gcc,$(CC)))
	LTO = -flto=auto
endif

override CFLAGS := -O2 `sdl2-config --cflags` $(LTO) $(CXXFLAGS)
override LDFLAGS := -lm `sdl2-config --libs` $(LDFLAGS)

override OBJS = $(patsubst %.c,%.o,$(wildcard *.c snes/*.c))
override BIN = zelda3

.PHONY: all clean

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(BIN) $(OBJS) $(LDFLAGS)
