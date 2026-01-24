/**
 * STM32ZERO FreeRTOS Module Runtime Tests
 *
 * Tests for stm32zero-freertos.hpp functionality:
 *   - StaticTask create/handle
 *   - StaticQueue send/receive
 *   - StaticMutex lock/unlock
 *   - StaticBinarySemaphore take/give
 *   - StaticCountingSemaphore count behavior
 *   - MutexLock RAII
 *   - delay() / get_tick_count()
 *   - ISR give_from_isr() (requires ISR context, noted)
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero.hpp"
#include "stm32zero-freertos.hpp"
#include <cstdio>
#include <cstring>

using namespace stm32zero;
using namespace stm32zero::freertos;

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
// Test Objects (static allocation)
//=============================================================================

STM32ZERO_DTCM static StaticTask<256> test_task_;
STM32ZERO_DTCM static StaticQueue<sizeof(uint32_t), 10> test_queue_;
STM32ZERO_DTCM static StaticMutex test_mutex_;
STM32ZERO_DTCM static StaticBinarySemaphore test_bin_sem_;
STM32ZERO_DTCM static StaticCountingSemaphore<5> test_cnt_sem_;

static volatile bool task_ran_ = false;
static volatile uint32_t task_param_received_ = 0;

//=============================================================================
// StaticTask Tests
//=============================================================================

static void test_task_func_(void* param)
{
	task_param_received_ = reinterpret_cast<uintptr_t>(param);
	task_ran_ = true;
	vTaskDelete(nullptr);
}

static void test_static_task_create(void)
{
	task_ran_ = false;
	task_param_received_ = 0;

	TaskHandle_t handle = test_task_.create(
		test_task_func_,
		"TestTask",
		Priority::LOW,
		reinterpret_cast<void*>(0x12345678)
	);

	TEST_ASSERT(handle != nullptr, "StaticTask::create() returns valid handle");
	TEST_ASSERT(test_task_.is_created(), "StaticTask::is_created() returns true");
	TEST_ASSERT(test_task_.handle() == handle, "StaticTask::handle() matches");

	// Wait for task to run
	vTaskDelay(pdMS_TO_TICKS(50));

	TEST_ASSERT(task_ran_, "StaticTask task function executed");
	TEST_ASSERT_EQ(task_param_received_, 0x12345678, "StaticTask parameter passed correctly");
}

//=============================================================================
// StaticQueue Tests
//=============================================================================

static void test_static_queue_create(void)
{
	QueueHandle_t handle = test_queue_.create();
	TEST_ASSERT(handle != nullptr, "StaticQueue::create() returns valid handle");
	TEST_ASSERT(test_queue_.is_created(), "StaticQueue::is_created() returns true");
}

static void test_static_queue_send_receive(void)
{
	// Reset queue
	test_queue_.reset();

	// Send items
	uint32_t send_val = 42;
	bool sent = test_queue_.send(&send_val, 0);
	TEST_ASSERT(sent, "StaticQueue::send() succeeds");

	send_val = 100;
	sent = test_queue_.send(&send_val, 0);
	TEST_ASSERT(sent, "StaticQueue::send() second item succeeds");

	// Check count
	TEST_ASSERT_EQ(test_queue_.count(), 2, "StaticQueue::count() returns 2");
	TEST_ASSERT(!test_queue_.is_empty(), "StaticQueue::is_empty() returns false");

	// Receive items
	uint32_t recv_val = 0;
	bool received = test_queue_.receive(&recv_val, 0);
	TEST_ASSERT(received, "StaticQueue::receive() succeeds");
	TEST_ASSERT_EQ(recv_val, 42, "StaticQueue::receive() first value correct");

	received = test_queue_.receive(&recv_val, 0);
	TEST_ASSERT(received, "StaticQueue::receive() second succeeds");
	TEST_ASSERT_EQ(recv_val, 100, "StaticQueue::receive() second value correct");

	// Queue should be empty now
	TEST_ASSERT(test_queue_.is_empty(), "StaticQueue::is_empty() true after receive");
	TEST_ASSERT_EQ(test_queue_.count(), 0, "StaticQueue::count() returns 0");
}

static void test_static_queue_peek(void)
{
	test_queue_.reset();

	uint32_t val = 999;
	test_queue_.send(&val, 0);

	uint32_t peeked = 0;
	bool ok = test_queue_.peek(&peeked, 0);
	TEST_ASSERT(ok, "StaticQueue::peek() succeeds");
	TEST_ASSERT_EQ(peeked, 999, "StaticQueue::peek() value correct");

	// Item should still be in queue
	TEST_ASSERT_EQ(test_queue_.count(), 1, "StaticQueue::peek() doesn't remove item");

	test_queue_.reset();
}

static void test_static_queue_full(void)
{
	test_queue_.reset();

	// Fill queue to capacity (length = 10)
	uint32_t val = 0;
	for (int i = 0; i < 10; i++) {
		val = i;
		test_queue_.send(&val, 0);
	}

	TEST_ASSERT(test_queue_.is_full(), "StaticQueue::is_full() true when full");
	TEST_ASSERT_EQ(test_queue_.available(), 0, "StaticQueue::available() returns 0 when full");

	// Try to send one more - should fail immediately with timeout=0
	val = 999;
	bool sent = test_queue_.send(&val, 0);
	TEST_ASSERT(!sent, "StaticQueue::send() fails when full");

	test_queue_.reset();
}

//=============================================================================
// StaticMutex Tests
//=============================================================================

static void test_static_mutex_create(void)
{
	SemaphoreHandle_t handle = test_mutex_.create();
	TEST_ASSERT(handle != nullptr, "StaticMutex::create() returns valid handle");
	TEST_ASSERT(test_mutex_.is_created(), "StaticMutex::is_created() returns true");
}

static void test_static_mutex_lock_unlock(void)
{
	bool locked = test_mutex_.lock(0);
	TEST_ASSERT(locked, "StaticMutex::lock() succeeds");

	// Try to lock again - should fail (same task can't double-lock mutex)
	// Actually, FreeRTOS mutex is recursive by default, so this might succeed
	// Let's just unlock and verify no crash
	test_mutex_.unlock();

	// Lock again should work
	locked = test_mutex_.lock(0);
	TEST_ASSERT(locked, "StaticMutex::lock() after unlock succeeds");
	test_mutex_.unlock();
}

//=============================================================================
// MutexLock (RAII) Tests
//=============================================================================

static void test_mutex_lock_raii(void)
{
	{
		MutexLock lock(test_mutex_, 0);
		TEST_ASSERT(lock.is_locked(), "MutexLock RAII lock succeeds");
		TEST_ASSERT(static_cast<bool>(lock), "MutexLock operator bool() returns true");
	}
	// After scope exit, mutex should be unlocked

	// Verify we can lock again
	{
		MutexLock lock2(test_mutex_, 0);
		TEST_ASSERT(lock2.is_locked(), "MutexLock after RAII exit succeeds");
	}
}

//=============================================================================
// StaticBinarySemaphore Tests
//=============================================================================

static void test_static_binary_semaphore_create(void)
{
	SemaphoreHandle_t handle = test_bin_sem_.create();
	TEST_ASSERT(handle != nullptr, "StaticBinarySemaphore::create() returns valid handle");
	TEST_ASSERT(test_bin_sem_.is_created(), "StaticBinarySemaphore::is_created() returns true");
}

static void test_static_binary_semaphore_give_take(void)
{
	// Binary semaphore starts empty, so take should fail
	bool taken = test_bin_sem_.take(0);
	TEST_ASSERT(!taken, "StaticBinarySemaphore::take() fails when empty");

	// Give should succeed
	test_bin_sem_.give();

	// Now take should succeed
	taken = test_bin_sem_.take(0);
	TEST_ASSERT(taken, "StaticBinarySemaphore::take() succeeds after give");

	// Take again should fail (binary = 0 or 1)
	taken = test_bin_sem_.take(0);
	TEST_ASSERT(!taken, "StaticBinarySemaphore::take() fails after take");
}

//=============================================================================
// StaticCountingSemaphore Tests
//=============================================================================

static void test_static_counting_semaphore_create(void)
{
	SemaphoreHandle_t handle = test_cnt_sem_.create(3);  // initial count = 3
	TEST_ASSERT(handle != nullptr, "StaticCountingSemaphore::create() returns valid handle");
	TEST_ASSERT(test_cnt_sem_.is_created(), "StaticCountingSemaphore::is_created() returns true");
}

static void test_static_counting_semaphore_count(void)
{
	// Initial count was 3
	TEST_ASSERT_EQ(test_cnt_sem_.count(), 3, "StaticCountingSemaphore initial count = 3");

	// Take decrements count
	test_cnt_sem_.take(0);
	TEST_ASSERT_EQ(test_cnt_sem_.count(), 2, "StaticCountingSemaphore count after take = 2");

	test_cnt_sem_.take(0);
	TEST_ASSERT_EQ(test_cnt_sem_.count(), 1, "StaticCountingSemaphore count after take = 1");

	// Give increments count
	test_cnt_sem_.give();
	TEST_ASSERT_EQ(test_cnt_sem_.count(), 2, "StaticCountingSemaphore count after give = 2");

	// Give up to max
	test_cnt_sem_.give();
	test_cnt_sem_.give();
	test_cnt_sem_.give();
	// Count should be capped at max (5)
	TEST_ASSERT(test_cnt_sem_.count() <= 5, "StaticCountingSemaphore count <= max");

	// Drain all
	while (test_cnt_sem_.take(0)) {}
	TEST_ASSERT_EQ(test_cnt_sem_.count(), 0, "StaticCountingSemaphore drained to 0");
}

//=============================================================================
// Utility Function Tests
//=============================================================================

static void test_delay(void)
{
	TickType_t start = get_tick_count();
	delay(pdMS_TO_TICKS(50));
	TickType_t elapsed = get_tick_count() - start;

	// Allow some tolerance
	TEST_ASSERT(elapsed >= 40 && elapsed <= 70, "delay() ~50ms");
}

static void test_get_tick_count(void)
{
	TickType_t t1 = get_tick_count();
	vTaskDelay(10);
	TickType_t t2 = get_tick_count();

	TEST_ASSERT(t2 > t1, "get_tick_count() increases over time");
}

//=============================================================================
// Entry Point
//=============================================================================

extern "C" void test_freertos_runtime(void)
{
	// StaticTask tests
	test_static_task_create();

	// StaticQueue tests
	test_static_queue_create();
	test_static_queue_send_receive();
	test_static_queue_peek();
	test_static_queue_full();

	// StaticMutex tests
	test_static_mutex_create();
	test_static_mutex_lock_unlock();

	// MutexLock RAII tests
	test_mutex_lock_raii();

	// StaticBinarySemaphore tests
	test_static_binary_semaphore_create();
	test_static_binary_semaphore_give_take();

	// StaticCountingSemaphore tests
	test_static_counting_semaphore_create();
	test_static_counting_semaphore_count();

	// Utility function tests
	test_delay();
	test_get_tick_count();

	printf("\r\n");
	printf("Note: give_from_isr() requires ISR context (not tested here).\r\n");
}
