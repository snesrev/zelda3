# adapted from original README.md:
# `clang++ -I/usr/include/SDL2 -lSDL2 -O2 -ozelda3 *.cpp snes/*.cpp`

ifneq (,$(shell command -v clang++))
	CXX = clang++
else ifneq (,$(shell command -v g++))
	CXX = g++
endif

ifneq (,$(findstring clang,$(CXX)))
	LTO = -flto=thin
else ifneq (,$(findstring g++,$(CXX)))
	LTO = -flto=auto
endif

override CXXFLAGS := -O2 -I/usr/include/SDL2 $(LTO) $(CXXFLAGS)
override LDFLAGS := -lSDL2 $(LDFLAGS)

override OBJS = $(patsubst %.cpp,%.o,$(wildcard *.cpp snes/*.cpp))
override BIN = zelda3

.PHONY: all clean

all: $(BIN)

clean:
	$(RM) $(BIN) $(OBJS)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(BIN) $(OBJS) $(LDFLAGS)
