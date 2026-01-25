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

## Runtime Tests

The demo includes a comprehensive runtime test suite that validates library functionality on actual hardware.

**Test output via UART (115200 baud):**

```
--- USTIM Tests ---
[PASS] ustim::get() monotonic (t2 >= t1)
[PASS] vTaskDelay(10ms) x10 (min 9679, max 9999, avg 9967)
[PASS] ustim::spin(100) (expected 100~200, actual 101)
```

**Test macros:**
- `TEST_ASSERT(cond, desc)` - Simple pass/fail
- `TEST_ASSERT_EQ(actual, expected, desc)` - Exact value match
- `TEST_ASSERT_RANGE(actual, min, max, desc)` - Range check with actual value display
- `TEST_ASSERT_STATS(stats, min, max, desc)` - Statistical test (min/max/avg over iterations)

## Project Structure

```
STM32ZERO-DEMO/
├── Main/
│   └── Src/
│       ├── app_init.cpp        # Application entry point
│       ├── test_runner.cpp     # Test framework and runner
│       ├── test_core.cpp       # Core module tests
│       ├── test_sio.cpp        # Serial I/O tests
│       ├── test_freertos.cpp   # FreeRTOS wrapper tests
│       └── test_ustim.cpp      # Microsecond timer tests
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
