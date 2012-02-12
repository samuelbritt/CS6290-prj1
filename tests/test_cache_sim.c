
#include "CuTest.h"

#include <cache_sim.c>

void test_decode_address(CuTest *tc)
{
	struct decoded_address d;
	struct cache c;
	c.c = 0;
	c.s = 0;
	c.b = 0;
	unsigned long addr = 0xDEADBEEF;

	decode_address(&d, addr, &c);
	CuAssertTrue(tc, addr == d.tag);
	CuAssertTrue(tc, 0 == d.index);
	CuAssertTrue(tc, 0 == d.offset);

	c.b = 4;
	c.c = 8;
	decode_address(&d, addr, &c);
	CuAssertTrue(tc, 0xDEADBE == d.tag);
	CuAssertTrue(tc, 0xE == d.index);
	CuAssertTrue(tc, 0xF == d.offset);

	c.b = 8;
	c.c = 16;
	decode_address(&d, addr, &c);
	CuAssertTrue(tc, 0xDEAD == d.tag);
	CuAssertTrue(tc, 0xBE == d.index);
	CuAssertTrue(tc, 0xEF == d.offset);

}

CuSuite* test_cache_sim_get_suite()
{
	CuSuite *suite = CuSuiteNew();

	// SUITE_ADD_TEST calls
	SUITE_ADD_TEST(suite, test_decode_address);

	return suite;
}

