# STM32ZERO-DEMO-NUCLEO-H753ZI

NUCLEO-H753ZI 보드용 데모 프로젝트 (STM32H753ZI 마이크로컨트롤러).

## 개요

STM32H753ZI는 최대 480 MHz로 동작하는 고성능 Cortex-M7 MCU입니다:
- 2 MB Flash 메모리 (듀얼 뱅크)
- 총 1 MB RAM (다중 도메인)
- 배정밀도 지원 하드웨어 FPU
- L1 캐시 (16KB 명령어 캐시 + 16KB 데이터 캐시)

## H7 메모리 아키텍처

일반적인 STM32 디바이스가 단일 연속 RAM 영역을 가지는 것과 달리, STM32H7 시리즈는 다중 RAM 도메인으로 구성된 분산 메모리 아키텍처를 특징으로 합니다:

| 영역     | 주소         | 크기   | 설명                              |
|----------|--------------|--------|-----------------------------------|
| ITCMRAM  | 0x00000000   | 64 KB  | 명령어 TCM (대기 없음)            |
| DTCMRAM  | 0x20000000   | 128 KB | 데이터 TCM (대기 없음)            |
| AXI SRAM | 0x24000000   | 512 KB | 메인 RAM (D1 도메인)              |
| SRAM1    | 0x30000000   | 128 KB | D2 도메인 RAM                     |
| SRAM2    | 0x30020000   | 128 KB | D2 도메인 RAM                     |
| SRAM3    | 0x30040000   | 32 KB  | D2 도메인 RAM                     |
| SRAM4    | 0x38000000   | 64 KB  | D3 도메인 (저전력 유지 가능)      |

### 일반 STM32와의 비교

| 특징             | 일반 STM32          | STM32H7                          |
|------------------|---------------------|----------------------------------|
| RAM 레이아웃     | 단일 연속 영역      | 다중 분산 영역                   |
| TCM 메모리       | 없음                | ITCMRAM (64KB) + DTCMRAM (128KB) |
| DMA 접근         | 모든 RAM 접근 가능  | 도메인 제약 있음                 |
| 캐시             | 보통 없음           | L1 명령어 캐시 + 데이터 캐시     |
| 스타트업         | 단일 .data/.bss     | 영역별 다중 섹션                 |

## 클럭 설정 (.ioc 기준)

```
CPU 클럭:      480 MHz (Cortex-M7)
AXI 클럭:      240 MHz
AHB 클럭:      240 MHz (HCLK)
APB1/2/3/4:    120 MHz
HSE:           8 MHz (외부 오실레이터)
PLL 소스:      HSE
```

## 캐시 & MPU 설정 (.ioc 기준)

### L1 캐시

| 캐시     | 상태    | 크기  |
|----------|---------|-------|
| I-Cache  | 활성화  | 16 KB |
| D-Cache  | 활성화  | 16 KB |

### MPU 영역

MPU는 D2 도메인 SRAM을 DMA 안전하게 (비캐시 또는 라이트스루) 설정합니다:

| 영역 | 주소       | 크기   | 대상   | 속성                                |
|------|------------|--------|--------|-------------------------------------|
| 1    | 0x30020000 | 128 KB | SRAM2  | TEX=1, 전체 접근, 실행 불가         |
| 2    | 0x30040000 | 512 B  | SRAM3  | 공유가능, 비캐시, 버퍼링            |
| 3    | 0x30000000 | 128 KB | SRAM1  | 공유가능, 버퍼링, 실행 불가         |

**MPU 설정이 중요한 이유:**
- AXI SRAM (0x24000000)은 기본적으로 캐시됨 - DMA 사용 시 캐시 관리 필요
- D2 SRAM (SRAM1/2/3)은 MPU를 통해 비캐시 또는 라이트스루로 설정됨
- SRAM1/2/3의 DMA 버퍼는 명시적 캐시 작업 없이 동작

### 메모리 맵과 페리페럴 할당

```
┌─────────────────────────────────────────────────────────────────┐
│ FLASH (0x08000000)                                    2 MB      │
│   └── 코드, 상수, 초기화 데이터                                 │
├─────────────────────────────────────────────────────────────────┤
│ ITCMRAM (0x00000000)                                 64 KB      │
│   └── (시간 중요 코드용으로 사용 가능)                          │
├─────────────────────────────────────────────────────────────────┤
│ DTCMRAM (0x20000000)                                128 KB      │
│   └── .dtcmram_data, .dtcmram_bss (가장 빠른 데이터 접근)       │
├─────────────────────────────────────────────────────────────────┤
│ AXI SRAM (0x24000000) - D1 도메인                   512 KB      │
│   └── .data, .bss, 힙, 스택 (메인 RAM)                          │
├─────────────────────────────────────────────────────────────────┤
│ SRAM1 (0x30000000) - D2 도메인 [MPU 영역 3]         128 KB      │
│   └── .dma_sec (.dma, .dma_tx, .dma_rx) - DMA 버퍼              │
├─────────────────────────────────────────────────────────────────┤
│ SRAM2 (0x30020000) - D2 도메인 [MPU 영역 1]         128 KB      │
│   └── .lwip_heap_sec - LWIP RAM 힙 (0x30020000)                 │
├─────────────────────────────────────────────────────────────────┤
│ SRAM3 (0x30040000) - D2 도메인 [MPU 영역 2]          32 KB      │
│   └── .lwip_sec - 이더넷 RX/TX 디스크립터                       │
│       ├── RxDecripSection (0x30040000)                          │
│       ├── TxDecripSection (0x30040100)                          │
│       └── Rx_PoolSection  (0x30040200)                          │
├─────────────────────────────────────────────────────────────────┤
│ SRAM4 (0x38000000) - D3 도메인                       64 KB      │
│   └── (사용 가능, 저전력 모드에서 유지됨)                       │
└─────────────────────────────────────────────────────────────────┘
```

