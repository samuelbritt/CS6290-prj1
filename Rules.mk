# Common variables to use in makefiles
CC = gcc
CFLAGS = -std=gnu99 -Wall -pedantic -g -O0

LINK = $(LINK.c) -o $@ $^
COMP = $(COMPILE.c) $(INCLUDES) $<

BINDIR = $(ROOT)
