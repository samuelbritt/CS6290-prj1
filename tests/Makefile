include $(ROOT)/Rules.mk

TGT = tests

INCDIRS = ../src

INCLUDES = $(INCDIRS:%=-I%)

SRC = test_cache_sim.c tests.c CuTest.c
OBJ = $(SRC:%.c=%.o)

all: $(TGT)

$(TGT): $(OBJ)
	$(LINK)

%.o: %.c
	$(COMP)

test_cache_sim.c: ../src/cache_sim.c
clean:
	rm -f $(OBJ) $(TGT)
