# STM32ZERO-DEMO-NUCLEO-H753ZI

Demo project for NUCLEO-H753ZI board featuring the STM32H753ZI microcontroller.

## Overview

The STM32H753ZI is a high-performance Cortex-M7 MCU running at up to 480 MHz with:
- 2 MB Flash memory (dual-bank)
- 1 MB total RAM across multiple domains
- Hardware FPU with double-precision support
- L1 cache (16KB I-cache + 16KB D-cache)

## H7 Memory Architecture

Unlike typical STM32 devices with a single contiguous RAM region, the STM32H7 series features a distributed memory architecture with multiple RAM domains:

| Region   | Address      | Size   | Description                        |
|----------|--------------|--------|------------------------------------|
| ITCMRAM  | 0x00000000   | 64 KB  | Instruction TCM (zero wait-state)  |
| DTCMRAM  | 0x20000000   | 128 KB | Data TCM (zero wait-state)         |
| AXI SRAM | 0x24000000   | 512 KB | Main RAM (D1 domain)               |
| SRAM1    | 0x30000000   | 128 KB | D2 domain RAM                      |
| SRAM2    | 0x30020000   | 128 KB | D2 domain RAM                      |
| SRAM3    | 0x30040000   | 32 KB  | D2 domain RAM                      |
| SRAM4    | 0x38000000   | 64 KB  | D3 domain (low-power capable)      |

### Comparison with Standard STM32

| Feature          | Standard STM32      | STM32H7                          |
|------------------|---------------------|----------------------------------|
| RAM Layout       | Single contiguous   | Multiple distributed regions     |
| TCM Memory       | Not available       | ITCMRAM (64KB) + DTCMRAM (128KB) |
| DMA Access       | All RAM accessible  | Domain restrictions apply        |
| Cache            | Usually none        | L1 I-cache + D-cache             |
| Startup          | Single .data/.bss   | Multiple sections per region     |

## Clock Configuration (from .ioc)

```
CPU Clock:     480 MHz (Cortex-M7)
AXI Clock:     240 MHz
AHB Clock:     240 MHz (HCLK)
APB1/2/3/4:    120 MHz
HSE:           8 MHz (external oscillator)
PLL Source:    HSE
```

## Cache & MPU Configuration (from .ioc)

### L1 Cache

| Cache    | Status  | Size  |
|----------|---------|-------|
| I-Cache  | Enabled | 16 KB |
| D-Cache  | Enabled | 16 KB |

### MPU Regions

The MPU is configured to make D2 domain SRAMs DMA-safe (non-cacheable or write-through):

| Region | Address    | Size   | Target | Attributes                          |
|--------|------------|--------|--------|-------------------------------------|
| 1      | 0x30020000 | 128 KB | SRAM2  | TEX=1, Full Access, No Exec         |
| 2      | 0x30040000 | 512 B  | SRAM3  | Shareable, Not Cacheable, Bufferable|
| 3      | 0x30000000 | 128 KB | SRAM1  | Shareable, Bufferable, No Exec      |

**Why MPU configuration matters:**
- AXI SRAM (0x24000000) is cached by default - requires cache maintenance for DMA
- D2 SRAMs (SRAM1/2/3) are configured as non-cacheable or write-through via MPU
- DMA buffers in SRAM1/2/3 work without explicit cache operations

### Memory Map with Peripheral Allocation

```
┌─────────────────────────────────────────────────────────────────┐
│ FLASH (0x08000000)                                    2 MB      │
│   └── Code, constants, initialization data                      │
├─────────────────────────────────────────────────────────────────┤
│ ITCMRAM (0x00000000)                                 64 KB      │
│   └── (Available for time-critical code)                        │
├─────────────────────────────────────────────────────────────────┤
│ DTCMRAM (0x20000000)                                128 KB      │
│   └── .dtcmram_data, .dtcmram_bss (fastest data access)         │
├─────────────────────────────────────────────────────────────────┤
│ AXI SRAM (0x24000000) - D1 Domain                   512 KB      │
│   └── .data, .bss, heap, stack (main RAM)                       │
├─────────────────────────────────────────────────────────────────┤
│ SRAM1 (0x30000000) - D2 Domain [MPU Region 3]       128 KB      │
│   └── .dma_sec (.dma, .dma_tx, .dma_rx) - DMA buffers           │
├─────────────────────────────────────────────────────────────────┤
│ SRAM2 (0x30020000) - D2 Domain [MPU Region 1]       128 KB      │
│   └── .lwip_heap_sec - LWIP RAM heap (0x30020000)               │
├─────────────────────────────────────────────────────────────────┤
│ SRAM3 (0x30040000) - D2 Domain [MPU Region 2]        32 KB      │
│   └── .lwip_sec - Ethernet RX/TX descriptors                    │
│       ├── RxDecripSection (0x30040000)                          │
│       ├── TxDecripSection (0x30040100)                          │
│       └── Rx_PoolSection  (0x30040200)                          │
├─────────────────────────────────────────────────────────────────┤
│ SRAM4 (0x38000000) - D3 Domain                       64 KB      │
│   └── (Available, retained in low-power modes)                  │
└─────────────────────────────────────────────────────────────────┘
```

