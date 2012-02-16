#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef int bool;
#define False 0;
#define True 1;

#define CACHE_COUNT 3

enum access_type {
	READ_ACCESS = 0,
	WRITE_ACCESS
};

struct decoded_address {
	unsigned tag;
	unsigned index;
	unsigned offset;
};

enum cache_level {
	L1 = 1,
	L2,
	L3,
};

/* flags for block */
#define VALID 0x01
#define DIRTY 0x02
struct cache_entry {
	unsigned int tag;
	unsigned long age;
	unsigned char flags;
	void *data_block;		// left empty for simulation
};

struct set {
	unsigned int entry_count;
	struct cache_entry *entries;	// array of entries
	struct cache_entry *empty;	// first empty slot
	struct cache_entry *LRU;	// current LRU entry
};

struct cache {
	struct cache *next;		// linked list; NULL for last cache
	enum cache_level level;
	unsigned int c;			// log_2(cache size)
	unsigned int b;			// log_2(block size)
	unsigned int s; 		// log_2(set associativity)
	unsigned int set_count;
	struct set *sets;		// array addressed by index

	unsigned int access_count[2];	// indexed by enum access_type
	unsigned int miss_count[2];	// indexed by enum access_type
	unsigned int writebacks;	// number of writeback accesses
	size_t data_transferred;	// bytes
	unsigned long total_storage;	// bits
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

static size_t bytes_per_block(struct cache *cache)
{
	return 1 << cache->b;
}

/* initialize block */
static void cache_init(struct cache *cache, int level, unsigned c, unsigned b,
                       unsigned s)
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
	return cache->access_count[READ_ACCESS]
	        + cache->access_count[WRITE_ACCESS];
}

static unsigned int total_misses(struct cache *cache)
{
	return cache->miss_count[READ_ACCESS] + cache->miss_count[WRITE_ACCESS];
}

/* returns writebacks in bytes */
static size_t writebacks(struct cache *cache)
{
	return cache->writebacks * bytes_per_block(cache);
}

static size_t total_storage(struct cache *cache)
{
	return 0;
}