## 링커 스크립트 (.ld)

이 프로젝트는 세 가지 링커 스크립트 변형을 제공합니다:

### 1. STM32H753ZITX_FLASH.ld (표준)

CubeMX 기본값을 사용하는 기본 Flash 빌드:
- FLASH에 코드 배치
- RAM_D1 (AXI SRAM)에 Data/BSS 배치
- 특수 섹션 처리 없음

### 2. STM32H753ZITX_RAM.ld (디버그)

디버깅용 RAM 실행:
- RAM_EXEC (AXI SRAM)에 코드와 데이터 로드
- 개발 중 빠른 반복 작업 가능
- Flash 프로그래밍 불필요

### 3. STM32H753XX_FLASH.ld (확장)

최적화된 메모리 배치의 프로덕션용:

**DTCMRAM 섹션:**
```
.dtcmram_data    - DTCMRAM의 초기화된 데이터 (Flash에서 복사)
.dtcmram_bss     - DTCMRAM의 영초기화 데이터
```

스타트업 초기화를 위한 심볼:
- `_sidtcmram` - DTCMRAM .data의 Flash 내 소스 주소
- `_sdtcmram`, `_edtcmram` - DTCMRAM .data 경계
- `_sdtcmram_bss`, `_edtcmram_bss` - DTCMRAM .bss 경계

**DMA 정렬 섹션:**
```
.dma_sec (NOLOAD) :
{
    . = ALIGN(32);   /* 캐시 일관성을 위한 32바이트 정렬 */
    *(.sram1)
    *(.dma)
    *(.dma_tx)
    *(.dma_rx)
} >SRAM1
```

**LWIP 이더넷 섹션:**
```
.lwip_heap_sec   - SRAM2의 LWIP 힙
.lwip_sec        - SRAM3의 이더넷 디스크립터
```

## 스타트업 어셈블리 (.s)

수정된 스타트업 파일 (`Core/Startup/startup_stm32h753zitx.s`)은 표준 STM32 스타트업을 H7 특화 초기화로 확장합니다.

### Reset_Handler 시퀀스

```
1. 스택 포인터 설정
2. ExitRun0Mode()        <- H7 전용 전원 설정
3. SystemInit()
4. Flash에서 RAM으로 .data 복사
5. .bss 영초기화
6. DTCMRAM .data 복사    <- H7 확장
7. DTCMRAM .bss 영초기화 <- H7 확장
8. __libc_init_array (C++ 생성자)
9. main()
```

### 일반 STM32와의 핵심 차이점

표준 스타트업은 단일 .data와 .bss 섹션만 초기화합니다. H7 스타트업은 DTCMRAM을 위한 별도의 초기화 루프를 추가합니다:

```asm
/* Flash에서 DTCMRAM 초기화 데이터 복사 */
  ldr r0, =_sdtcmram
  ldr r1, =_edtcmram
  ldr r2, =_sidtcmram
  ...

/* DTCMRAM bss 영초기화 */
  ldr r2, =_sdtcmram_bss
  ldr r4, =_edtcmram_bss
  ...
```

## 사용 가이드

### DTCMRAM에 변수 배치하기

GCC 섹션 속성을 사용하여 성능이 중요한 데이터를 DTCMRAM에 배치합니다:

```c
/* DTCMRAM의 초기화된 데이터 */
__attribute__((section(".dtcmram_data")))
int fast_counter = 100;

/* DTCMRAM의 영초기화(BSS) 데이터 */
__attribute__((section(".dtcmram_bss")))
uint8_t fast_buffer[1024];
```

### DMA 버퍼 정렬

DMA 버퍼는 `.dma` 섹션과 적절한 정렬을 사용합니다:

```c
__attribute__((section(".dma"), aligned(32)))
uint8_t dma_rx_buffer[256];

__attribute__((section(".dma_tx"), aligned(32)))
uint8_t dma_tx_buffer[256];
```

### 링커 스크립트 선택

| 사용 사례                         | 링커 스크립트              |
|-----------------------------------|----------------------------|
| 기본 개발                         | STM32H753ZITX_FLASH.ld     |
| 빠른 디버그 반복                  | STM32H753ZITX_RAM.ld       |
| 성능 중요, DMA, LWIP 사용         | STM32H753XX_FLASH.ld       |

링커 스크립트를 변경하려면 `CMakeLists.txt` 또는 IDE 프로젝트 설정을 업데이트하세요.

## 중요 사항

1. **캐시 일관성**: 캐시된 메모리(AXI SRAM)와 DMA를 사용할 때 적절한 캐시 관리가 필요합니다 (읽기 전 무효화, 쓰기 전 클린).

2. **DTCMRAM 접근**: DTCMRAM은 대기 시간 없는 접근을 제공하지만 DMA가 접근할 수 없습니다. DMA 버퍼는 SRAM1-4에 배치하세요.

3. **전원 도메인**: SRAM4 (D3 도메인)는 저전력 모드에서도 전원이 유지될 수 있습니다. 슬립 상태에서도 유지해야 하는 데이터에 사용하세요.

4. **ExitRun0Mode**: H7은 스타트업 시 명시적인 전원 공급 설정이 필요합니다. 이는 `SystemInit()` 호출 전에 `ExitRun0Mode()`에서 처리됩니다.