## Linker Scripts (.ld)

This project provides three linker script variants:

### 1. STM32H753ZITX_FLASH.ld (Standard)

Basic Flash build using CubeMX defaults:
- Code in FLASH
- Data/BSS in RAM_D1 (AXI SRAM)
- No special section handling

### 2. STM32H753ZITX_RAM.ld (Debug)

RAM execution for debugging:
- Code and data loaded to RAM_EXEC (AXI SRAM)
- Faster iteration during development
- No flash programming required

### 3. STM32H753XX_FLASH.ld (Extended)

Production-ready with optimized memory placement:

**DTCMRAM Sections:**
```
.dtcmram_data    - Initialized data in DTCMRAM (copied from Flash)
.dtcmram_bss     - Zero-initialized data in DTCMRAM
```

Symbols for startup initialization:
- `_sidtcmram` - Source address in Flash for DTCMRAM .data
- `_sdtcmram`, `_edtcmram` - DTCMRAM .data bounds
- `_sdtcmram_bss`, `_edtcmram_bss` - DTCMRAM .bss bounds

**DMA-aligned Section:**
```
.dma_sec (NOLOAD) :
{
    . = ALIGN(32);   /* 32-byte alignment for cache coherency */
    *(.sram1)
    *(.dma)
    *(.dma_tx)
    *(.dma_rx)
} >SRAM1
```

**LWIP Ethernet Sections:**
```
.lwip_heap_sec   - LWIP heap in SRAM2
.lwip_sec        - Ethernet descriptors in SRAM3
```

## Startup Assembly (.s)

The modified startup file (`Core/Startup/startup_stm32h753zitx.s`) extends the standard STM32 startup with H7-specific initialization.

### Reset_Handler Sequence

```
1. Set stack pointer
2. ExitRun0Mode()        <- H7-specific power configuration
3. SystemInit()
4. Copy .data from Flash to RAM
5. Zero-fill .bss
6. Copy DTCMRAM .data    <- H7 extension
7. Zero-fill DTCMRAM .bss <- H7 extension
8. __libc_init_array (C++ constructors)
9. main()
```

### Key Difference from Standard STM32

Standard startup only initializes a single .data and .bss section. The H7 startup adds separate initialization loops for DTCMRAM:

```asm
/* Copy DTCMRAM initialized data from flash */
  ldr r0, =_sdtcmram
  ldr r1, =_edtcmram
  ldr r2, =_sidtcmram
  ...

/* Zero fill DTCMRAM bss */
  ldr r2, =_sdtcmram_bss
  ldr r4, =_edtcmram_bss
  ...
```

## Usage Guide

### Placing Variables in DTCMRAM

Use GCC section attributes to place performance-critical data in DTCMRAM:

```c
/* Initialized data in DTCMRAM */
__attribute__((section(".dtcmram_data")))
int fast_counter = 100;

/* Zero-initialized (BSS) in DTCMRAM */
__attribute__((section(".dtcmram_bss")))
uint8_t fast_buffer[1024];
```

### DMA Buffer Alignment

For DMA buffers, use the `.dma` section with proper alignment:

```c
__attribute__((section(".dma"), aligned(32)))
uint8_t dma_rx_buffer[256];

__attribute__((section(".dma_tx"), aligned(32)))
uint8_t dma_tx_buffer[256];
```

### Choosing a Linker Script

| Use Case                          | Linker Script              |
|-----------------------------------|----------------------------|
| Basic development                 | STM32H753ZITX_FLASH.ld     |
| Fast debug iteration              | STM32H753ZITX_RAM.ld       |
| Performance-critical, DMA, LWIP   | STM32H753XX_FLASH.ld       |

To change the linker script, update `CMakeLists.txt` or the IDE project settings.

## Important Notes

1. **Cache Coherency**: When using DMA with cached memory (AXI SRAM), ensure proper cache maintenance (invalidate before read, clean before write).

2. **DTCMRAM Access**: DTCMRAM provides zero wait-state access but is NOT accessible by DMA. Place DMA buffers in SRAM1-4 instead.

3. **Power Domains**: SRAM4 (D3 domain) can remain powered in low-power modes. Use it for data that must persist across sleep states.

4. **ExitRun0Mode**: The H7 requires explicit power supply configuration at startup. This is handled by `ExitRun0Mode()` called before `SystemInit()`.
