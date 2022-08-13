CC=gcc
CXX=g++
RM=rm -f
CPPFLAGS=-Ii:/devtools/SDL2-2.0.22/x86_64-w64-mingw32/include/SDL2/ -O2
LDFLAGS=-Li:/devtools/SDL2-2.0.22/x86_64-w64-mingw32/lib/
LDLIBS=-lSDL2

SRCS=$(wildcard *.cpp) $(wildcard snes/*.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all: zelda3

zelda3: $(OBJS)
	$(CXX) $(LDFLAGS) -o zelda3 $(OBJS) $(LDLIBS)

depend: .depend

.depend: $(SRCS)
	$(RM) ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) *~ .depend

include .depend
