# STM32ZERO-DEMO-WeACT-H503

Demo project for WeACT H503 board featuring the STM32H503CB microcontroller.

## Overview

The STM32H503CB is a cost-effective Cortex-M33 MCU running at up to 250 MHz with:
- 128 KB Flash memory
- 32 KB RAM
- Hardware FPU with single-precision support
- TrustZone security (ARMv8-M)

## H5 Memory Architecture

Unlike the STM32H7 series with its complex multi-domain memory layout, the STM32H5 features a simple, traditional memory structure:

| Region | Address    | Size   | Description          |
|--------|------------|--------|----------------------|
| FLASH  | 0x08000000 | 128 KB | Program storage      |
| RAM    | 0x20000000 | 32 KB  | Single SRAM region   |

### Comparison: H5 vs H7

| Feature          | STM32H5 (H503)          | STM32H7 (H753)                   |
|------------------|-------------------------|----------------------------------|
| Core             | Cortex-M33 (ARMv8-M)    | Cortex-M7 (ARMv7-M)              |
| RAM Layout       | Single contiguous       | Multiple distributed regions     |
| TCM Memory       | Not available           | ITCMRAM (64KB) + DTCMRAM (128KB) |
| TrustZone        | Supported               | Not available                    |
| Cache            | None                    | L1 I-cache + D-cache             |
| Startup          | Standard                | Extended (DTCMRAM init)          |
| DMA Restrictions | None                    | Domain-specific                  |

## Linker Scripts (.ld)

This project provides two linker script variants. **No modifications** were made to the CubeMX-generated defaults.

### 1. STM32H503xx_FLASH.ld (Standard)

Standard Flash build:
- Code in FLASH (0x08000000)
- Data/BSS in RAM (0x20000000)
- TLS (Thread Local Storage) sections included

### 2. STM32H503xx_RAM.ld (Debug)

RAM-only execution for debugging:
- All sections loaded to RAM
- Faster iteration during development
- No flash programming required

### TLS Sections

The H5 linker scripts include Thread Local Storage support:

```
.tdata    - Initialized thread-local data
.tbss     - Zero-initialized thread-local data
```

These sections are part of the standard CubeMX output for Cortex-M33 and support C11/C++11 `_Thread_local` / `thread_local` keywords.

## Startup Assembly (.s)

The startup file (`startup_stm32h503xx.s`) uses the **standard STM32 startup sequence** with no modifications.

### Reset_Handler Sequence

```
1. Set stack pointer
2. Copy .data from Flash to RAM
3. Zero-fill .bss
4. SystemInit()
5. __libc_init_array (C++ constructors)
6. main()
```

### Key Differences from H7

| Aspect           | H5 Startup                  | H7 Startup                      |
|------------------|-----------------------------|---------------------------------|
| Power setup      | Not required                | ExitRun0Mode() before SystemInit|
| SystemInit call  | After .data/.bss init       | Before .data/.bss init          |
| DTCMRAM init     | Not applicable              | Separate copy/zero loops        |
| Extra sections   | None                        | DTCMRAM .data/.bss              |

## Usage Guide

### Choosing a Linker Script

| Use Case              | Linker Script          |
|-----------------------|------------------------|
| Normal development    | STM32H503xx_FLASH.ld   |
| Fast debug iteration  | STM32H503xx_RAM.ld     |

### DMA Buffer Placement

Unlike H7, the H5 has no DMA access restrictions. All RAM is accessible by DMA without special section placement:

```c
/* Simple declaration - no special attributes needed */
uint8_t dma_buffer[256];
```

### Thread Local Storage

If using TLS variables (C11/C++11):

```c
_Thread_local int per_thread_counter = 0;
```

## STM32ZERO Section Macros Compatibility

The STM32ZERO library provides section placement macros for H7-specific memory regions:

```c
STM32ZERO_ITCM        // .itcmram section
STM32ZERO_DTCM        // .dtcmram section (BSS)
STM32ZERO_DTCM_DATA   // .dtcmram_data section (initialized)
STM32ZERO_DMA         // .dma section
STM32ZERO_DMA_TX      // .dma_tx section
STM32ZERO_DMA_RX      // .dma_rx section
```

**On H5, these macros work without issues.** Since the H5 linker script does not define these special sections, variables using these macros automatically fall back to the default `.data` or `.bss` sections in RAM.

```c
/* This code works on both H5 and H7 */
STM32ZERO_DMA_RX uint8_t rx_buffer[256];  // H7: placed in .dma_rx (SRAM1)
                                          // H5: placed in .bss (RAM)

STM32ZERO_DTCM_DATA int fast_var = 100;   // H7: placed in .dtcmram_data (DTCMRAM)
                                          // H5: placed in .data (RAM)
```

This allows sharing the same source code between H5 and H7 projects without `#ifdef` guards for memory placement.

## Important Notes

1. **No Cache Management**: Unlike H7, the H5 has no L1 cache. No cache maintenance operations are needed for DMA.

2. **Simple Memory Model**: Single RAM region means no need to consider memory placement for performance optimization.

3. **TrustZone**: The Cortex-M33 supports TrustZone, but this demo project does not configure secure/non-secure partitioning.

4. **Standard Startup**: No custom modifications to linker scripts or startup files. CubeMX defaults are sufficient for most use cases.

5. **Cross-Platform Code**: STM32ZERO section macros can be used freely. On H5, they have no effect and variables go to standard sections.
