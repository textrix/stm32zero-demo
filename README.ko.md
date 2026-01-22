# STM32ZERO-DEMO

[STM32ZERO](https://github.com/textrix/stm32zero) 라이브러리 데모 프로젝트

[English](README.md)

## 프로젝트

### NUCLEO-H753ZI

NUCLEO-H753ZI 개발 보드 (STM32H753ZIT6, Cortex-M7) 데모 프로젝트.

**데모 기능:**
- TIM3/TIM4/TIM12 캐스케이드 연결 마이크로초 타이머 (`ustim`)
- USART3 DMA 기반 stdout (`sout`)
- readline이 포함된 DMA 기반 stdin (`sin`)
- DTCM RAM에 FreeRTOS 정적 태스크 생성
- 섹션 배치 매크로 (`STM32ZERO_DTCM`)

## 프로젝트 구조

```
STM32ZERO-DEMO/
├── Main/
│   └── Src/
│       └── app_init.cpp        # 애플리케이션 진입점
├── STM32ZERO/                   # 라이브러리 서브모듈
└── STM32ZERO-DEMO-NUCLEO-H753ZI/
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