static float miss_rate(struct cache *cache)
{
	return cache->miss_count[READ_ACCESS]
	        / ((float) cache->access_count[READ_ACCESS]);
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

/* combines an address from its tag, index, and offset into a full pointer */
static void *encode_address(struct decoded_address *d_addr, struct cache *cache)
{
	unsigned long addr =
	        (d_addr->tag << (cache->c - cache->s) |
	                d_addr->index << (cache->b) |
	                d_addr->offset);
	return (void *) addr;
}

/* splits address into tag, index, offset */
static void decode_address(struct decoded_address *d_addr,
                           void *addr,
                           struct cache *cache)
{
	unsigned long addr_ = (unsigned long) addr; // can't do bit-ops on a pointer
	memset(d_addr, 0, sizeof(*d_addr));
	d_addr->tag = addr_ >> (cache->c - cache->s);
	unsigned long mask = (1 << (cache->c - cache->s)) - 1;
	d_addr->index = (addr_ & mask) >> cache->b;
	mask = (1 << cache->b) - 1;
	d_addr->offset = addr_ & mask;
}

static bool entry_access(struct cache_entry *e, unsigned tag,
                         enum access_type type)
{
	if (e->tag == tag) {
		e->age = 0;
		if (type == WRITE_ACCESS)
			e->flags |= DIRTY;
		return True;
	}
	e->age++;
	return False;
}

static bool set_access(struct set *set, unsigned tag, enum access_type type)
{
	bool hit = False;
	set->empty = NULL;
	set->LRU = NULL;
	struct cache_entry *e;
	for (int i = 0; i < set->entry_count; ++i) {
		e = &set->entries[i];
		if (!(e->flags & VALID)) {
			set->empty = e;
			continue;
		}

		hit = entry_access(e, tag, type) || hit;
		if ((!set->LRU) || (e->age > set->LRU->age))
			set->LRU = e;
	}
	return hit;
}

static struct cache_entry *entry_to_evict(struct set *set)
{
	if (set->empty)
		return set->empty;
	if (set->LRU)
		return set->LRU;
	fail("Cache should always have something to evict");
	return NULL;
}

static void cache_access(struct cache *c, enum access_type type, void *addr);
static void cache_miss(struct cache *cache, enum access_type type,
                       struct set *set,
                       struct decoded_address *access_addr)
{
	cache->miss_count[type]++;
	struct cache_entry *e = entry_to_evict(set);
	if ((e->flags & VALID) && (e->flags & DIRTY)) {
		// write the contents of the evicted address to the
		// next level cache
		struct decoded_address evict_addr;
		evict_addr.tag = e->tag;
		evict_addr.index = access_addr->index;
		evict_addr.offset = 0;
		cache->writebacks++;
		cache_access(cache->next, WRITE_ACCESS,
		             encode_address(&evict_addr, cache));
	}

	cache_access(cache->next, READ_ACCESS, encode_address(access_addr,
							      cache));
	e->tag = access_addr->tag;
	e->age = 0;
	e->flags = VALID;

	if (type == WRITE_ACCESS)
		e->flags |= DIRTY;
}

static void cache_access(struct cache *cache, enum access_type type, void *addr)
{
	if (!cache)
		return;
	cache->access_count[type]++;
	struct decoded_address d_addr;
	decode_address(&d_addr, addr, cache);
	struct set *set = &cache->sets[d_addr.index];

	bool hit = set_access(set, d_addr.tag, type);
	if (!hit)
		cache_miss(cache, type, set, &d_addr);
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
 * c_i, b_i, s_i, for i = 1..3 being the cache levels.
 */
static void parse_args(int argc, char *argv[], int *cache_params)
{
	char *usage_fmt = "Usage: %s c1 b1 s1 c2 b2 s2 c3 b3 s3\n";
	if (argc < 1 + CACHE_COUNT * 3) {
		fprintf(stderr, usage_fmt, argv[0]);
		exit(EXIT_FAILURE);
	}

	for (int i = 1; i < argc; ++i) {
		cache_params[i - 1] = atoi(argv[i]);
	}
}

void print_cache(struct cache *c)
{
	printf("L%d Cache\n", c->level);
	printf("Accesses: %d\n", total_accesses(c));
	printf("Reads: %d\n", c->access_count[READ_ACCESS]);
	printf("Read Misses: %d\n", c->miss_count[READ_ACCESS]);
	printf("Writes: %d\n", c->access_count[WRITE_ACCESS]);
	printf("Write Misses: %d\n", c->miss_count[WRITE_ACCESS]);
	printf("Writebacks (Bytes): %zd\n", writebacks(c));
	printf("Data Transferred (Bytes): %zd\n", c->data_transferred);
	printf("Total Misses: %d\n", total_misses(c));
	printf("Miss Rate: %0.6g\n", miss_rate(c));
	printf("Total Storage (Bits): %lu\n", c->total_storage);
}

void print_results(struct cache caches[])
{
	printf("Parameters:\n");
	for (int i = 0; i < CACHE_COUNT; i++)
	{
		printf("C%d: %d\n", caches[i].level, caches[i].c);
		printf("B%d: %d\n", caches[i].level, caches[i].b);
		printf("S%d: %d\n", caches[i].level, caches[i].s);
	}

	printf("\nAAT: %d ns\n", average_access_time(caches));
	printf("Total Storage (Bytes): %lu\n", total_storage(caches));

	for (int i = 0; i < CACHE_COUNT; ++i) {
		printf("\n");
		print_cache(&caches[i]);
	}
}

int main_(int argc, char *argv[])
{
	int cache_params[CACHE_COUNT * 3];
	parse_args(argc, argv, cache_params);

	struct cache caches[CACHE_COUNT];
	int j = 0;
	for (int i = 0; i < CACHE_COUNT; ++i) {
		int level = i + 1;
		int c = cache_params[j++];
		int b = cache_params[j++];
		int s = cache_params[j++];
		cache_init(&caches[i], level, c, b, s);
		caches[i].next = &caches[i+1];
	}
	caches[CACHE_COUNT - 1].next = NULL;

	void *addr;
	char rw;
	while (!feof(stdin)) {
		fscanf(stdin, "%c %p\n", &rw, &addr);
		cache_access(caches, rw2access_type(rw), addr);
	}
	print_results(caches);
	return 0;
}
