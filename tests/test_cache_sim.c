
#include "CuTest.h"

#include <cache_sim.c>

void test_validate(CuTest *tc) {
	/* validates all the files in tests/validation */
	char *scripts[] = {
		"tests/validation/gcc_validation.sh 2> /dev/null",
		"tests/validation/go_validation.sh 2> /dev/null",
		"tests/validation/mcf_validation.sh 2> /dev/null",
		""
	};
	char *s;
	int i = 0;
	while (*(s = scripts[i++]) != '\0')
		system(s);

}

void test_decode_address(CuTest *tc)
{
	struct decoded_address d;
	struct cache c;
	c.c = 0;
	c.s = 0;
	c.b = 0;
	void *addr = (void *) 0xDEADBEEF;

	decode_address(&d, addr, &c);
	CuAssertTrue(tc, (unsigned long) addr == d.tag);
	CuAssertTrue(tc, 0 == d.index);
	CuAssertTrue(tc, 0 == d.offset);

	c.b = 4;
	c.c = 8;
	decode_address(&d, addr, &c);
	CuAssertIntEquals(tc, 0xDEADBE, d.tag);
	CuAssertIntEquals(tc, 0xE, d.index);
	CuAssertIntEquals(tc, 0xF, d.offset);

	c.b = 8;
	c.c = 16;
	decode_address(&d, addr, &c);
	CuAssertIntEquals(tc, 0xDEAD, d.tag);
	CuAssertIntEquals(tc, 0xBE, d.index);
	CuAssertIntEquals(tc, 0xEF, d.offset);

	c.c = 16;
	c.s = 4;
	c.b = 4;
	decode_address(&d, addr, &c);
	CuAssertIntEquals(tc, 0xDEADB, d.tag);
	CuAssertIntEquals(tc, 0xEE, d.index);
	CuAssertIntEquals(tc, 0xF, d.offset);
}

void test_encode_address(CuTest *tc) {
	struct cache c;
	c.c = c.s = c.b = 0;
	struct decoded_address d;
	d.tag = 0xDEADBEEF;
	d.index = 0;
	d.offset = 0;
	CuAssertPtrEquals(tc, (void *) 0xDEADBEEF,
	                  encode_address(&d, &c));
	c.c = 16;
	c.s = 4;
	c.b = 4;
	d.tag = 0xDEADB;
	d.index = 0xEE;
	d.offset = 0xF;
	CuAssertPtrEquals(tc, (void *) 0xDEADBEEF,
	                  encode_address(&d, &c));
}

void test_set_access(CuTest *tc) {
	struct decoded_address addr;
	struct cache cache;
	int c = 16;
	int b = 4;
	int s = 4;
	cache_init(&cache, NULL, 0, c, b, s);
	CuAssertIntEquals(tc, 16, cache.sets[0].entry_count);

	addr.tag = 0xDEADB;
	addr.index = 0xEE;
	addr.offset = 0xF;

	/* fill in with some dummy data */
	struct set *set = cache.sets;
	struct cache_entry *e;
	for (int i = 0; i < set->entry_count; ++i) {
		e = &set->entries[i];
		e->age = i;
		e->tag = i;
		e->flags = VALID;
	}

	int mid = set->entry_count / 2;
	set->entries[mid].age = 250;
	set->entries[mid].tag = 42;
	int hit = set_access(set, addr.tag, READ_ACCESS);
	CuAssertIntEquals(tc, 0, hit);
	CuAssertIntEquals(tc, 251, set->entries[mid].age);
	CuAssertTrue(tc, set->entries[mid].flags & VALID);
	CuAssertTrue(tc, !(set->entries[mid].flags & DIRTY));

	set->entries[mid].tag = 42;
	addr.tag = 43;
	hit = set_access(set, addr.tag, WRITE_ACCESS);
	CuAssertIntEquals(tc, 0, hit);
	CuAssertIntEquals(tc, 252, set->entries[mid].age);
	CuAssertTrue(tc, set->entries[mid].flags & VALID);
	CuAssertTrue(tc, !(set->entries[mid].flags & DIRTY));

	set->entries[mid].tag = 42;
	addr.tag = 42;
	hit = set_access(set, addr.tag, WRITE_ACCESS);
	CuAssertIntEquals(tc, 1, hit);
	CuAssertIntEquals(tc, 0, set->entries[mid].age);
	CuAssertTrue(tc, set->entries[mid].flags & VALID);
	CuAssertTrue(tc, (set->entries[mid].flags & DIRTY));
}

CuSuite* test_cache_sim_get_suite()
{
	CuSuite *suite = CuSuiteNew();

	// SUITE_ADD_TEST calls
	SUITE_ADD_TEST(suite, test_decode_address);
	SUITE_ADD_TEST(suite, test_encode_address);
	SUITE_ADD_TEST(suite, test_set_access);
	SUITE_ADD_TEST(suite, test_validate);

	return suite;
}

