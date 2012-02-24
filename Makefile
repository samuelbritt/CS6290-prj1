# Top level make
ROOT = $(shell pwd)
export ROOT

include Rules.mk

SRC_DIRS = src optimize
TEST_DIRS = tests

all:
	@for dir in $(SRC_DIRS); do $(MAKE) -C $$dir; done
test:
	@$(MAKE)
	@for dir in $(TEST_DIRS); do $(MAKE) -C $$dir; done
	@for dir in $(TEST_DIRS); \
		do echo ; \
		$$dir/tests; \
		done
clean:
	@for dir in $(SRC_DIRS) $(TEST_DIRS); do $(MAKE) -C $$dir clean; done

