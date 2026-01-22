/**
 * Application Initialization
 */

#include "main.h"
#include "cmsis_os.h"
#include "stm32zero-debug.hpp"
#include <cstdio>

/*
    FreeRTOS 태스크 우선순위 정의

    CMSIS-RTOS 버전에 따라 다른 우선순위 값을 사용한다.
    v1과 v2의 호환성을 위해 통일된 인터페이스를 제공한다.

    우선순위 레벨:
        IDLE            유휴 태스크 (가장 낮음)
        LOW             낮은 우선순위 작업
        BELOWNORMAL     보통보다 낮음
        NORMAL          기본 우선순위 (대부분 태스크)
        ABOVENORMAL     보통보다 높음
        HIGH            높은 우선순위 (중요 작업)
        REALTIME        실시간 우선순위 (가장 높음)

    참고:
        - CMSIS v1: 0-6 범위
        - CMSIS v2: 1-48 범위 (8단위 증가)
        - 높은 숫자가 높은 우선순위
*/
#if (osCMSIS < 0x20000U)
#define TASK_IDLE_PRIORITY          0
#define TASK_LOW_PRIORITY           1
#define TASK_BELOWNORMAL_PRIORITY   2
#define TASK_NORMAL_PRIORITY        3
#define TASK_ABOVENORMAL_PRIORITY   4
#define TASK_HIGH_PRIORITY          5
#define TASK_REALTIME_PRIORITY      6
#else
#define TASK_IDLE_PRIORITY          1   // osPriorityIdle
#define TASK_LOW_PRIORITY           8   // osPriorityLow
#define TASK_BELOWNORMAL_PRIORITY   16  // osPriorityBelowNormal
#define TASK_NORMAL_PRIORITY        24  // osPriorityNormal
#define TASK_ABOVENORMAL_PRIORITY   32  // osPriorityAboveNormal
#define TASK_HIGH_PRIORITY          40  // osPriorityHigh
#define TASK_REALTIME_PRIORITY      48  // osPriorityRealtime
#endif

static __NO_RETURN void SYSTEM_task_func_(void* arg)
{
	uint32_t count = 0;
	(void)arg;

	while (true) {
		printf("%lu SYSTEM task running...\r\n", count++);
		vTaskDelay(1000); // Delay for 1 second
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
	stm32zero::debug::init();

	// RTOS initialization and start (does not return)
	osKernelInitialize();
	xTaskCreate(INIT_task_func_, "INIT", 128, NULL, TASK_NORMAL_PRIORITY, NULL);
	xTaskCreate(SYSTEM_task_func_, "SYSTEM", 256, NULL, TASK_NORMAL_PRIORITY, NULL);
	osKernelStart();

	// Should never reach here
	while (true) {
	}
}
