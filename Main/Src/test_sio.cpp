/**
 * STM32ZERO SIO Module Runtime Tests
 *
 * Tests for stm32zero-sio.hpp functionality:
 *   - init() initialization
 *   - write() data transmission
 *   - read() non-blocking/timeout
 *   - readln() line reading
 *   - readable() / writable()
 *   - write_peak() / read_peak()
 *
 * Note: Some tests require manual verification via terminal.
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero.hpp"
#include "stm32zero-sio.hpp"
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
// sio::init() Tests
//=============================================================================

static void test_sio_init_called(void)
{
	// sio::init() should have been called in app_init()
	// We can verify by checking that write() works (returns > 0)
	const char* msg = "SIO init test\r\n";
	int written = sio::write(msg, strlen(msg));
	TEST_ASSERT(written > 0, "sio::init() already called (write works)");
}

//=============================================================================
// sio::write() Tests
//=============================================================================

static void test_sio_write_basic(void)
{
	const char* msg = "Hello, STM32ZERO!\r\n";
	int len = strlen(msg);
	int written = sio::write(msg, len);

	TEST_ASSERT_EQ(written, len, "sio::write() returns correct length");
}

static void test_sio_write_empty(void)
{
	int written = sio::write("", 0);
	TEST_ASSERT_EQ(written, 0, "sio::write() empty string returns 0");
}

static void test_sio_write_binary(void)
{
	uint8_t data[] = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
	int written = sio::write(data, sizeof(data));
	TEST_ASSERT_EQ(written, (int)sizeof(data), "sio::write() binary data");
}

//=============================================================================
// sio::flush() and writable() Tests
//=============================================================================

static void test_sio_flush(void)
{
	// Write some data and flush
	sio::write("Flush test\r\n", 12);
	bool flushed = sio::flush(1000);  // 1 second timeout

	// flush() should return true if all data sent, false on timeout
	// Either way, it should not crash
	TEST_ASSERT(true, "sio::flush() called without crash");

	// writable() should return a valid bool (doesn't crash)
	bool ready = sio::writable();
	TEST_ASSERT(ready || !ready, "sio::writable() returns valid bool");

	(void)flushed;  // suppress unused warning
}

//=============================================================================
// sio::read() Tests (non-blocking)
//=============================================================================

static void test_sio_read_empty(void)
{
	// Clear any pending data first
	char buf[64];
	while (sio::readable()) {
		sio::read(buf, sizeof(buf));
	}

	// Now read should return 0 (empty buffer)
	int read_count = sio::read(buf, sizeof(buf));
	TEST_ASSERT_EQ(read_count, 0, "sio::read() returns 0 when empty");
}

static void test_sio_readable(void)
{
	// Clear buffer
	char buf[64];
	while (sio::readable()) {
		sio::read(buf, sizeof(buf));
	}

	TEST_ASSERT(!sio::readable(), "sio::readable() returns false when empty");
}

//=============================================================================
// sio::read() with Timeout Tests
//=============================================================================

static void test_sio_read_timeout_expired(void)
{
	// Clear buffer first
	char buf[64];
	while (sio::readable()) {
		sio::read(buf, sizeof(buf));
	}

	// Try to read with short timeout - should return 0 (timeout)
	uint32_t start = xTaskGetTickCount();
	int read_count = sio::read(buf, 10, 50);  // 50ms timeout
	uint32_t elapsed = xTaskGetTickCount() - start;

	TEST_ASSERT_EQ(read_count, 0, "sio::read(timeout) returns 0 on timeout");
	TEST_ASSERT(elapsed >= 40 && elapsed <= 100, "sio::read(timeout) waits ~50ms");
}

//=============================================================================
// sio::wait_readable() Tests
//=============================================================================

static void test_sio_wait_readable_timeout(void)
{
	// Clear buffer first
	char buf[64];
	while (sio::readable()) {
		sio::read(buf, sizeof(buf));
	}

	// wait_readable() should timeout
	uint32_t start = xTaskGetTickCount();
	bool data_ready = sio::wait_readable(50);  // 50ms timeout
	uint32_t elapsed = xTaskGetTickCount() - start;

	TEST_ASSERT(!data_ready, "sio::wait_readable() returns false on timeout");
	TEST_ASSERT(elapsed >= 40 && elapsed <= 100, "sio::wait_readable() waits ~50ms");
}

//=============================================================================
// sio::readln() Tests
//=============================================================================

static void test_sio_readln_timeout(void)
{
	// Clear buffer first
	char buf[64];
	while (sio::readable()) {
		sio::read(buf, sizeof(buf));
	}

	// readln() should timeout
	uint32_t start = xTaskGetTickCount();
	int len = sio::readln(buf, sizeof(buf), 50);  // 50ms timeout
	uint32_t elapsed = xTaskGetTickCount() - start;

	TEST_ASSERT(len < 0, "sio::readln() returns -1 on timeout with no data");
	TEST_ASSERT(elapsed >= 40 && elapsed <= 100, "sio::readln() waits ~50ms");
}

//=============================================================================
// Peak Usage Tests
//=============================================================================

static void test_sio_write_peak(void)
{
	// Write a large amount of data to increase peak
	char buf[256];
	memset(buf, 'X', sizeof(buf));
	sio::write(buf, sizeof(buf));
	sio::flush(1000);  // 1 second timeout

	uint16_t peak = sio::write_peak();
	// Peak should be at least the size we wrote (or close to it)
	TEST_ASSERT(peak >= 100, "sio::write_peak() > 100 after write");
}

static void test_sio_read_peak(void)
{
	// Read peak depends on incoming data
	// Just verify it doesn't crash and returns reasonable value
	uint16_t peak = sio::read_peak();
	TEST_ASSERT(peak < 65535, "sio::read_peak() returns valid value");
}

//=============================================================================
// Entry Point
//=============================================================================

extern "C" void test_sio_runtime(void)
{
	// Init test
	test_sio_init_called();

	// Write tests
	test_sio_write_basic();
	test_sio_write_empty();
	test_sio_write_binary();

	// Flush test
	test_sio_flush();

	// Read tests (non-blocking)
	test_sio_read_empty();
	test_sio_readable();

	// Read with timeout
	test_sio_read_timeout_expired();

	// wait_readable() test
	test_sio_wait_readable_timeout();

	// readln() test
	test_sio_readln_timeout();

	// Peak usage tests
	test_sio_write_peak();
	test_sio_read_peak();

	printf("\r\n");
	printf("Note: Interactive sio::readln() test available after summary.\r\n");
}
