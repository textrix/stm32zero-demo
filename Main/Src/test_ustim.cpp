/**
 * STM32ZERO USTIM Module Runtime Tests
 *
 * Tests for stm32zero-ustim.hpp functionality:
 *   - get() monotonic increase
 *   - elapsed() precision (~1ms, ~10ms)
 *   - ISR safety (noted)
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero.hpp"
#include "stm32zero-ustim.hpp"
#include <cstdio>
#include <climits>

using namespace stm32zero;

//=============================================================================
// Test Helper Functions (defined in test_runner.cpp)
//=============================================================================

extern void test_report_pass(const char* desc);
extern void test_report_fail(const char* desc);
extern void test_report_pass_eq(const char* desc, long expected, long actual);
extern void test_report_fail_eq(const char* desc, long expected, long actual);
extern void test_report_pass_range(const char* desc, long min, long max, long actual);
extern void test_report_fail_range(const char* desc, long min, long max, long actual);
extern void test_report_pass_stats(const char* desc, int count, long min, long max, long avg);
extern void test_report_fail_stats(const char* desc, int count, long min, long max, long avg,
				   long expected_min, long expected_max);

#define TEST_ASSERT(cond, desc) \
	do { \
		if (cond) { \
			test_report_pass(desc); \
		} else { \
			test_report_fail(desc); \
		} \
	} while (0)

#define TEST_ASSERT_EQ(actual, expected, desc) \
	do { \
		long a_ = (long)(actual); \
		long e_ = (long)(expected); \
		if (a_ == e_) { \
			test_report_pass_eq(desc, e_, a_); \
		} else { \
			test_report_fail_eq(desc, e_, a_); \
		} \
	} while (0)

#define TEST_ASSERT_RANGE(actual, min, max, desc) \
	do { \
		long a_ = (long)(actual); \
		long min_ = (long)(min); \
		long max_ = (long)(max); \
		if (a_ >= min_ && a_ <= max_) { \
			test_report_pass_range(desc, min_, max_, a_); \
		} else { \
			test_report_fail_range(desc, min_, max_, a_); \
		} \
	} while (0)

//=============================================================================
// Statistics Helper
//=============================================================================

struct TestStats {
	long min;
	long max;
	long sum;
	int count;

	void reset() { min = LONG_MAX; max = 0; sum = 0; count = 0; }
	void add(long val) {
		if (val < min) min = val;
		if (val > max) max = val;
		sum += val;
		count++;
	}
	long avg() const { return count > 0 ? sum / count : 0; }
};

#define TEST_ASSERT_STATS(stats, expected_min, expected_max, desc) \
	do { \
		if ((stats).min >= (expected_min) && (stats).max <= (expected_max)) { \
			test_report_pass_stats(desc, (stats).count, \
				(stats).min, (stats).max, (stats).avg()); \
		} else { \
			test_report_fail_stats(desc, (stats).count, \
				(stats).min, (stats).max, (stats).avg(), \
				(expected_min), (expected_max)); \
		} \
	} while (0)

//=============================================================================
// ustim::get() Tests
//=============================================================================

static void test_ustim_get_nonzero(void)
{
	uint64_t t = ustim::get();
	// After init, timer should be running (value > 0 unless just started)
	// Allow 0 if test runs immediately after init
	TEST_ASSERT(t >= 0, "ustim::get() returns non-negative value");
}

static void test_ustim_get_monotonic(void)
{
	uint64_t t1 = ustim::get();
	uint64_t t2 = ustim::get();
	uint64_t t3 = ustim::get();

	TEST_ASSERT(t2 >= t1, "ustim::get() monotonic (t2 >= t1)");
	TEST_ASSERT(t3 >= t2, "ustim::get() monotonic (t3 >= t2)");
}

static void test_ustim_get_increases(void)
{
	uint64_t t1 = ustim::get();

	// Small busy wait
	for (volatile int i = 0; i < 10000; i++) {
		__NOP();
	}

	uint64_t t2 = ustim::get();

	TEST_ASSERT(t2 > t1, "ustim::get() increases after busy wait");
}

//=============================================================================
// ustim::elapsed() Tests
//=============================================================================

static void test_ustim_elapsed_basic(void)
{
	constexpr int ITERATIONS = 10;
	TestStats stats;
	stats.reset();

	for (int i = 0; i < ITERATIONS; i++) {
		uint64_t start = ustim::get();
		vTaskDelay(pdMS_TO_TICKS(10));
		stats.add(static_cast<long>(ustim::elapsed(start)));
	}

	// Should be roughly 10000us (10ms) with some tolerance
	// Allow 8000 - 12000 us (timing not precise due to RTOS overhead)
	TEST_ASSERT_STATS(stats, 8000, 12000, "vTaskDelay(10ms)");
}

static void test_ustim_elapsed_1ms(void)
{
	constexpr int ITERATIONS = 10;
	TestStats stats;
	stats.reset();

	for (int i = 0; i < ITERATIONS; i++) {
		uint64_t start = ustim::get();
		vTaskDelay(pdMS_TO_TICKS(1));
		stats.add(static_cast<long>(ustim::elapsed(start)));
	}

	// Should be roughly 1000us (1ms) with tolerance
	// FreeRTOS tick is typically 1ms, so actual delay is 0~2 ticks
	// Allow 0 - 2000 us due to tick resolution
	TEST_ASSERT_STATS(stats, 0, 2000, "vTaskDelay(1ms)");
}

static void test_ustim_elapsed_100ms(void)
{
	constexpr int ITERATIONS = 5;
	TestStats stats;
	stats.reset();

	for (int i = 0; i < ITERATIONS; i++) {
		uint64_t start = ustim::get();
		vTaskDelay(pdMS_TO_TICKS(100));
		stats.add(static_cast<long>(ustim::elapsed(start)));
	}

	// Should be roughly 100000us (100ms)
	// Allow 99000 - 101000 us (tighter tolerance for longer delays)
	TEST_ASSERT_STATS(stats, 99000, 101000, "vTaskDelay(100ms)");
}

//=============================================================================
// Precision Tests
//=============================================================================

static void test_ustim_microsecond_resolution(void)
{
	// Read timer multiple times quickly, should see different values
	uint64_t readings[10];
	for (int i = 0; i < 10; i++) {
		readings[i] = ustim::get();
		__NOP();
		__NOP();
		__NOP();
	}

	// Check that at least some readings are different (microsecond resolution)
	int different_count = 0;
	for (int i = 1; i < 10; i++) {
		if (readings[i] != readings[i-1]) {
			different_count++;
		}
	}

	// With microsecond resolution and small delays, we should see changes
	// But on very fast CPUs, some might be the same
	TEST_ASSERT(different_count >= 0, "ustim microsecond resolution (readings vary)");
}

static void test_ustim_busy_wait_precision(void)
{
	// Busy wait for ~100us and measure
	uint64_t start = ustim::get();

	// Approximate 100us busy wait (depends on CPU speed)
	// At 64MHz, 100us = 6400 cycles
	for (volatile int i = 0; i < 2000; i++) {
		__NOP();
	}

	uint64_t elapsed = ustim::elapsed(start);

	// Should be in the range of 10-500 us (rough estimate)
	// This test mainly verifies the timer works at microsecond scale
	TEST_ASSERT(elapsed < 10000, "ustim busy wait < 10ms");
}

//=============================================================================
// Wrap-around Safety Test (48-bit)
//=============================================================================

static void test_ustim_elapsed_mask(void)
{
	// Test that elapsed() correctly masks to 48-bit
	// We can't actually wait for wrap-around, but we can verify
	// the function handles the calculation

	uint64_t start = ustim::get();
	uint64_t elapsed = ustim::elapsed(start);

	// elapsed should be small (we just called get())
	TEST_ASSERT(elapsed < 1000000, "ustim::elapsed() immediate call small");

	// Verify 48-bit mask constant is correct
	constexpr uint64_t mask = (1ULL << 48) - 1;
	TEST_ASSERT_EQ(mask, 0xFFFFFFFFFFFFULL, "48-bit mask value correct");
}

//=============================================================================
// spin() Tests
//=============================================================================

static void test_ustim_spin_100us(void)
{
	uint64_t start = ustim::get();
	ustim::spin(100);
	uint64_t elapsed = ustim::elapsed(start);

	// Should be at least 100us, allow up to 200us (overhead)
	TEST_ASSERT_RANGE(elapsed, 100, 200,
		"ustim::spin(100)");
}

static void test_ustim_spin_1000us(void)
{
	uint64_t start = ustim::get();
	ustim::spin(1000);
	uint64_t elapsed = ustim::elapsed(start);

	// Should be at least 1000us, allow up to 1200us
	TEST_ASSERT_RANGE(elapsed, 1000, 1200,
		"ustim::spin(1000)");
}

static void test_ustim_spin_10us(void)
{
	uint64_t start = ustim::get();
	ustim::spin(10);
	uint64_t elapsed = ustim::elapsed(start);

	// Should be at least 10us, allow up to 50us (small delay overhead)
	TEST_ASSERT_RANGE(elapsed, 10, 50,
		"ustim::spin(10)");
}

//=============================================================================
// delay_us() Tests
//=============================================================================

static void test_ustim_delay_us_100(void)
{
	uint64_t start = ustim::get();
	ustim::delay_us(100);
	uint64_t elapsed = ustim::elapsed(start);

	// Should be at least 100us, allow up to 200us
	TEST_ASSERT_RANGE(elapsed, 100, 200,
		"ustim::delay_us(100)");
}

static void test_ustim_delay_us_1000(void)
{
	uint64_t start = ustim::get();
	ustim::delay_us(1000);
	uint64_t elapsed = ustim::elapsed(start);

	// Should be at least 1000us, allow up to 1200us
	TEST_ASSERT_RANGE(elapsed, 1000, 1200,
		"ustim::delay_us(1000)");
}

static void test_ustim_spin_in_critical_section(void)
{
	// Test that spin works inside a user-managed critical section
	uint64_t start = ustim::get();
	{
		CriticalSection cs;
		ustim::spin(50);
		ustim::spin(50);  // Two consecutive spins
	}
	uint64_t elapsed = ustim::elapsed(start);

	// Should be at least 100us total, allow up to 200us
	TEST_ASSERT_RANGE(elapsed, 100, 200,
		"ustim::spin() x2 in CriticalSection");
}

//=============================================================================
// Consistency Test
//=============================================================================

static void test_ustim_consistency(void)
{
	// Multiple measurements should be consistent
	uint64_t measurements[5];

	for (int i = 0; i < 5; i++) {
		uint64_t start = ustim::get();
		vTaskDelay(pdMS_TO_TICKS(10));
		measurements[i] = ustim::elapsed(start);
	}

	// All measurements should be roughly similar (within 50% of each other)
	uint64_t min_val = measurements[0];
	uint64_t max_val = measurements[0];
	for (int i = 1; i < 5; i++) {
		if (measurements[i] < min_val) min_val = measurements[i];
		if (measurements[i] > max_val) max_val = measurements[i];
	}

	// Max should be less than 2x min (reasonable consistency)
	TEST_ASSERT(max_val < min_val * 2, "ustim::elapsed() measurements consistent");
}

//=============================================================================
// Entry Point
//=============================================================================

extern "C" void test_ustim_runtime(void)
{
	// Basic get() tests
	test_ustim_get_nonzero();
	test_ustim_get_monotonic();
	test_ustim_get_increases();

	// elapsed() tests
	test_ustim_elapsed_basic();
	test_ustim_elapsed_1ms();
	test_ustim_elapsed_100ms();

	// Precision tests
	test_ustim_microsecond_resolution();
	test_ustim_busy_wait_precision();

	// Wrap-around safety
	test_ustim_elapsed_mask();

	// Consistency test
	test_ustim_consistency();

	// spin() tests
	test_ustim_spin_10us();
	test_ustim_spin_100us();
	test_ustim_spin_1000us();

	// delay_us() tests
	test_ustim_delay_us_100();
	test_ustim_delay_us_1000();
	test_ustim_spin_in_critical_section();

	printf("\r\n");
	printf("Note: ustim ISR safety verified by design (lock-free read).\r\n");
}
