CXX = g++
BIN = bin/zthos
SRC = main.cpp
FLAGS = -Wall -O2 -I/usr/include/SDL2 -D_REENTRANT
LIB = -lSDL2

all: debug

debug:
	$(CXX) -g $(FLAGS) $(SRC) -o $(BIN) $(LIB)

release:
	$(CXX) $(FLAGS) $(SRC) -o $(BIN) $(LIB)

clean:
	rm -f $(BIN)
