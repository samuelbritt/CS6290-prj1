#include <stdio.h>

struct cache {
	int c;				/* log_2(cache size) */
	int b;				/* log_2(block size) */
	int s;				/* log_2(set associativity)*/
	int read_count;
	int read_misses;
	int write_count;
	int write_misses;
	size_t writebacks;		/* bytes */
	size_t data_transferred;	/* bytes */
	size_t total_storage;		/* bits */
}

/* global caches */
struct cache L1, L2, L3;

/* functions to perform statistics on caches */
static int total_accesses(struct cache *c)
{
	return c->read_count + c->write_count;
}

static float miss_rate(struct cache *c)
{
	return c->read_misses / ((float) c->read_count);
}

/*
 * First level of cache access
 */
static void cache_access(char c, int addr)
{
	switch (c) {
		case 'w':
			printf("Write");
			break;
		case 'r':
			printf("Read");
			break;
	}
	printf("\t0x%x\n", addr);
}

int main(int argc, char const *argv[])
{
	int num_access = 0;
	int addr;
	char rw;
	while (!feof(stdin)) {
		num_access++;
		fscanf(stdin, "%c %x\n", &rw, &addr);
		cache_access(rw, addr);
	}

	printf("Total number of accesses: %d", num_access);
}
