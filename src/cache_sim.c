#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define CACHE_COUNT 3

/* flags for block */
#define VALID 0x01
#define DIRTY 0x02

enum access_type {
	READ_ACCESS = 0,
	WRITE_ACCESS
};

struct decoded_address {
	unsigned tag;
	unsigned index;
	unsigned offset;
};

struct access_params {
	enum access_type type;
	struct decoded_address addr;
};

enum cache_level {
	L1 = 1,
	L2,
	L3,
};

struct cache_entry {
	unsigned int tag;
	unsigned long age;
	unsigned char flags;
	void *data_block;		// left empty for simulation
};

struct set {
	unsigned int entry_count;
	struct cache_entry *entries;
};

struct cache {
	struct cache *next;		// linked list; NULL for last cache
	enum cache_level level;
	unsigned int c;			// log_2(cache size)
	unsigned int b;			// log_2(block size)
	unsigned int s; 		// log_2(set associativity)
	unsigned int set_count;
	struct set *sets;		// array addressed by index

	unsigned int average_access_time; // nanoseconds
	unsigned int access_count[2];	// indexed by enum access_type
	unsigned int miss_count[2];	// indexed by enum access_type
	size_t writebacks;		// bytes
	size_t data_transferred;	// byte
	size_t total_storage;		// bits
};

#define fail(msg) do {			\
	fprintf(stderr, "%s\n", (msg));	\
	exit(EXIT_FAILURE);		\
} while (0);

/* *alloc's that fail */
static inline void *emalloc(size_t size)
{
	void *p = malloc(size);
	if (!p)
		fail("Memory error");
	return p;
}
static inline void *ecalloc(size_t size)
{
	void *p = calloc(1, size);
	if (!p)
		fail("Memory error");
	return p;
}

static int set_count(struct cache *cache)
{
	return 1 << (cache->c - cache->b - cache->s);
}

static int blocks_per_set(struct cache *cache)
{
	return 1 << cache->s;
}

/* initialize block */
static void cache_init(struct cache *cache, int level, unsigned c, unsigned b, unsigned s)
{
	/* most values are 0 to start with */
	memset(cache, 0, sizeof(*cache));

	cache->c = c;
	cache->b = b;
	cache->s = s;
	cache->level = level;

	/* Allocate all the memory at once. The buffer is structured as set0 for
	 * the cache, followed by set1, set2, ..., followed by all the entries
	 * for set0, followed for all the entries for set1, etc. */
	cache->set_count = set_count(cache);
	int entries_per_set = blocks_per_set(cache);
	size_t sizeof_sets_total = cache->set_count * sizeof(*cache->sets);
	int entry_count_total = cache->set_count * entries_per_set;
	size_t sizeof_single_entry = sizeof(*cache->sets[0].entries);
	size_t sizeof_entries_total = entry_count_total * sizeof_single_entry;
	cache->sets = ecalloc(sizeof_sets_total + sizeof_entries_total);

	/* set up the pointers for the above memory layout. Fist cache_entry
	 * comes after all the sets */
	struct cache_entry *e;
	e = (struct cache_entry *) (cache->sets + cache->set_count);
	for (int i = 0; i < cache->set_count; ++i) {
		cache->sets[i].entry_count = entries_per_set;
		cache->sets[i].entries = e;
		e += cache->sets[i].entry_count;
	}
}

/* functions to perform statistics on caches */
static unsigned int total_accesses(struct cache *cache)
{
	return cache->access_count[READ_ACCESS] + cache->access_count[WRITE_ACCESS];
}

static float miss_rate(struct cache *cache)
{
	return cache->miss_count[READ_ACCESS] / ((float) cache->access_count[READ_ACCESS]);
}

static float hit_time(struct cache *cache)
{
	return 2 * cache->level + 0.2 * cache->level * cache->s;
}

static unsigned int miss_penalty(struct cache *cache);
static unsigned int average_access_time(struct cache *cache)
{
	return hit_time(cache) + miss_rate(cache) * miss_penalty(cache);
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

/* splits address into tag, index, offset */
static void decode_address(struct decoded_address *d_addr,
			   unsigned long addr,
			   struct cache *cache)
{
	memset(d_addr, 0, sizeof(*d_addr));
	d_addr->tag = addr >> (cache->c - cache->s);
	unsigned long mask = (1 << (cache->c - cache->s)) - 1;
	d_addr->index = (addr & mask) >> cache->b;
	mask = (1 << cache->b) - 1;
	d_addr->offset = addr & mask;
}

/* attempts to access the data within a set */
static int set_access(struct set *set, int access_type, struct decoded_address *addr)
{
	fprintf(stderr, "set_access() not implemented"); exit(1); /* TODO */
}

static void cache_access(struct cache *cache, enum access_type type, unsigned long addr)
{
	if (!cache)
		return;
	cache->access_count[type]++;
	struct decoded_address d_addr;
	decode_address(&d_addr, addr, cache);
	struct set *set = &cache->sets[d_addr.index];
	int available = set_access(set, type, &d_addr);
	if (!available) {
		cache->miss_count[type]++;
		cache_access(cache->next, type, addr);
	}
}

/* returns the access_type specified by the char input. Returns -1 on error */
static int rw2access_type(char rw)
{
	switch (rw) {
		case 'r':
			return READ_ACCESS;
			break;
		case 'w':
			return WRITE_ACCESS;
			break;
		default:
			fail("invalid r/w input");
			break;
	}
	return -1;
}

/* Parses the command line arguments. The program takes 9 required arguments,
 * c_i, b_i, s_i, for i = 1..3 being the cache levels. It also takes one
 * optional argument, `-f <FILENAME>`, where FILENAME is the name of the input
 * file. If the `-f` argument is not specified, it reads from stdin.
 */
static void parse_args(int argc, char const *argv[]) {
	int opt;
	char *filename = NULL;

	char *usage = "Usage: %s c1 b1 s1 c2 b2 s2 c3 b3 s3 [-f <INPUT_FILE>]\n"
	while ((opt = getopt(argc, argv, "f:")) != -1) {
		switch (opt) {
		case 'f':
			filename = optarg;
			break;
		default:
			fail(usage);
		}
	}

	if (argc - optind > 9)
}

int main_(int argc, char const *argv[])
{
	struct cache caches[CACHE_COUNT];

	int C[] = {9, 10, 11};
	int B[] = {6, 6, 6};
	int S[] = {2, 3, 4};
	for (int i = 0; i < CACHE_COUNT; ++i) {
		int level = i + 1;
		cache_init(&caches[i], level, C[i], B[i], S[i]);
		caches[i].next = &caches[i+1];
	}
	caches[CACHE_COUNT - 1].next = NULL;

	void *addr;
	char rw;
	while (!feof(stdin)) {
		fscanf(stdin, "%c %p\n", &rw, &addr);
		cache_access(caches, rw2access_type(rw), (unsigned long) addr);
	}

	struct cache;
	printf("Total number of accesses: %d\n", total_accesses(&caches[0]));
	printf("Total number of reads: %d\n", caches[0].read_count);
	printf("Total number of writes: %d\n", caches[0].write_count);

	return 0;
}
