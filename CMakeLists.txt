cmake_minimum_required(VERSION 3.15)

project(MyRtosF103 C ASM)

# Cross compile using ARM toolchain
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain-arm-none-eabi.cmake)

set(CMAKE_C_STANDARD 11)

# Linker/startup paths
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/linker/stm32f103.ld)
set(STARTUP_FILE ${CMAKE_SOURCE_DIR}/cmsis/Source/Templates/gcc/startup_stm32f103xb.s)
set(SYSTEM_FILE ${CMAKE_SOURCE_DIR}/cmsis/Source/Templates/system_stm32f1xx.c)

# Compile options
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
    -mcpu=cortex-m3
    -mthumb
    -specs=nosys.specs
    -specs=nano.specs
)

# Include paths
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/rtos

    # HAL + CMSIS
    ${CMAKE_SOURCE_DIR}/cmsis/Include
    ${CMAKE_SOURCE_DIR}/cmsis/Core/Include
    ${CMAKE_SOURCE_DIR}/drivers/Inc
    ${CMAKE_SOURCE_DIR}/drivers/Inc/Legacy
)

# Select target device
add_compile_definitions(STM32F103xB)

# Sources
file(GLOB SOURCES
    src/*.c
    rtos/*.c
    drivers/Src/*.c
)
# Exclude HAL timebase template duplicates
list(FILTER SOURCES EXCLUDE REGEX "stm32f1xx_hal_timebase_.*\\.c$")
list(APPEND SOURCES
    ${SYSTEM_FILE}
    ${STARTUP_FILE}
)

add_executable(firmware.elf
    ${SOURCES}
)

target_link_options(firmware.elf PRIVATE -T${LINKER_SCRIPT} -nostartfiles -Wl,--gc-sections)
