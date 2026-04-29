set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_SYSTEM_VERSION 1)

# ARM GCC 工具链路径
set(TOOLCHAIN_PREFIX "C:/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin")

# 用 FORCE 覆盖 CMake Tools kit 传来的编译器路径
set(CMAKE_C_COMPILER    "${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc.exe"   CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_PREFIX}/arm-none-eabi-g++.exe"  CACHE STRING "C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER  "${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc.exe"   CACHE STRING "ASM compiler" FORCE)
set(CMAKE_AR            "${TOOLCHAIN_PREFIX}/arm-none-eabi-ar.exe"    CACHE STRING "Archiver"     FORCE)
set(CMAKE_OBJCOPY       "${TOOLCHAIN_PREFIX}/arm-none-eabi-objcopy.exe" CACHE STRING "objcopy"   FORCE)
set(CMAKE_OBJDUMP       "${TOOLCHAIN_PREFIX}/arm-none-eabi-objdump.exe" CACHE STRING "objdump"   FORCE)
set(CMAKE_SIZE          "${TOOLCHAIN_PREFIX}/arm-none-eabi-size.exe"    CACHE STRING "size"      FORCE)

# 关键：跳过链接测试，交叉编译器不能链接 Windows exe
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "" FORCE)

# 告诉 CMake 不要验证编译器（避免主机链接器标志混入）
set(CMAKE_C_COMPILER_FORCED   TRUE CACHE BOOL "" FORCE)
set(CMAKE_CXX_COMPILER_FORCED TRUE CACHE BOOL "" FORCE)

# 编译器标志
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O2 -g1" CACHE STRING "" FORCE)

# 输出后缀
set(CMAKE_EXECUTABLE_SUFFIX ".elf" CACHE STRING "" FORCE)
