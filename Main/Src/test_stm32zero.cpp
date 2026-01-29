/**
 * STM32ZERO Library Compile-Time Tests
 *
 * All tests here are verified at compile time via static_assert.
 * If this file compiles, all compile-time tests pass.
 */

#include "main.h"
#include "stm32zero.hpp"
#include "stm32zero-freertos.hpp"

using namespace stm32zero;
using namespace stm32zero::freertos;

//=============================================================================
// DMA Buffer Test Instances
//=============================================================================

STM32ZERO_DMA_TX DmaBuffer<64> tx_buf;
STM32ZERO_DMA_RX DmaBuffer<128> rx_buf;

//=============================================================================
// dimof() Tests
//=============================================================================

// 1D array
static int test_array_1d[10];
static_assert(dimof(test_array_1d) == 10, "dimof 1D array");

// 2D array
static int test_array_2d[3][4];
static_assert(dimof(test_array_2d) == 3, "dimof 2D array - first dimension");
static_assert(dimof(test_array_2d[0]) == 4, "dimof 2D array - second dimension");

// 3D array
static int test_array_3d[2][3][5];
static_assert(dimof(test_array_3d) == 2, "dimof 3D array - first dimension");
static_assert(dimof(test_array_3d[0]) == 3, "dimof 3D array - second dimension");
static_assert(dimof(test_array_3d[0][0]) == 5, "dimof 3D array - third dimension");

// Zero-size array is not valid in C++, so we skip that test

// Character array
static char test_char_array[] = "Hello";
static_assert(dimof(test_char_array) == 6, "dimof char array (includes null terminator)");

//=============================================================================
// is_power_of_2() Tests
//=============================================================================

// True cases
static_assert(is_power_of_2(1), "1 is 2^0");
static_assert(is_power_of_2(2), "2 is 2^1");
static_assert(is_power_of_2(4), "4 is 2^2");
static_assert(is_power_of_2(8), "8 is 2^3");
static_assert(is_power_of_2(16), "16 is 2^4");
static_assert(is_power_of_2(32), "32 is 2^5");
static_assert(is_power_of_2(64), "64 is 2^6");
static_assert(is_power_of_2(128), "128 is 2^7");
static_assert(is_power_of_2(256), "256 is 2^8");
static_assert(is_power_of_2(1024), "1024 is 2^10");
static_assert(is_power_of_2(4096), "4096 is 2^12");
static_assert(is_power_of_2(65536), "65536 is 2^16");

// False cases
static_assert(!is_power_of_2(0), "0 is not power of 2");
static_assert(!is_power_of_2(3), "3 is not power of 2");
static_assert(!is_power_of_2(5), "5 is not power of 2");
static_assert(!is_power_of_2(6), "6 is not power of 2");
static_assert(!is_power_of_2(7), "7 is not power of 2");
static_assert(!is_power_of_2(9), "9 is not power of 2");
static_assert(!is_power_of_2(10), "10 is not power of 2");
static_assert(!is_power_of_2(12), "12 is not power of 2");
static_assert(!is_power_of_2(15), "15 is not power of 2");
static_assert(!is_power_of_2(100), "100 is not power of 2");
static_assert(!is_power_of_2(1000), "1000 is not power of 2");

//=============================================================================
// align_up<N>() Tests
//=============================================================================

// Alignment to 4
static_assert(align_up<4>(0) == 0, "0 aligned to 4");
static_assert(align_up<4>(1) == 4, "1 aligned to 4");
static_assert(align_up<4>(2) == 4, "2 aligned to 4");
static_assert(align_up<4>(3) == 4, "3 aligned to 4");
static_assert(align_up<4>(4) == 4, "4 aligned to 4 (exact)");
static_assert(align_up<4>(5) == 8, "5 aligned to 4");

// Alignment to 8
static_assert(align_up<8>(0) == 0, "0 aligned to 8");
static_assert(align_up<8>(1) == 8, "1 aligned to 8");
static_assert(align_up<8>(7) == 8, "7 aligned to 8");
static_assert(align_up<8>(8) == 8, "8 aligned to 8 (exact)");
static_assert(align_up<8>(9) == 16, "9 aligned to 8");

// Alignment to 32
static_assert(align_up<32>(0) == 0, "0 aligned to 32");
static_assert(align_up<32>(1) == 32, "1 aligned to 32");
static_assert(align_up<32>(31) == 32, "31 aligned to 32");
static_assert(align_up<32>(32) == 32, "32 aligned to 32 (exact)");
static_assert(align_up<32>(33) == 64, "33 aligned to 32");
static_assert(align_up<32>(100) == 128, "100 aligned to 32");

// Alignment to 64
static_assert(align_up<64>(0) == 0, "0 aligned to 64");
static_assert(align_up<64>(1) == 64, "1 aligned to 64");
static_assert(align_up<64>(63) == 64, "63 aligned to 64");
static_assert(align_up<64>(64) == 64, "64 aligned to 64 (exact)");
static_assert(align_up<64>(65) == 128, "65 aligned to 64");

//=============================================================================
// cache_align() Tests
//=============================================================================

#ifdef __SCB_DCACHE_LINE_SIZE
static_assert(cache_line_size == 32, "H7 cache line is 32 bytes");

