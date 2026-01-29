/**
 * Test: TIM<N> template usage
 *
 * Demonstrates using TIM<N> template for timer metadata access.
 * Uses integer N as template parameter (not TIM_TypeDef* which is not constexpr).
 */

#include "main.h"
#include "stm32zero-tim.hpp"
#include <cstdint>

using namespace stm32zero;

// Test 1: Single timer access via TIM<N>
template<typename T>
struct TimerTest1 {
	static uint32_t get_cnt() { return T::ptr()->CNT; }
	static void start() { T::ptr()->CR1 |= TIM_CR1_CEN; }
	static constexpr int bits = T::bits;
};

// Test 2: Two timers as template parameters
template<typename Low, typename High>
struct TimerTest2 {
	static uint64_t get() {
		return (static_cast<uint64_t>(High::ptr()->CNT) << Low::bits) |
		       Low::ptr()->CNT;
	}
	static constexpr int total_bits = Low::bits + High::bits;
};

// Instantiation test
using test1 = TimerTest1<TIM<2>>;
#ifdef TIM4
using test2 = TimerTest2<TIM<2>, TIM<4>>;
#endif

// Compile-time verification
static_assert(test1::bits == 32, "TIM2 should be 32-bit");
#ifdef TIM4
static_assert(test2::total_bits == 48, "TIM2+TIM4 should be 48-bit");
#endif

// Force instantiation
volatile uint32_t dummy1 = 0;
#ifdef TIM4
volatile uint64_t dummy2 = 0;
#endif

void test_tim_template()
{
	dummy1 = test1::get_cnt();
#ifdef TIM4
	dummy2 = test2::get();
#endif
}
