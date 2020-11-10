#include <stdlib.h>
#include <unistd.h>

#include "unity.h"

void setUp(void)
{
	// set stuff up here
}

void tearDown(void)
{
	// clean stuff up here
}

void test_function_should_doBlahAndBlah(void)
{
	environ = "name=value";
	char *envval = getenv("name");
	TEST_ASSERT_EQUAL_STRING(envval, "value");
}
