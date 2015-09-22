CC = g++
DEBUG = -g
CFLAGS = -Wall -std=c++11 $(DEBUG)
LFLAGS = -Iinclude/ -lsfml-system -lsfml-window -lsfml-graphics -lpthread

all: game

game:
	$(CC) $(LFLAGS) $(CFLAGS) src/barrier.cpp src/game.cpp
