#ifndef CACHE_SIM_H_
#define CACHE_SIM_H_

#define CACHE_COUNT 3

struct set;
struct cache {
	struct cache *next;		// linked list; NULL for last cache
	int cache_level;		// L1, L2, L3, etc
	unsigned int c;			// log_2(cache size)
	unsigned int b;			// log_2(block size)
	unsigned int s; 		// log_2(set associativity)
	unsigned int set_count;
	struct set *sets;		// array addressed by index

	unsigned int access_count[2];	// indexed by enum access_type
	unsigned int miss_count[2];	// indexed by enum access_type
	unsigned int writeback_count;	// number of writeback accesses
};

void cache_init(struct cache *cache, struct cache *next, int level, unsigned c, unsigned b,
		unsigned s);
void cache_destroy(struct cache *cache);

/* returns the total area, including overhead, for a linked list of caches */
size_t total_cache_area(struct cache *cache_list);

/* returns average access time for the cache (in ns) */
unsigned int average_access_time(struct cache *cache);

/* access a cache for a given address. access_type can be either "READ_ACCESS"
 * or "WRITE_ACCESS" */
void cache_access(struct cache *cache, int access_type, void *addr);

/* convert a 'r' or 'w' char to approprate access_type */
int rw2access_type(char rw);

#endif /* end of include guard: CACHE_SIM_H_ */

