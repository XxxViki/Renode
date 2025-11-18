cmake_minimum_required(VERSION 3.15)

project(MyRtosF103 C ASM)

# 使用 ARM 交叉编译
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain-arm-none-eabi.cmake)

set(CMAKE_C_STANDARD 11)

# 启动文件、链接脚本路径
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/stm32f103.ld)

# 编译选项
add_compile_options(
    -mcpu=cortex-m3
    -mthumb
    -O0
    -g3
)

add_link_options(
    -T${LINKER_SCRIPT}
    -nostartfiles
    -Wl,--gc-sections
)

# 源文件
file(GLOB SOURCES
    src/*.c
    rtos/*.c
    drivers/*.c
    startup/*.s
)

add_executable(firmware.elf
    src/main.c
    src/system_stm32f1xx.c
    startup/startup_stm32f103.s
)

target_link_options(firmware.elf PRIVATE -T${CMAKE_SOURCE_DIR}/linker/stm32f103.ld -nostartfiles -Wl,--gc-sections)
