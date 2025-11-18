set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

# ARM GCC 工具链路径（请改成你的实际路径）
set(TOOLCHAIN_PREFIX "D:/Program Files (x86)/Arm GNU Toolchain arm-none-eabi/14.3 rel1/bin")

set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc.exe")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/arm-none-eabi-g++.exe")
set(CMAKE_AR "${TOOLCHAIN_PREFIX}/arm-none-eabi-ar.exe")
set(CMAKE_OBJCOPY "${TOOLCHAIN_PREFIX}/arm-none-eabi-objcopy.exe")
set(CMAKE_OBJDUMP "${TOOLCHAIN_PREFIX}/arm-none-eabi-objdump.exe")
set(CMAKE_SIZE "${TOOLCHAIN_PREFIX}/arm-none-eabi-size.exe")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
