# STM32ZERO-DEMO

[STM32ZERO](https://github.com/textrix/stm32zero) 라이브러리 데모 프로젝트

[English](README.md)

## 프로젝트

### NUCLEO-H753ZI

NUCLEO-H753ZI 개발 보드 (STM32H753ZIT6, Cortex-M7) 데모 프로젝트.

**데모 기능:**
- TIM3/TIM4/TIM12 캐스케이드 연결 마이크로초 타이머 (`ustim`)
- USART3 DMA 기반 시리얼 I/O (`sio`)
- DTCM RAM에 FreeRTOS 정적 태스크 생성
- 섹션 배치 매크로 (`STM32ZERO_DTCM`)

**문서:** [README (English)](STM32ZERO-DEMO-NUCLEO-H753ZI/README.md) | [README](STM32ZERO-DEMO-NUCLEO-H753ZI/README.ko.md)
- H7 메모리 아키텍처 (DTCMRAM, AXI SRAM, SRAM1-4)
- 캐시 & MPU 설정
- 수정된 링커 스크립트 및 스타트업 파일

### WeACT-H503

WeACT STM32H503 개발 보드 (STM32H503CBT6, Cortex-M33) 데모 프로젝트.

**데모 기능:**
- TIM2/TIM3 캐스케이드 연결 마이크로초 타이머 (`ustim`)
- USART1 DMA 기반 시리얼 I/O (`sio`)
- FreeRTOS 정적 태스크 생성
- H5 시리즈 호환성 (캐시 없음, 단순화된 메모리 모델)

**문서:** [README (English)](STM32ZERO-DEMO-WeACT-H503/README.md) | [README](STM32ZERO-DEMO-WeACT-H503/README.ko.md)
- 단순한 메모리 모델 (단일 RAM 영역)
- STM32ZERO 섹션 매크로 호환성
- 표준 링커 스크립트 (변경 없음)

## 런타임 테스트

실제 하드웨어에서 라이브러리 기능을 검증하는 런타임 테스트 스위트 포함.

**UART 테스트 출력 (115200 baud):**

```
--- USTIM Tests ---
[PASS] ustim::get() monotonic (t2 >= t1)
[PASS] vTaskDelay(10ms) x10 (min 9679, max 9999, avg 9967)
[PASS] ustim::spin(100) (expected 100~200, actual 101)
```

**테스트 매크로:**
- `TEST_ASSERT(cond, desc)` - 단순 pass/fail
- `TEST_ASSERT_EQ(actual, expected, desc)` - 정확한 값 일치
- `TEST_ASSERT_RANGE(actual, min, max, desc)` - 범위 검사 + 실제값 표시
- `TEST_ASSERT_STATS(stats, min, max, desc)` - 통계 테스트 (반복 측정 min/max/avg)

## 프로젝트 구조

```
STM32ZERO-DEMO/
├── Main/
│   └── Src/
│       ├── app_init.cpp        # 애플리케이션 진입점
│       ├── test_runner.cpp     # 테스트 프레임워크 및 러너
│       ├── test_core.cpp       # Core 모듈 테스트
│       ├── test_sio.cpp        # 시리얼 I/O 테스트
│       ├── test_freertos.cpp   # FreeRTOS 래퍼 테스트
│       └── test_ustim.cpp      # 마이크로초 타이머 테스트
├── STM32ZERO/                   # 라이브러리 서브모듈
├── STM32ZERO-DEMO-NUCLEO-H753ZI/
│   ├── Core/                    # STM32CubeMX 생성 코드
│   ├── Drivers/                 # HAL 드라이버
│   └── CMakeLists.txt           # CMake 빌드 설정
└── STM32ZERO-DEMO-WeACT-H503/
    ├── Core/                    # STM32CubeMX 생성 코드
    ├── Drivers/                 # HAL 드라이버
    └── CMakeLists.txt           # CMake 빌드 설정
```

## 요구사항

- STM32CubeIDE 또는 CMake + ARM GCC 툴체인
- C++17 이상

## 빌드

### CMake

```bash
cd STM32ZERO-DEMO-NUCLEO-H753ZI
cmake --preset Debug
cmake --build build/Debug
```

### STM32CubeIDE

1. 프로젝트 임포트: `File > Import > Existing Projects into Workspace`
2. `STM32ZERO-DEMO-NUCLEO-H753ZI` 폴더 선택
3. 빌드: `Project > Build Project`

## 클론

```bash
git clone --recursive https://github.com/textrix/stm32zero-demo.git
```

이미 클론한 경우:

```bash
git submodule update --init --recursive
```

## 라이선스

MIT License
