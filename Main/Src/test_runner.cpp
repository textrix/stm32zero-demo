/**
 * STM32ZERO Runtime Test Runner
 *
 * Executes all runtime tests and outputs results via sio (115200 baud).
 * Output format: [PASS] or [FAIL] followed by test description.
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero.hpp"
#include "stm32zero-sio.hpp"
#include "stm32zero-freertos.hpp"
#include <cstdio>
#include <cstring>

using namespace stm32zero;
using namespace stm32zero::freertos;

//=============================================================================
// Test Framework
//=============================================================================

static uint32_t test_pass_count = 0;
static uint32_t test_fail_count = 0;

#define TEST_ASSERT(cond, desc) \
	do { \
		if (cond) { \
			printf("[PASS] %s\r\n", desc); \
			test_pass_count++; \
		} else { \
			printf("[FAIL] %s\r\n", desc); \
			test_fail_count++; \
		} \
	} while (0)

#define TEST_ASSERT_EQ(actual, expected, desc) \
	do { \
		if ((actual) == (expected)) { \
			printf("[PASS] %s\r\n", desc); \
			test_pass_count++; \
		} else { \
			printf("[FAIL] %s (expected %ld, got %ld)\r\n", desc, (long)(expected), (long)(actual)); \
			test_fail_count++; \
		} \
	} while (0)

//=============================================================================
// External Test Functions
//=============================================================================

extern "C" void test_core_runtime(void);
extern "C" void test_sio_runtime(void);
extern "C" void test_freertos_runtime(void);
extern "C" void test_ustim_runtime(void);

//=============================================================================
// Test Runner Task
//=============================================================================

STM32ZERO_DTCM static StaticTask<512> test_runner_task_;

static __NO_RETURN void test_runner_func_(void*)
{
	// Wait for system to stabilize
	vTaskDelay(pdMS_TO_TICKS(100));

	printf("\r\n");
	printf("========================================\r\n");
	printf("STM32ZERO Runtime Test Suite\r\n");
	printf("========================================\r\n");
	printf("\r\n");

	// Run all test modules
	printf("--- Core Tests ---\r\n");
	test_core_runtime();
	printf("\r\n");

	printf("--- SIO Tests ---\r\n");
	test_sio_runtime();
	printf("\r\n");

	printf("--- FreeRTOS Tests ---\r\n");
	test_freertos_runtime();
	printf("\r\n");

	printf("--- USTIM Tests ---\r\n");
	test_ustim_runtime();
	printf("\r\n");

	// Print summary
	printf("========================================\r\n");
	printf("Test Summary\r\n");
	printf("========================================\r\n");
	printf("  Passed: %lu\r\n", test_pass_count);
	printf("  Failed: %lu\r\n", test_fail_count);
	printf("  Total:  %lu\r\n", test_pass_count + test_fail_count);
	printf("========================================\r\n");

	if (test_fail_count == 0) {
		printf("All tests PASSED!\r\n");
	} else {
		printf("Some tests FAILED!\r\n");
	}

	printf("\r\n");
	printf("Press any key to run interactive SIO tests...\r\n");

	// Wait for user input to run interactive tests
	sio::wait(UINT32_MAX);

	printf("\r\n--- Interactive SIO Tests ---\r\n");
	printf("Type a line and press Enter:\r\n");

	char buf[128];
	while (true) {
		int len = sio::readln(buf, sizeof(buf), 5000);
		if (len >= 0) {
			printf("Echo[%d]: %s\r\n", len, buf);
		} else {
			printf("(timeout - type something)\r\n");
		}
	}
}

//=============================================================================
// Test Runner Entry Point
//=============================================================================

extern "C" void test_runner_start(void)
{
	test_runner_task_.create(test_runner_func_, "TEST", Priority::NORMAL);
}

//=============================================================================
// Test Result Access (for external use)
//=============================================================================

extern "C" uint32_t test_get_pass_count(void)
{
	return test_pass_count;
}

extern "C" uint32_t test_get_fail_count(void)
{
	return test_fail_count;
}

void test_report_pass(const char* desc)
{
	printf("[PASS] %s\r\n", desc);
	test_pass_count++;
}

void test_report_fail(const char* desc)
{
	printf("[FAIL] %s\r\n", desc);
	test_fail_count++;
}

void test_report_pass_eq(const char* desc, long expected, long actual)
{
	(void)expected;
	(void)actual;
	printf("[PASS] %s\r\n", desc);
	test_pass_count++;
}

void test_report_fail_eq(const char* desc, long expected, long actual)
{
	printf("[FAIL] %s (expected %ld, got %ld)\r\n", desc, expected, actual);
	test_fail_count++;
}
