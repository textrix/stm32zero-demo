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
#include <cstring>

using namespace stm32zero;
using namespace stm32zero::freertos;

//=============================================================================
// Test Framework
//=============================================================================

static uint32_t test_pass_count = 0;
static uint32_t test_fail_count = 0;
static char fmt_buf_[256];

#define TEST_ASSERT(cond, desc) \
	do { \
		if (cond) { \
			sio::writef(fmt_buf_, "[PASS] %s\r\n", desc); \
			test_pass_count++; \
		} else { \
			sio::writef(fmt_buf_, "[FAIL] %s\r\n", desc); \
			test_fail_count++; \
		} \
	} while (0)

#define TEST_ASSERT_EQ(actual, expected, desc) \
	do { \
		if ((actual) == (expected)) { \
			sio::writef(fmt_buf_, "[PASS] %s\r\n", desc); \
			test_pass_count++; \
		} else { \
			sio::writef(fmt_buf_, "[FAIL] %s (expected %ld, got %ld)\r\n", \
				    desc, (long)(expected), (long)(actual)); \
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
extern "C" void test_fdcan_runtime(void);

//=============================================================================
// Test Runner Task
//=============================================================================

STM32ZERO_DTCM static StaticTask<1024> test_runner_task_;

static __NO_RETURN void test_runner_func_(void*)
{
	// Wait for system to stabilize
	vTaskDelay(pdMS_TO_TICKS(100));


	sio::writef(fmt_buf_, "--- FDCAN Tests ---\r\n");
	//test_fdcan_runtime();
	sio::writef(fmt_buf_, "\r\n");

	sio::writef(fmt_buf_, "\r\n");
	sio::writef(fmt_buf_, "========================================\r\n");
	sio::writef(fmt_buf_, "STM32ZERO Runtime Test Suite\r\n");
	sio::writef(fmt_buf_, "========================================\r\n");
	sio::writef(fmt_buf_, "\r\n");

	// Run all test modules
	sio::writef(fmt_buf_, "--- Core Tests ---\r\n");
	test_core_runtime();
	sio::writef(fmt_buf_, "\r\n");

	sio::writef(fmt_buf_, "--- SIO Tests ---\r\n");
	test_sio_runtime();
	sio::writef(fmt_buf_, "\r\n");

	sio::writef(fmt_buf_, "--- FreeRTOS Tests ---\r\n");
	test_freertos_runtime();
	sio::writef(fmt_buf_, "\r\n");

	sio::writef(fmt_buf_, "--- USTIM Tests ---\r\n");
	test_ustim_runtime();
	sio::writef(fmt_buf_, "\r\n");

	// Print summary
	sio::writef(fmt_buf_, "========================================\r\n");
	sio::writef(fmt_buf_, "Test Summary\r\n");
	sio::writef(fmt_buf_, "========================================\r\n");
	sio::writef(fmt_buf_, "  Passed: %lu\r\n", test_pass_count);
	sio::writef(fmt_buf_, "  Failed: %lu\r\n", test_fail_count);
	sio::writef(fmt_buf_, "  Total:  %lu\r\n", test_pass_count + test_fail_count);
	sio::writef(fmt_buf_, "========================================\r\n");

	if (test_fail_count == 0) {
		sio::writef(fmt_buf_, "All tests PASSED!\r\n");
	} else {
		sio::writef(fmt_buf_, "Some tests FAILED!\r\n");
	}

	sio::writef(fmt_buf_, "\r\n");
	sio::writef(fmt_buf_, "Press any key to run interactive SIO tests...\r\n");

	// Wait for user input to run interactive tests
	sio::wait_readable(UINT32_MAX);

	sio::writef(fmt_buf_, "\r\n--- Interactive SIO Tests ---\r\n");
	sio::writef(fmt_buf_, "Type a line and press Enter:\r\n");

	static char buf[128];
	while (true) {
		auto result = sio::readln(buf, sizeof(buf), 5000);
		if (result) {
			sio::writef(fmt_buf_, "Echo[%d]: %s\r\n", result.count, buf);
		} else {
			sio::writef(fmt_buf_, "(timeout - type something)\r\n");
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
	sio::writef(fmt_buf_, "[PASS] %s\r\n", desc);
	test_pass_count++;
}

void test_report_fail(const char* desc)
{
	sio::writef(fmt_buf_, "[FAIL] %s\r\n", desc);
	test_fail_count++;
}

void test_report_pass_eq(const char* desc, long expected, long actual)
{
	(void)expected;
	(void)actual;
	sio::writef(fmt_buf_, "[PASS] %s\r\n", desc);
	test_pass_count++;
}

void test_report_fail_eq(const char* desc, long expected, long actual)
{
	sio::writef(fmt_buf_, "[FAIL] %s (expected %ld, got %ld)\r\n", desc, expected, actual);
	test_fail_count++;
}

void test_report_pass_range(const char* desc, long min, long max, long actual)
{
	sio::writef(fmt_buf_, "[PASS] %s (expected %ld~%ld, actual %ld)\r\n", desc, min, max, actual);
	test_pass_count++;
}

void test_report_fail_range(const char* desc, long min, long max, long actual)
{
	sio::writef(fmt_buf_, "[FAIL] %s (expected %ld~%ld, actual %ld)\r\n", desc, min, max, actual);
	test_fail_count++;
}

void test_report_pass_stats(const char* desc, int count, long min, long max, long avg)
{
	sio::writef(fmt_buf_, "[PASS] %s x%d (min %ld, max %ld, avg %ld)\r\n", desc, count, min, max, avg);
	test_pass_count++;
}

void test_report_fail_stats(const char* desc, int count, long min, long max, long avg,
			    long expected_min, long expected_max)
{
	sio::writef(fmt_buf_, "[FAIL] %s x%d (min %ld, max %ld, avg %ld) expected %ld~%ld\r\n",
		    desc, count, min, max, avg, expected_min, expected_max);
	test_fail_count++;
}
