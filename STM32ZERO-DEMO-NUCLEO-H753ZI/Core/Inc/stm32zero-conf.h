/**
 * STM32ZERO Configuration for NUCLEO-H753ZI Demo
 */

#ifndef STM32ZERO_CONF_H
#define STM32ZERO_CONF_H

// STM32H7 D-Cache line size
#define STM32ZERO_CACHE_LINE_SIZE  32

// Serial I/O UART (NUCLEO-H753ZI uses USART3 via ST-Link VCP)
#define STM32ZERO_SIO_UART  huart3

// RX ring buffer size
#define STM32ZERO_SIO_RX_SIZE  256

// TX dual buffer size (4KB x 2 = 8KB total)
#define STM32ZERO_SIO_TX_SIZE  4096

// RX DMA buffer size
#define STM32ZERO_SIO_DMA_SIZE  64

// FreeRTOS enabled
#define STM32ZERO_RTOS_FREERTOS    1

#endif // STM32ZERO_CONF_H
