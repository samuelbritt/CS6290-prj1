include $(ROOT)/Rules.mk

TGT = $(BINDIR)/cache_sim

SRC = cache_sim.c main.c
OBJ = $(SRC:%.c=%.o)

all: $(TGT)

$(TGT): $(OBJ)
	$(LINK)

*.o : *.c
	$(COMP)

clean:
	rm -f $(OBJ) $(TGT)
