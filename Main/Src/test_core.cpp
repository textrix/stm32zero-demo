/**
 * STM32ZERO Core Module Runtime Tests
 *
 * Tests for stm32zero.hpp runtime functionality:
 *   - DmaBuffer read/write
 *   - CriticalSection RAII behavior
 *   - is_in_isr() context detection
 *   - wait_until() (non-RTOS only, skipped here)
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero.hpp"
#include <cstdio>
#include <cstring>

using namespace stm32zero;

//=============================================================================
// Test Helper Functions (defined in test_runner.cpp)
//=============================================================================

extern void test_report_pass(const char* desc);
extern void test_report_fail(const char* desc);
extern void test_report_pass_eq(const char* desc, long expected, long actual);
extern void test_report_fail_eq(const char* desc, long expected, long actual);

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

//=============================================================================
// Test Data
//=============================================================================

STM32ZERO_DMA_TX static DmaBuffer<64> test_tx_buf_;
STM32ZERO_DMA_RX static DmaBuffer<128> test_rx_buf_;

//=============================================================================
// DmaBuffer Tests
//=============================================================================

static void test_dma_buffer_write_read(void)
{
	// Write test pattern
	for (size_t i = 0; i < test_tx_buf_.size(); i++) {
		test_tx_buf_[i] = static_cast<uint8_t>(i & 0xFF);
	}

	// Verify pattern
	bool pattern_ok = true;
	for (size_t i = 0; i < test_tx_buf_.size(); i++) {
		if (test_tx_buf_[i] != static_cast<uint8_t>(i & 0xFF)) {
			pattern_ok = false;
			break;
		}
	}
	TEST_ASSERT(pattern_ok, "DmaBuffer write/read pattern");
}

static void test_dma_buffer_data_pointer(void)
{
	volatile uint8_t* ptr = test_tx_buf_.data();
	TEST_ASSERT(ptr != nullptr, "DmaBuffer::data() not null");

	// Write via data pointer
	ptr[0] = 0xAA;
	ptr[1] = 0x55;

	TEST_ASSERT_EQ(test_tx_buf_[0], 0xAA, "DmaBuffer data pointer write [0]");
	TEST_ASSERT_EQ(test_tx_buf_[1], 0x55, "DmaBuffer data pointer write [1]");
}

static void test_dma_buffer_alignment(void)
{
	// Check that data is cache-line aligned (32 bytes for H7)
	uintptr_t addr_tx = reinterpret_cast<uintptr_t>(test_tx_buf_.data());
	uintptr_t addr_rx = reinterpret_cast<uintptr_t>(test_rx_buf_.data());

	TEST_ASSERT((addr_tx % 32) == 0, "DmaBuffer TX cache-line aligned (32 bytes)");
	TEST_ASSERT((addr_rx % 32) == 0, "DmaBuffer RX cache-line aligned (32 bytes)");
}

//=============================================================================
// CriticalSection Tests
//=============================================================================

static volatile uint32_t cs_test_counter_ = 0;

static void test_critical_section_basic(void)
{
	cs_test_counter_ = 0;

	{
		CriticalSection cs;
		cs_test_counter_++;
		// Verify counter incremented inside critical section
		TEST_ASSERT_EQ(cs_test_counter_, 1, "CriticalSection counter increment");
	}

	// After exiting scope, critical section should be released
	// (We can't easily verify this without ISR, but at least verify no crash)
	TEST_ASSERT(true, "CriticalSection RAII exit (no crash)");
}

static void test_critical_section_nested(void)
{
	cs_test_counter_ = 0;

	{
		CriticalSection cs1;
		cs_test_counter_++;

		{
			CriticalSection cs2;
			cs_test_counter_++;
		}

		cs_test_counter_++;
	}

	TEST_ASSERT_EQ(cs_test_counter_, 3, "CriticalSection nested increment");
}

//=============================================================================
// is_in_isr() Tests
//=============================================================================

static void test_is_in_isr_task_context(void)
{
	// When called from a FreeRTOS task, should return false
	bool in_isr = is_in_isr();
	TEST_ASSERT(!in_isr, "is_in_isr() false in task context");
}

// Note: Testing is_in_isr() returning true requires actual ISR context
// which would need hardware interrupt. We verify it returns false in task.

//=============================================================================
// Runtime Version of align_up (optional, already tested at compile-time)
//=============================================================================

static void test_align_up_runtime(void)
{
	// Runtime version with variable alignment
	TEST_ASSERT_EQ(align_up(100, 32), 128, "align_up(100, 32) runtime");
	TEST_ASSERT_EQ(align_up(32, 32), 32, "align_up(32, 32) runtime");
	TEST_ASSERT_EQ(align_up(1, 64), 64, "align_up(1, 64) runtime");
}

//=============================================================================
// Entry Point
//=============================================================================

extern "C" void test_core_runtime(void)
{
	// DmaBuffer tests
	test_dma_buffer_write_read();
	test_dma_buffer_data_pointer();
	test_dma_buffer_alignment();

	// CriticalSection tests
	test_critical_section_basic();
	test_critical_section_nested();

	// is_in_isr() tests
	test_is_in_isr_task_context();

	// Runtime align_up tests
	test_align_up_runtime();
}
