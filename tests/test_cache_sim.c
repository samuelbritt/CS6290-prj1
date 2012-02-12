
#include "CuTest.h"

#include <cache_sim.c>

void test_decode_address(CuTest *tc)
{
	CuFail(tc, "Not yet implemented");
}

CuSuite* test_cache_sim_get_suite()
{
	CuSuite *suite = CuSuiteNew();

	// SUITE_ADD_TEST calls
	SUITE_ADD_TEST(suite, test_decode_address);

	return suite;
}

