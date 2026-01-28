/**
 * STM32ZERO Configuration for NUCLEO-H753ZI Demo
 */

#ifndef __STM32ZERO_CONF_H__
#define __STM32ZERO_CONF_H__

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

// Microsecond timer (48-bit, 16+16+16 cascaded timers)
#define STM32ZERO_USTIM_LOW   3
#define STM32ZERO_USTIM_MID   4
#define STM32ZERO_USTIM_HIGH  12

// Namespace alias
#define STM32ZERO_NAMESPACE_ALIAS zero

#endif // __STM32ZERO_CONF_H__
