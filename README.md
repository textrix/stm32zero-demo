# STM32ZERO-DEMO

Demo projects for [STM32ZERO](https://github.com/textrix/stm32zero) library

[한국어](README.ko.md)

## Projects

### NUCLEO-H753ZI

Demo project for NUCLEO-H753ZI development board (STM32H753ZIT6, Cortex-M7).

**Features demonstrated:**
- Microsecond timer (`ustim`) with cascaded TIM3/TIM4/TIM12
- DMA-based serial I/O via USART3 (`sio`)
- FreeRTOS static task creation in DTCM RAM
- Section placement macros (`STM32ZERO_DTCM`)

## Project Structure

```
STM32ZERO-DEMO/
├── Main/
│   └── Src/
│       └── app_init.cpp        # Application entry point
├── STM32ZERO/                   # Library submodule
└── STM32ZERO-DEMO-NUCLEO-H753ZI/
    ├── Core/                    # STM32CubeMX generated code
    ├── Drivers/                 # HAL drivers
    └── CMakeLists.txt           # CMake build configuration
```

## Requirements

- STM32CubeIDE or CMake + ARM GCC toolchain
- C++17 or later

## Build

### CMake

```bash
cd STM32ZERO-DEMO-NUCLEO-H753ZI
cmake --preset Debug
cmake --build build/Debug
```

### STM32CubeIDE

1. Import project: `File > Import > Existing Projects into Workspace`
2. Select `STM32ZERO-DEMO-NUCLEO-H753ZI` folder
3. Build: `Project > Build Project`

## Clone

```bash
git clone --recursive https://github.com/textrix/stm32zero-demo.git
```

Or if already cloned:

```bash
git submodule update --init --recursive
```

## License

MIT License
