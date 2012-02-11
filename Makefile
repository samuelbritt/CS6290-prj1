TGT = cache_sim
CC = gcc
CFLAGS = -std=gnu99 -Wall -pedantic

SRC = src/cache_sim.c
OBJ = src/cache_sim.o

all: $(TGT)

$(TGT): src/cache_sim.o

$(OBJ): $(SRC)

*.o : *.c
