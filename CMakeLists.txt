cmake_minimum_required(VERSION 3.15)

# Toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain-arm-none-eabi.cmake)

project(MyRtosF412 C ASM)
set(CMAKE_C_STANDARD 11)

# Paths (separate kernel / port / app)
set(RTOS_DIR        ${CMAKE_SOURCE_DIR}/rtos)
set(RTOS_SRC_DIR    ${RTOS_DIR}/source)
set(RTOS_CONFIG_DIR ${RTOS_DIR}/config)
set(PORT_DIR        ${CMAKE_SOURCE_DIR}/ports/arm-cortex-m)
set(APP_DIR         ${CMAKE_SOURCE_DIR}/apps/blinky)

# Device / CPU Defines
set(TARGET_CPU cortex-m4)

add_compile_definitions(
    STM32F412Zx            # Device macro (enables correct register definitions)
    USE_FULL_LL_DRIVER     # Keep LL driver macros available
)

# CMSIS Startup + System + Linker
set(STARTUP_FILE   ${CMAKE_SOURCE_DIR}/drivers/Projects/STM32F412ZG-Nucleo/Templates/STM32CubeIDE/Example/Startup/startup_stm32f412zgjx.s)
set(SYSTEM_FILE    ${CMAKE_SOURCE_DIR}/cmsis/Source/Templates/system_stm32f4xx.c)
set(LINKER_SCRIPT  ${CMAKE_SOURCE_DIR}/drivers/Projects/STM32F412ZG-Nucleo/Templates/STM32CubeIDE/STM32F412ZGJX_FLASH.ld)

# Compiler Flags
add_compile_options(
    -mcpu=${TARGET_CPU}
    -mthumb
    -O0
    -g3
    -Wall
    -fdata-sections
    -ffunction-sections
)

# Linker Flags
add_link_options(
    -T${LINKER_SCRIPT}
    -mcpu=${TARGET_CPU}
    -mthumb
    -nostartfiles
    -Wl,--gc-sections
    -specs=nosys.specs
    -specs=nano.specs
)

# Source Files
file(GLOB APP_SOURCES   ${APP_DIR}/*.c)
file(GLOB PORT_SOURCES  ${PORT_DIR}/*.c)
file(GLOB RTOS_SOURCES  ${RTOS_SRC_DIR}/*.c)

add_executable(firmware.elf
    ${APP_SOURCES}
    ${PORT_SOURCES}
    ${RTOS_SOURCES}
    ${SYSTEM_FILE}
    ${STARTUP_FILE}
)

# Includes (minimize exposure)
target_include_directories(firmware.elf PRIVATE
    ${APP_DIR}
    ${RTOS_SRC_DIR}
    ${RTOS_CONFIG_DIR}
    ${PORT_DIR}

    ${CMAKE_SOURCE_DIR}/drivers/Drivers/CMSIS/Core/Include  # core_cm4.h
    ${CMAKE_SOURCE_DIR}/cmsis/Include                       # stm32f412xx.h
)
