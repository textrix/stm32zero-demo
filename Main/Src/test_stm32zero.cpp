/**
 * STM32ZERO Library Test
 */

#include "main.h"		// HAL headers
#include "stm32zero.hpp"
#include "stm32zero-debug.hpp"

using namespace stm32zero;

// Debug buffer instance (4KB dual buffer)
STM32ZERO_DEBUG_DEFINE(4096);

// DMA buffer test
STM32ZERO_DMA_TX DmaBuffer<64> tx_buf;
STM32ZERO_DMA_RX DmaBuffer<128> rx_buf;

// Compile-time tests
static_assert(is_power_of_2(32), "32 is power of 2");
static_assert(is_power_of_2(64), "64 is power of 2");
static_assert(!is_power_of_2(0), "0 is not power of 2");
static_assert(!is_power_of_2(3), "3 is not power of 2");

static_assert(cache_line_size == 32, "H7 cache line is 32 bytes");
static_assert(cache_align(1) == 32, "1 aligned to 32");
static_assert(cache_align(32) == 32, "32 aligned to 32");
static_assert(cache_align(33) == 64, "33 aligned to 64");
static_assert(cache_align(100) == 128, "100 aligned to 128");

static_assert(align_up<32>(100) == 128, "align_up test");
static_assert(align_up<4>(5) == 8, "align_up test 2");

static_assert(DmaBuffer<64>::size() == 64, "DmaBuffer size");
static_assert(DmaBuffer<64>::aligned_size() == 64, "DmaBuffer aligned_size");
static_assert(DmaBuffer<50>::size() == 50, "DmaBuffer size 50");
static_assert(DmaBuffer<50>::aligned_size() == 64, "DmaBuffer 50 aligned to 64");

// dimof test
static int test_array[10];
static_assert(dimof(test_array) == 10, "dimof test");

/**
 * Initialize debug output
 *
 * Call after MX_USARTx_UART_Init() in main.c
 *
 * @param huart Pointer to UART handle (e.g., &huart3)
 */
extern "C" void debug_init(UART_HandleTypeDef* huart)
{
	debug::buffer.set_uart(huart);
}

/**
 * DMA TX complete callback
 *
 * Call from HAL_UART_TxCpltCallback() in main.c or stm32h7xx_it.c
 *
 * @param huart Pointer to UART handle
 */
extern "C" void debug_tx_complete(UART_HandleTypeDef* huart)
{
	(void)huart;
	debug::buffer.tx_complete_isr();
}

extern "C" void test_stm32zero(void)
{
	tx_buf[0] = 0x55;
	tx_buf[1] = 0xAA;

	rx_buf[0] = 0x00;
}
