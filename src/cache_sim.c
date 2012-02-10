#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CACHE_COUNT 3

/* flags for block */
#define VALID 0x01
#define DIRTY 0x02

enum access_type {
	READ,
	WRITE
};

enum cache_level {
	L1 = 1,
	L2,
	L3,
};

struct block {
	void *address;				/* tag and index, no offset */
	unsigned short flags;
	void *data; 				/* left empty for simulation */
};

struct set {
	unsigned int tag;
	unsigned int block_count;
	struct block *blocks;
};

struct cache {
	struct cache *next;		/* linked list; NULL for last cache */
	enum cache_level level;
	unsigned int c;				/* log_2(cache size) */
	unsigned int b;				/* log_2(block size) */
	unsigned int s;				/* log_2(set associativity) */
	unsigned int set_count;
	struct set *sets;			/* array addressed by index */
	unsigned int blocks_per_set;
	unsigned int bit_len_in_tag;
	unsigned int bit_len_in_index;
	unsigned int bit_len_in_offset;

	unsigned int average_access_time;	/* nanoseconds */
	unsigned int read_count;
	unsigned int read_misses;
	unsigned int write_count;
	unsigned int write_misses;
	size_t writebacks;			/* bytes */
	size_t data_transferred;		/* bytes */
	size_t total_storage;			/* bits */
};

#define fail(msg) do {			\
	fprintf(stderr, "%s\n", (msg));	\
	exit(EXIT_FAILURE);		\
} while (0);

/* *alloc's that fail */
static inline void *emalloc(size_t size) {
	void *p = malloc(size);
	if (!p) {
		fail("Memory error");
	}
	return p;
}
static inline void *ecalloc(size_t size) {
	void *p = calloc(1, size);
	if (!p)
		fail("Memory error");
	return p;
}

/* initialize data */
static void cache_init(struct cache *cache, unsigned c, unsigned b, unsigned s)
{
	/* most values are 0 to start with */
	memset(cache, 0, sizeof(*cache));

	cache->c = c;
	cache->b = b;
	cache->s = s;

	cache->set_count = set_count(cache);
	cache->sets = ecalloc(cache->set_count * sizeof(*cache->sets));
	int blocks_per_set = 1 << cache->s;
	struct set *set;
	for (int i = 0; i < cache->set_count; ++i) {
		set = &cache->sets[i];
		set->block_count = blocks_per_set;
		set->blocks = ecalloc(set->block_count * sizeof(*set->blocks));

	}
}

/* functions to perform statistics on caches */
static unsigned int total_accesses(struct cache *cache)
{
	return cache->read_count + cache->write_count;
}

static float miss_rate(struct cache *cache)
{
	return cache->read_misses / ((float) cache->read_count);
}

static unsigned int miss_penalty(struct cache *cache);
static unsigned int average_access_time(struct cache *cache)
{
	unsigned hit_time = 2 * cache->level + 0.2 * cache->level * cache->s;
	return hit_time + miss_rate(cache) * miss_penalty(cache);
}

/* returns the miss penalty in nanoseconds. recursively calls
 * average_access_time */
static unsigned int miss_penalty(struct cache *cache)
{
	if (cache->level == CACHE_COUNT) {
		return 500;
	} else {
		return average_access_time(cache->next);
	}
}

/*
 * First level of cache access
 */
static void cache_access(char c, void *addr)
{
	switch (c) {
		case 'w':
			printf("Write");
			break;
		case 'r':
			printf("Read");
			break;
	}
	printf("\t%p\n", addr);
}

int main(int argc, char const *argv[])
{
	int num_access = 0;
	void *addr;
	char rw;
	while (!feof(stdin)) {
		num_access++;
		fscanf(stdin, "%c %p\n", &rw, &addr);
		cache_access(rw, addr);
	}

	printf("Total number of accesses: %d", num_access);
}