static_assert(cache_align(0) == 0, "0 cache aligned");
static_assert(cache_align(1) == 32, "1 cache aligned");
static_assert(cache_align(31) == 32, "31 cache aligned");
static_assert(cache_align(32) == 32, "32 cache aligned (exact)");
static_assert(cache_align(33) == 64, "33 cache aligned");
static_assert(cache_align(64) == 64, "64 cache aligned (exact)");
static_assert(cache_align(65) == 96, "65 cache aligned");
static_assert(cache_align(100) == 128, "100 cache aligned");
static_assert(cache_align(128) == 128, "128 cache aligned (exact)");
#endif

//=============================================================================
// DmaBuffer Tests
//=============================================================================

// size() test
static_assert(DmaBuffer<1>::size() == 1, "DmaBuffer<1>::size()");
static_assert(DmaBuffer<32>::size() == 32, "DmaBuffer<32>::size()");
static_assert(DmaBuffer<50>::size() == 50, "DmaBuffer<50>::size()");
static_assert(DmaBuffer<64>::size() == 64, "DmaBuffer<64>::size()");
static_assert(DmaBuffer<100>::size() == 100, "DmaBuffer<100>::size()");
static_assert(DmaBuffer<128>::size() == 128, "DmaBuffer<128>::size()");

// aligned_size() test (cache line = 32)
#ifdef __SCB_DCACHE_LINE_SIZE
static_assert(DmaBuffer<1>::aligned_size() == 32, "DmaBuffer<1>::aligned_size()");
static_assert(DmaBuffer<32>::aligned_size() == 32, "DmaBuffer<32>::aligned_size()");
static_assert(DmaBuffer<33>::aligned_size() == 64, "DmaBuffer<33>::aligned_size()");
static_assert(DmaBuffer<50>::aligned_size() == 64, "DmaBuffer<50>::aligned_size()");
static_assert(DmaBuffer<64>::aligned_size() == 64, "DmaBuffer<64>::aligned_size()");
static_assert(DmaBuffer<65>::aligned_size() == 96, "DmaBuffer<65>::aligned_size()");
static_assert(DmaBuffer<100>::aligned_size() == 128, "DmaBuffer<100>::aligned_size()");
static_assert(DmaBuffer<128>::aligned_size() == 128, "DmaBuffer<128>::aligned_size()");
#endif

//=============================================================================
// FreeRTOS StaticTask Tests
//=============================================================================

static_assert(StaticTask<128>::stack_words() == 128, "StaticTask<128>::stack_words()");
static_assert(StaticTask<256>::stack_words() == 256, "StaticTask<256>::stack_words()");
static_assert(StaticTask<512>::stack_words() == 512, "StaticTask<512>::stack_words()");

static_assert(StaticTask<128>::stack_bytes() == 128 * sizeof(StackType_t), "StaticTask<128>::stack_bytes()");
static_assert(StaticTask<256>::stack_bytes() == 256 * sizeof(StackType_t), "StaticTask<256>::stack_bytes()");
static_assert(StaticTask<512>::stack_bytes() == 512 * sizeof(StackType_t), "StaticTask<512>::stack_bytes()");

// On 32-bit system, sizeof(StackType_t) == 4
static_assert(StaticTask<256>::stack_bytes() == 1024, "StaticTask<256> = 1024 bytes on 32-bit");

//=============================================================================
// FreeRTOS StaticQueue Tests
//=============================================================================

static_assert(StaticQueue<4, 10>::item_size() == 4, "StaticQueue<4,10>::item_size()");
static_assert(StaticQueue<4, 10>::length() == 10, "StaticQueue<4,10>::length()");

static_assert(StaticQueue<8, 20>::item_size() == 8, "StaticQueue<8,20>::item_size()");
static_assert(StaticQueue<8, 20>::length() == 20, "StaticQueue<8,20>::length()");

static_assert(StaticQueue<sizeof(int), 100>::item_size() == sizeof(int), "StaticQueue<sizeof(int),100>::item_size()");
static_assert(StaticQueue<sizeof(int), 100>::length() == 100, "StaticQueue<sizeof(int),100>::length()");

//=============================================================================
// FreeRTOS Priority Conversion Tests
//=============================================================================

// +Priority should return UBaseType_t
static_assert(+Priority::IDLE == 0, "+Priority::IDLE");
static_assert(+Priority::LOW == 1, "+Priority::LOW");
static_assert(+Priority::BELOW_NORMAL == 2, "+Priority::BELOW_NORMAL");
static_assert(+Priority::NORMAL == 3, "+Priority::NORMAL");
static_assert(+Priority::ABOVE_NORMAL == 4, "+Priority::ABOVE_NORMAL");
static_assert(+Priority::HIGH == 5, "+Priority::HIGH");
static_assert(+Priority::REALTIME == 6, "+Priority::REALTIME");

//=============================================================================
// Dummy Runtime Function (to satisfy linker if needed)
//=============================================================================

extern "C" void test_stm32zero(void)
{
	// Runtime buffer access test (just to prevent unused variable warnings)
	tx_buf[0] = 0x55;
	tx_buf[1] = 0xAA;
	rx_buf[0] = 0x00;
}
