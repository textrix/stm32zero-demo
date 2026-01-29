/**
 * STM32ZERO FDCAN Runtime Test
 *
 * Test: FDCAN Echo (ID 0x100 -> 0x180)
 *   - Receives CAN-FD messages on ID 0x100
 *   - Echoes back on ID 0x180 (0x100 + 0x80)
 *   - Uses 64-byte DLC with BRS (500K nominal / 2M data)
 *   - Runs until any key is pressed
 *
 * Requirements:
 *   - FDCAN1 configured in STM32CubeMX (80MHz clock)
 *   - USE_HAL_FDCAN_REGISTER_CALLBACKS=1
 */

#include "main.h"
#include "fdcan.h"
#include "stm32zero.hpp"
#include "stm32zero-sio.hpp"
#include "stm32zero-fdcan.hpp"
#include "stm32zero-freertos.hpp"

using namespace stm32zero;
using namespace stm32zero::fdcan;
using namespace stm32zero::freertos;

//=============================================================================
// Test Configuration
//=============================================================================

#define FDCAN_RX_ID		0x100
#define FDCAN_TX_ID_OFFSET	0x80
#define FDCAN_QUEUE_LEN		16
#define FDCAN_TIMEOUT_MS	100

//=============================================================================
// FDCAN Instance
//=============================================================================

STM32ZERO_DEFINE_FDCAN(can1, hfdcan1, FDCAN_QUEUE_LEN);

//=============================================================================
// Test Reporting (from test_runner)
//=============================================================================

extern void test_report_pass(const char* desc);
extern void test_report_fail(const char* desc);

//=============================================================================
// Statistics
//=============================================================================

static volatile uint32_t fdcan_error_count = 0;
static volatile uint32_t fdcan_last_error = 0;
static volatile uint32_t rx_count = 0;
static volatile uint32_t tx_count = 0;

static void fdcan_error_callback(Fdcan* self, uint32_t error_code)
{
	(void)self;
	fdcan_error_count++;
	fdcan_last_error = error_code;
}

//=============================================================================
// Runtime Tests
//=============================================================================

extern "C" void test_fdcan_runtime(void)
{
	static char fmt_buf[128];

	sio::writef(fmt_buf, "[FDCAN] Initializing FDCAN1...\r\n");

	// Initialize FDCAN
	STM32ZERO_INIT_FDCAN(can1, hfdcan1);
	can1.set_error_callback(fdcan_error_callback);

	// Test 1: DLC conversion
	{
		bool pass = true;
		pass &= (dlc_to_bytes(Dlc::DLC_0) == 0);
		pass &= (dlc_to_bytes(Dlc::DLC_8) == 8);
		pass &= (dlc_to_bytes(Dlc::DLC_12) == 12);
		pass &= (dlc_to_bytes(Dlc::DLC_64) == 64);

		if (pass) {
			test_report_pass("FDCAN dlc_to_bytes");
		} else {
			test_report_fail("FDCAN dlc_to_bytes");
		}
	}

	// Test 2: Bytes to DLC (round up)
	{
		bool pass = true;
		pass &= (bytes_to_dlc(0) == Dlc::DLC_0);
		pass &= (bytes_to_dlc(8) == Dlc::DLC_8);
		pass &= (bytes_to_dlc(9) == Dlc::DLC_12);   // rounds up
		pass &= (bytes_to_dlc(13) == Dlc::DLC_16);  // rounds up
		pass &= (bytes_to_dlc(64) == Dlc::DLC_64);

		if (pass) {
			test_report_pass("FDCAN bytes_to_dlc");
		} else {
			test_report_fail("FDCAN bytes_to_dlc");
		}
	}

	// Test 3: Open channel (new API)
	{
		can1.set_format(FrameFormat::FD_BRS);
		can1.set_nominal(Bitrate::K500);
		can1.set_data(Bitrate::M2);
		can1.set_filter_id(FDCAN_RX_ID);

		Status status = can1.open();

		if (status == Status::OK && can1.is_open()) {
			test_report_pass("FDCAN open (FD_BRS 500K/2M)");
		} else {
			test_report_fail("FDCAN open (FD_BRS 500K/2M)");
			return;  // Cannot continue without open channel
		}

		// Test bus state API
		BusState state = can1.bus_state();
		if (state == BusState::ACTIVE) {
			test_report_pass("FDCAN bus_state (ACTIVE)");
		} else {
			test_report_fail("FDCAN bus_state (ACTIVE)");
		}
	}

	// Test 4: Echo loop (runs until key pressed)
	sio::writef(fmt_buf, "\r\n");
	sio::writef(fmt_buf, "[FDCAN] Echo started (RX:0x%03lX -> TX:0x%03lX)\r\n",
		    (uint32_t)FDCAN_RX_ID, (uint32_t)(FDCAN_RX_ID + FDCAN_TX_ID_OFFSET));
	sio::writef(fmt_buf, "[FDCAN] Press any key to stop...\r\n");
	sio::writef(fmt_buf, "\r\n");

	RxMessage rx_msg;
	uint32_t last_report_tick = xTaskGetTickCount();

	while (true) {
		// Check for key press (non-blocking)
		if (sio::readable()) {
			// Consume the key
			char c;
			sio::read(&c, 1);
			break;
		}

		// Try to receive CAN message
		Status status = can1.read(&rx_msg, FDCAN_TIMEOUT_MS);

		if (status == Status::OK) {
			rx_count++;

			// Echo back with offset ID
			uint16_t echo_id = static_cast<uint16_t>(rx_msg.id + FDCAN_TX_ID_OFFSET);
			Status tx_status = can1.write(echo_id, rx_msg.data, rx_msg.length, FDCAN_TIMEOUT_MS);

			if (tx_status == Status::OK) {
				tx_count++;
			}
		}

		// Periodic status report (every 1 second)
		uint32_t now = xTaskGetTickCount();
		if ((now - last_report_tick) >= pdMS_TO_TICKS(1000)) {
			last_report_tick = now;
			if (rx_count > 0 || fdcan_error_count > 0) {
				sio::writef(fmt_buf, "[FDCAN] RX:%lu TX:%lu ERR:%lu\r\n",
					    rx_count, tx_count, fdcan_error_count);
			}
		}
	}

	// Print final stats
	sio::writef(fmt_buf, "\r\n");
	sio::writef(fmt_buf, "[FDCAN] Echo stopped\r\n");
	sio::writef(fmt_buf, "[FDCAN] Total: RX:%lu TX:%lu ERR:%lu\r\n",
		    rx_count, tx_count, fdcan_error_count);

	if (fdcan_last_error != 0) {
		sio::writef(fmt_buf, "[FDCAN] Last error: 0x%08lX\r\n", fdcan_last_error);
	}

	// Close FDCAN
	can1.close();

	test_report_pass("FDCAN echo loop completed");
}
