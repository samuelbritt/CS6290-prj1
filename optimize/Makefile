include $(ROOT)/Rules.mk

TGT = $(BINDIR)/opt

SRC = ../src/cache_sim.c optimize.c
OBJ = $(SRC:%.c=%.o)

all: $(TGT)

$(TGT): $(OBJ)
	$(LINK)

%.o : %.c
	$(COMP)

clean:
	rm -f $(OBJ) $(TGT)
