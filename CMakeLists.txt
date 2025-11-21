cmake_minimum_required(VERSION 3.15)

# -----------------------------
# Toolchain
# -----------------------------
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain-arm-none-eabi.cmake)

project(MyRtosF412 C ASM)
set(CMAKE_C_STANDARD 11)

# -----------------------------
# Device / CPU Defines
# -----------------------------
set(TARGET_CPU cortex-m4)

add_compile_definitions(
    STM32F412Zx            # Device macro（启用正确的寄存器定义）
    USE_FULL_LL_DRIVER     # 如果你使用 LL 或裸寄存器，也可以不写
)

# -----------------------------
# CMSIS Startup + System + Linker
# -----------------------------
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/drivers/Projects/STM32F412ZG-Nucleo/Templates/STM32CubeIDE/Example/Startup/startup_stm32f412zgjx.s)
set(SYSTEM_FILE  ${CMAKE_SOURCE_DIR}/cmsis/Source/Templates/system_stm32f4xx.c)
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/drivers/Projects/STM32F412ZG-Nucleo/Templates/STM32CubeIDE/STM32F412ZGJX_FLASH.ld)

# -----------------------------
# Compiler Flags
# -----------------------------
add_compile_options(
    -mcpu=${TARGET_CPU}
    -mthumb
    -O0
    -g3
    -Wall
    -fdata-sections
    -ffunction-sections
)

# -----------------------------
# Linker Flags
# -----------------------------
add_link_options(
    -T${LINKER_SCRIPT}
    -mcpu=${TARGET_CPU}
    -mthumb
    -nostartfiles
    -Wl,--gc-sections
    -specs=nosys.specs
    -specs=nano.specs
)

# -----------------------------
# Includes（最小化）
# -----------------------------
include_directories(
    src
    rtos/source
    rtos/config
    rtos_port
 
    drivers/Drivers/CMSIS/Core/Include  # core_cm4.h
    cmsis/Include           # stm32f412xx.h
)

# -----------------------------
# Source Files
# -----------------------------
file(GLOB SOURCES
    src/*.c
    rtos/source/*.c
    rtos_port/*.c
)

# 添加 CMSIS 启动/系统文件
list(APPEND SOURCES
    ${SYSTEM_FILE}
    ${STARTUP_FILE}
)

# -----------------------------
# Final binary
# -----------------------------
add_executable(firmware.elf ${SOURCES})
