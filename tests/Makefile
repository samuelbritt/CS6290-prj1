include $(ROOT)/Rules.mk

TGT = tests

INCDIRS = ../src

INCLUDES = $(INCDIRS:%=-I%)

SRC = ../src/cache_sim.c test_cache_sim.c tests.c CuTest.c
OBJ = $(SRC:%.c=%.o)

all: $(TGT)

$(TGT): $(OBJ)
	$(LINK)

%.o: %.c
	$(COMP)

clean:
	rm -f $(OBJ) $(TGT)
