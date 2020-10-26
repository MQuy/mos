#include <unity.h>

void test_Demo(void)
{
}

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
	TEST_ASSERT_EQUAL_HEX(0x1234, 0x1235);
}

void test_function_should_doAlsoDoBlah(void)
{
	TEST_ASSERT_EQUAL_HEX(0x1234, 0x1234);
}

// not needed when using generate_test_runner.rb
int main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_function_should_doBlahAndBlah);
	RUN_TEST(test_function_should_doAlsoDoBlah);
	return UNITY_END();
}
