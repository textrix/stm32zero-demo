/**
 * STM32ZERO Configuration for NUCLEO-H753ZI Demo
 */

#ifndef STM32ZERO_CONF_H
#define STM32ZERO_CONF_H

// STM32H7 D-Cache line size
#define STM32ZERO_CACHE_LINE_SIZE  32

// Stdout UART (NUCLEO-H753ZI uses USART3 via ST-Link VCP)
#define STM32ZERO_STDOUT_UART  huart3

// Stdout buffer size (4KB x 2 = 8KB total)
#define STM32ZERO_STDOUT_BUFFER_SIZE  4096

// Stdin UART (same as stdout for VCP)
#define STM32ZERO_STDIN_UART  huart3

// Stdin ring buffer size
#define STM32ZERO_STDIN_BUFFER_SIZE  256

// Stdin DMA buffer size
#define STM32ZERO_STDIN_DMA_SIZE  64

// FreeRTOS enabled
#define STM32ZERO_RTOS_FREERTOS    1

#endif // STM32ZERO_CONF_H
