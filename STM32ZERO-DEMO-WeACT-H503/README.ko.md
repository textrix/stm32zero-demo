# STM32ZERO-DEMO-WeACT-H503

WeACT H503 보드용 데모 프로젝트 (STM32H503CB 마이크로컨트롤러).

## 개요

STM32H503CB는 최대 250 MHz로 동작하는 비용 효율적인 Cortex-M33 MCU입니다:
- 128 KB Flash 메모리
- 32 KB RAM
- 단정밀도 지원 하드웨어 FPU
- TrustZone 보안 (ARMv8-M)

## H5 메모리 아키텍처

복잡한 다중 도메인 메모리 레이아웃을 가진 STM32H7 시리즈와 달리, STM32H5는 단순하고 전통적인 메모리 구조를 특징으로 합니다:

| 영역  | 주소       | 크기   | 설명              |
|-------|------------|--------|-------------------|
| FLASH | 0x08000000 | 128 KB | 프로그램 저장소   |
| RAM   | 0x20000000 | 32 KB  | 단일 SRAM 영역    |

### H5 vs H7 비교

| 특징             | STM32H5 (H503)          | STM32H7 (H753)                   |
|------------------|-------------------------|----------------------------------|
| 코어             | Cortex-M33 (ARMv8-M)    | Cortex-M7 (ARMv7-M)              |
| RAM 레이아웃     | 단일 연속 영역          | 다중 분산 영역                   |
| TCM 메모리       | 없음                    | ITCMRAM (64KB) + DTCMRAM (128KB) |
| TrustZone        | 지원                    | 미지원                           |
| 캐시             | 없음                    | L1 명령어 캐시 + 데이터 캐시     |
| 스타트업         | 표준                    | 확장 (DTCMRAM 초기화)            |
| DMA 제약         | 없음                    | 도메인별 제약                    |

## 링커 스크립트 (.ld)

이 프로젝트는 두 가지 링커 스크립트 변형을 제공합니다. CubeMX 생성 기본값에서 **변경 없음**.

### 1. STM32H503xx_FLASH.ld (표준)

표준 Flash 빌드:
- FLASH에 코드 배치 (0x08000000)
- RAM에 Data/BSS 배치 (0x20000000)
- TLS (Thread Local Storage) 섹션 포함

### 2. STM32H503xx_RAM.ld (디버그)

디버깅용 RAM 전용 실행:
- 모든 섹션을 RAM에 로드
- 개발 중 빠른 반복 작업 가능
- Flash 프로그래밍 불필요

### TLS 섹션

H5 링커 스크립트는 Thread Local Storage 지원을 포함합니다:

```
.tdata    - 초기화된 스레드 로컬 데이터
.tbss     - 영초기화된 스레드 로컬 데이터
```

이 섹션들은 Cortex-M33용 표준 CubeMX 출력의 일부이며 C11/C++11의 `_Thread_local` / `thread_local` 키워드를 지원합니다.

## 스타트업 어셈블리 (.s)

스타트업 파일 (`startup_stm32h503xx.s`)은 변경 없이 **표준 STM32 스타트업 시퀀스**를 사용합니다.

### Reset_Handler 시퀀스

```
1. 스택 포인터 설정
2. Flash에서 RAM으로 .data 복사
3. .bss 영초기화
4. SystemInit()
5. __libc_init_array (C++ 생성자)
6. main()
```

### H7과의 주요 차이점

| 항목             | H5 스타트업                 | H7 스타트업                     |
|------------------|-----------------------------|---------------------------------|
| 전원 설정        | 불필요                      | SystemInit 전에 ExitRun0Mode()  |
| SystemInit 호출  | .data/.bss 초기화 후        | .data/.bss 초기화 전            |
| DTCMRAM 초기화   | 해당 없음                   | 별도 복사/영초기화 루프         |
| 추가 섹션        | 없음                        | DTCMRAM .data/.bss              |

## 사용 가이드

### 링커 스크립트 선택

| 사용 사례             | 링커 스크립트          |
|-----------------------|------------------------|
| 일반 개발             | STM32H503xx_FLASH.ld   |
| 빠른 디버그 반복      | STM32H503xx_RAM.ld     |

### DMA 버퍼 배치

H7과 달리 H5는 DMA 접근 제약이 없습니다. 모든 RAM이 특별한 섹션 배치 없이 DMA로 접근 가능합니다:

```c
/* 간단한 선언 - 특별한 속성 불필요 */
uint8_t dma_buffer[256];
```

### Thread Local Storage

TLS 변수 사용 시 (C11/C++11):

```c
_Thread_local int per_thread_counter = 0;
```

## STM32ZERO 섹션 매크로 호환성

STM32ZERO 라이브러리는 H7 전용 메모리 영역을 위한 섹션 배치 매크로를 제공합니다:

```c
STM32ZERO_ITCM        // .itcmram 섹션
STM32ZERO_DTCM        // .dtcmram 섹션 (BSS)
STM32ZERO_DTCM_DATA   // .dtcmram_data 섹션 (초기화됨)
STM32ZERO_DMA         // .dma 섹션
STM32ZERO_DMA_TX      // .dma_tx 섹션
STM32ZERO_DMA_RX      // .dma_rx 섹션
```

**H5에서 이 매크로들은 문제없이 동작합니다.** H5 링커 스크립트에는 이러한 특수 섹션이 정의되어 있지 않으므로, 이 매크로를 사용하는 변수들은 자동으로 RAM의 기본 `.data` 또는 `.bss` 섹션으로 배치됩니다.

```c
/* 이 코드는 H5와 H7 모두에서 동작함 */
STM32ZERO_DMA_RX uint8_t rx_buffer[256];  // H7: .dma_rx에 배치 (SRAM1)
                                          // H5: .bss에 배치 (RAM)

STM32ZERO_DTCM_DATA int fast_var = 100;   // H7: .dtcmram_data에 배치 (DTCMRAM)
                                          // H5: .data에 배치 (RAM)
```

이를 통해 메모리 배치를 위한 `#ifdef` 가드 없이 H5와 H7 프로젝트 간에 동일한 소스 코드를 공유할 수 있습니다.

## 중요 사항

1. **캐시 관리 불필요**: H7과 달리 H5는 L1 캐시가 없습니다. DMA를 위한 캐시 관리 작업이 필요 없습니다.

2. **단순한 메모리 모델**: 단일 RAM 영역은 성능 최적화를 위한 메모리 배치 고려가 불필요함을 의미합니다.

3. **TrustZone**: Cortex-M33은 TrustZone을 지원하지만, 이 데모 프로젝트는 보안/비보안 파티셔닝을 구성하지 않습니다.

4. **표준 스타트업**: 링커 스크립트나 스타트업 파일에 커스텀 변경 없음. 대부분의 사용 사례에서 CubeMX 기본값으로 충분합니다.

5. **크로스 플랫폼 코드**: STM32ZERO 섹션 매크로를 자유롭게 사용 가능. H5에서는 효과가 없고 변수들이 표준 섹션으로 배치됩니다.
