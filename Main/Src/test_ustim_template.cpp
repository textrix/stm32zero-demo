/**
 * STM32ZERO USTIM Template API Tests
 *
 * Tests for template-based Ustim:
 *   - Ustim<TIM<5>, TIM<8>> (32+16 mode)
 *   - Ustim<TIM<3>, TIM<4>, TIM<12>> (16+16+16 mode)
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero-ustim.hpp"
#include "stm32zero-tim.hpp"
#include <cstdio>

using namespace stm32zero;

//=============================================================================
// Test Helper Functions (defined in test_runner.cpp)
//=============================================================================

extern void test_report_pass(const char* desc);
extern void test_report_fail(const char* desc);

#define TEST_ASSERT(cond, desc) \
	do { \
		if (cond) { \
			test_report_pass(desc); \
		} else { \
			test_report_fail(desc); \
		} \
	} while (0)

//=============================================================================
// Compile-time tests (static_assert)
//=============================================================================

static_assert(TIM<2>::bits == 32, "TIM2 must be 32-bit");
static_assert(TIM<3>::bits == 16, "TIM3 must be 16-bit");
static_assert(TIM<4>::bits == 16, "TIM4 must be 16-bit");
static_assert(TIM<5>::bits == 32, "TIM5 must be 32-bit");
static_assert(TIM<8>::bits == 16, "TIM8 must be 16-bit");
static_assert(TIM<12>::bits == 16, "TIM12 must be 16-bit");

//=============================================================================
// Timer configurations
//=============================================================================

// 32+16 mode: TIM5(32-bit) -> TIM8(16-bit)
using ustim_2t = Ustim<TIM<5>, TIM<8>>;

// 16+16+16 mode: TIM3 -> TIM4 -> TIM12
using ustim_3t = Ustim<TIM<3>, TIM<4>, TIM<12>>;

//=============================================================================
// 2-Timer Mode Tests (32+16)
//=============================================================================

static void test_ustim_2t_get_monotonic(void)
{
	uint64_t t1 = ustim_2t::get();
	uint64_t t2 = ustim_2t::get();
	uint64_t t3 = ustim_2t::get();

	TEST_ASSERT(t2 >= t1, "ustim_2t::get() monotonic (t2 >= t1)");
	TEST_ASSERT(t3 >= t2, "ustim_2t::get() monotonic (t3 >= t2)");
}

static void test_ustim_2t_get_increases(void)
{
	uint64_t t1 = ustim_2t::get();

	for (volatile int i = 0; i < 10000; i++) {
		__NOP();
	}

	uint64_t t2 = ustim_2t::get();

	TEST_ASSERT(t2 > t1, "ustim_2t::get() increases after busy wait");
}

static void test_ustim_2t_elapsed_10ms(void)
{
	uint64_t start = ustim_2t::get();
	vTaskDelay(pdMS_TO_TICKS(10));
	uint64_t elapsed = ustim_2t::elapsed(start);

	TEST_ASSERT(elapsed >= 8000 && elapsed <= 15000,
		"ustim_2t::elapsed() ~10ms (8000-15000 us)");
}

static void test_ustim_2t_elapsed_100ms(void)
{
	uint64_t start = ustim_2t::get();
	vTaskDelay(pdMS_TO_TICKS(100));
	uint64_t elapsed = ustim_2t::elapsed(start);

	TEST_ASSERT(elapsed >= 90000 && elapsed <= 120000,
		"ustim_2t::elapsed() ~100ms (90000-120000 us)");
}

//=============================================================================
// 3-Timer Mode Tests (16+16+16)
//=============================================================================

static void test_ustim_3t_get_monotonic(void)
{
	uint64_t t1 = ustim_3t::get();
	uint64_t t2 = ustim_3t::get();
	uint64_t t3 = ustim_3t::get();

	TEST_ASSERT(t2 >= t1, "ustim_3t::get() monotonic (t2 >= t1)");
	TEST_ASSERT(t3 >= t2, "ustim_3t::get() monotonic (t3 >= t2)");
}

static void test_ustim_3t_get_increases(void)
{
	uint64_t t1 = ustim_3t::get();

	for (volatile int i = 0; i < 10000; i++) {
		__NOP();
	}

	uint64_t t2 = ustim_3t::get();

	TEST_ASSERT(t2 > t1, "ustim_3t::get() increases after busy wait");
}

static void test_ustim_3t_elapsed_10ms(void)
{
	uint64_t start = ustim_3t::get();
	vTaskDelay(pdMS_TO_TICKS(10));
	uint64_t elapsed = ustim_3t::elapsed(start);

	TEST_ASSERT(elapsed >= 8000 && elapsed <= 15000,
		"ustim_3t::elapsed() ~10ms (8000-15000 us)");
}

static void test_ustim_3t_elapsed_100ms(void)
{
	uint64_t start = ustim_3t::get();
	vTaskDelay(pdMS_TO_TICKS(100));
	uint64_t elapsed = ustim_3t::elapsed(start);

	TEST_ASSERT(elapsed >= 90000 && elapsed <= 120000,
		"ustim_3t::elapsed() ~100ms (90000-120000 us)");
}

//=============================================================================
// Cross-mode comparison test
//=============================================================================

static void test_ustim_both_modes_similar(void)
{
	uint64_t start_2t = ustim_2t::get();
	uint64_t start_3t = ustim_3t::get();

	vTaskDelay(pdMS_TO_TICKS(50));

	uint64_t elapsed_2t = ustim_2t::elapsed(start_2t);
	uint64_t elapsed_3t = ustim_3t::elapsed(start_3t);

	// Both should measure ~50ms, within 10% of each other
	int64_t diff = (int64_t)elapsed_2t - (int64_t)elapsed_3t;
	if (diff < 0) diff = -diff;

	TEST_ASSERT(diff < 5000, "2t and 3t modes measure similar time (<5ms diff)");
}

//=============================================================================
// Entry Point
//=============================================================================

extern "C" void test_ustim_template(void)
{
	// Initialize both timer chains
	ustim_2t::init();
	ustim_3t::init();

	printf("\r\n--- 2-Timer Mode (32+16: TIM5->TIM8) ---\r\n");
	test_ustim_2t_get_monotonic();
	test_ustim_2t_get_increases();
	test_ustim_2t_elapsed_10ms();
	test_ustim_2t_elapsed_100ms();

	printf("\r\n--- 3-Timer Mode (16+16+16: TIM3->TIM4->TIM12) ---\r\n");
	test_ustim_3t_get_monotonic();
	test_ustim_3t_get_increases();
	test_ustim_3t_elapsed_10ms();
	test_ustim_3t_elapsed_100ms();

	printf("\r\n--- Cross-mode comparison ---\r\n");
	test_ustim_both_modes_similar();
}
