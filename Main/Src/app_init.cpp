/**
 * Application Initialization
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero.hpp"
#include "stm32zero-ustim.hpp"
#include "stm32zero-debug.hpp"
#include "stm32zero-freertos.hpp"
#include <cstdio>

using namespace stm32zero;
using namespace stm32zero::freertos;

// Static task in DTCM (zero heap allocation, fast access)
STM32ZERO_DTCM static StaticTask<256> system_task_;  // 256 words = 1024 bytes

static __NO_RETURN void SYSTEM_task_func_(void* arg)
{
	uint32_t count = 0;
	(void)arg;

	while (true) {
		uint64_t t = ustim::get();
		printf("%lu.%06lu %lu SYSTEM task running...\r\n", (uint32_t)(t / 1000000), (uint32_t)(t % 1000000), count++);
		vTaskDelay(10); // Delay for 1 second
	}
}

static __NO_RETURN void INIT_task_func_(void* arg)
{
	// Debug output initialization (after MX_USART3_UART_Init)

#if 0
	LED_init();
	uart_init();
	uart_put(0, (uint8_t*)"Hello\r\n", 7);
	//DEBUG_init();
	B1BTN_init();
	//ADC_init();
	//RGB_init();

	/* init code for USB_DEVICE */
	MX_USB_DEVICE_Init();

	/* init code for LWIP */
	MX_LWIP_Init();

	UDP_init();

	SLAVE_init();
	//MASTER_init();
	//UART_CMD_init();

	/*while (true) {
		static uint32_t count_ = 0;
		vTaskDelay(1000); // Delay for 1 second
		printf("%lu System task running...\r\n", count_++);
		//printAllTaskStackUsage();
	}*/
#endif

	vTaskDelete(NULL); // Delete this task if not needed

	// Eat warning: 'noreturn' function does return
	while (true) {
	}
}

extern "C" __NO_RETURN void app_init(void)
{
	stm32zero::ustim::init();
	stm32zero::debug::init();

	// RTOS initialization and start (does not return)
	osKernelInitialize();

	// Dynamic task creation (uses heap)
	xTaskCreate(INIT_task_func_, "INIT", 128, NULL, +Priority::NORMAL, NULL);

	// Static task creation (zero heap allocation)
	system_task_.create(SYSTEM_task_func_, "SYSTEM", Priority::NORMAL);

	osKernelStart();

	// Should never reach here
	while (true) {
	}
}
