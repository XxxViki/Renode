# VS Code + Renode 嵌入式调试完全手册

> 从零搭建 ARM 嵌入式 Renode 调试环境，踩坑全记录
> 适用：STM32F4 / Cortex-M 系列，可扩展到其他架构
> 日期：2026-04-29

---

## 目录

1. [环境准备](#1-环境准备)
2. [项目结构](#2-项目结构)
3. [CMake 交叉编译配置](#3-cmake-交叉编译配置)
4. [Renode 模拟脚本](#4-renode-模拟脚本)
5. [VS Code 调试配置](#5-vs-code-调试配置)
6. [VS Code 任务配置](#6-vs-code-任务配置)
7. [调试工作流](#7-调试工作流)
8. [查看寄存器和内存](#8-查看寄存器和内存)
9. [常见问题与解决](#9-常见问题与解决)
10. [快速参考卡](#10-快速参考卡)

---

## 1. 环境准备

### 必须安装

| 工具 | 用途 | 下载 |
|------|------|------|
| **Renode** | ARM 模拟器 | https://renode.io |
| **VS Code** | IDE + 调试前端 | https://code.visualstudio.com |
| **arm-none-eabi-gcc** | ARM 交叉编译器 | STM32CubeCLT 或 ARM GNU Toolchain |
| **CMake + Ninja** | 构建系统 | 通常随编译器一起安装 |

### 可选

| 工具 | 用途 |
|------|------|
| **STM32CubeCLT** | 含 SVD 文件（寄存器描述） |

### 环境变量

确保以下路径在 PATH 中：
```
D:\Renode                          ← renode.exe
C:\ST\STM32CubeCLT_1.18.0\...\bin  ← arm-none-eabi-gcc.exe
```

---

## 2. 项目结构

```
project/
├── CMakeLists.txt                       # CMake 构建定义
├── STM32F412ZGJX_FLASH.ld               # 链接脚本
├── cmake/
│   └── toolchain-arm-none-eabi.cmake    # ⭐ 交叉编译工具链
├── cmsis/                               # CMSIS 启动文件
├── apps/                                # 应用代码 (main.c)
├── ports/                               # 移植层
├── rtos/                                # RTOS 内核
├── renode/
│   └── platform.resc                    # ⭐ Renode 模拟脚本
├── build/                               # CMake 输出 (自动生成)
│   └── firmware.elf                     # ⭐ 调试用的 ELF
└── .vscode/
    ├── launch.json                      # ⭐ 调试配置
    ├── tasks.json                       # ⭐ 构建任务
    └── settings.json                    # CMake 设置
```

---

## 3. CMake 交叉编译配置

### 3.1 toolchain-arm-none-eabi.cmake（完整模板）

```cmake
# ── 必须最先设置 ──
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)
set(CMAKE_SYSTEM_VERSION 1)

# ── 编译器路径 ──
set(TOOLCHAIN_PREFIX "C:/ST/STM32CubeCLT_1.18.0/GNU-tools-for-STM32/bin")

# ⚠️ 全部用 CACHE STRING "" FORCE，防止 CMake Tools kit 覆盖
set(CMAKE_C_COMPILER    "${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc.exe"   CACHE STRING "C compiler"   FORCE)
set(CMAKE_CXX_COMPILER  "${TOOLCHAIN_PREFIX}/arm-none-eabi-g++.exe"  CACHE STRING "C++ compiler" FORCE)
set(CMAKE_ASM_COMPILER  "${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc.exe"   CACHE STRING "ASM compiler" FORCE)
set(CMAKE_AR            "${TOOLCHAIN_PREFIX}/arm-none-eabi-ar.exe"    CACHE STRING "Archiver"     FORCE)
set(CMAKE_OBJCOPY       "${TOOLCHAIN_PREFIX}/arm-none-eabi-objcopy.exe" CACHE STRING "objcopy"   FORCE)
set(CMAKE_OBJDUMP       "${TOOLCHAIN_PREFIX}/arm-none-eabi-objdump.exe" CACHE STRING "objdump"   FORCE)
set(CMAKE_SIZE          "${TOOLCHAIN_PREFIX}/arm-none-eabi-size.exe"    CACHE STRING "size"      FORCE)

# ⚠️ 关键：跳过链接测试（交叉编译器不能链接 Windows exe）
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "" FORCE)

# ⚠️ 跳过编译器验证
set(CMAKE_C_COMPILER_FORCED   TRUE CACHE BOOL "" FORCE)
set(CMAKE_CXX_COMPILER_FORCED TRUE CACHE BOOL "" FORCE)

# 调试/发布标志
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O2 -g1" CACHE STRING "" FORCE)

# ELF 后缀
set(CMAKE_EXECUTABLE_SUFFIX ".elf" CACHE STRING "" FORCE)
```

### 3.2 settings.json（VS Code CMake Tools）

```json
{
    "cmake.configureOnOpen": true,
    "cmake.generator": "Ninja",
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "cmake.sourceDirectory": "${workspaceFolder}",
    "cmake.configureArgs": [
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
        "-DCMAKE_TOOLCHAIN_FILE=${workspaceFolder}/cmake/toolchain-arm-none-eabi.cmake"
    ],
    "cmake.buildArgs": [],
    "cmake.debugConfig": {
        "name": "Debug on Renode (Cortex-Debug)",
        "type": "cortex-debug",
        "request": "launch",
        "servertype": "external",
        "gdbTarget": "localhost:3333",
        "executable": "${workspaceFolder}/build/firmware.elf"
    }
}
```

**⚠️ 容易踩的坑：**
- VS Code 底部状态栏必须选 **ARM 交叉编译器 kit**（不是 MinGW）
- 如果看到 `unrecognized option '--major-image-version'`，说明 toolchain 的 `CACHE FORCE` 没写对

---

## 4. Renode 模拟脚本

### 4.1 platform.resc（完整模板）

```python
using sysbus

# ── 1. 创建机器 ──
mach create "myboard"

# ── 2. 加载平台描述（Renode 内置）──
machine LoadPlatformDescription @platforms/cpus/stm32f4.repl

# ── 3. 启动 GDB 服务器 ⚠️ 必须在加载 ELF 之前 ──
machine StartGdbServer 3333 true

# ── 4. 加载固件 ⚠️ 用绝对路径 + 引号 ──
sysbus LoadELF "D:/path/to/project/build/firmware.elf"

# ── 5. 打开串口（如果用到 printf）──
showAnalyzer sysbus.usart2

# ── 6. 不调用 start！让 GDB 控制 CPU ──
# （这里不要写 start）

# ── 7. 复位宏（调试 restart 时用）──
macro myboard.reset
"""
    sysbus LoadELF "D:/path/to/project/build/firmware.elf"
    cpu Reset
"""
```

### 4.2 platform.resc 关键规则

| 规则 | 说明 |
|------|------|
| `@` 路径 | 相对于脚本所在目录 |
| 不带 `@` 的路径 | 相对于 Renode 进程的 cwd |
| ⚠️ ELF 路径 | 建议用 **绝对路径 + 引号**（`sysbus LoadELF` 不认 `@../` 语法） |
| ⚠️ `start` | **不要写**，否则 GDB 连上时 CPU 已跑飞 |
| ⚠️ GDB 顺序 | GDB Server → ELF → UART（先开 Server 再加载） |

---

## 5. VS Code 调试配置

### 5.1 ⭐ 推荐：Cortex-Debug（含外设视图）

[Cortex-Debug](https://marketplace.visualstudio.com/items?itemName=marus25.cortex-debug) 是专为 ARM Cortex-M 设计的调试扩展，支持：
- 🧩 **外设视图** — 通过 SVD 文件图形化展示所有外设寄存器
- 内核寄存器视图（R0-R15、PC、SP、CPSR 等）
- RTOS 感知（FreeRTOS 线程列表）

#### launch.json（Cortex-Debug + Renode）

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug STM32F412 on Renode (Cortex-Debug)",
            "type": "cortex-debug",
            "request": "launch",
            "servertype": "external",
            "gdbTarget": "localhost:3333",
            "executable": "${workspaceFolder}/build/firmware.elf",
            "cwd": "${workspaceFolder}",
            "armToolchainPath": "C:\\ST\\STM32CubeCLT_1.18.0\\GNU-tools-for-STM32\\bin",
            "svdFile": "C:\\ST\\STM32CubeCLT_1.18.0\\STMicroelectronics_CMSIS_SVD\\STM32F412.svd",
            "preLaunchTask": "Start Renode",
            "postDebugTask": "Stop Renode",
            "runToEntryPoint": "main",
            "showDevDebugOutput": "raw"
        }
    ]
}
```

#### 关键字段

| 字段 | 必须 | 说明 |
|------|------|------|
| `type` | ✅ | `"cortex-debug"` |
| `servertype` | ✅ | `"external"` — Renode 已提供 GDB Server |
| `gdbTarget` | ✅ | `"localhost:3333"` — 连接 Renode |
| `executable` | ✅ | ELF 文件（用于符号解析，非加载） |
| `armToolchainPath` | ✅ | `arm-none-eabi-gdb.exe` 所在目录 |
| `svdFile` | ⭐ | SVD 文件路径 → **开启外设视图的关键** |
| `runToEntryPoint` | 推荐 | `"main"` — 在 main 函数入口自动暂停 |
| `showDevDebugOutput` | 可选 | `"raw"` — 调试 SVD 解析问题时开启 |

> ℹ️ `servertype: "external"` 表示 GDB Server 由外部提供（Renode），Cortex-Debug 只负责启动 GDB 客户端去连接。不会执行 `load` 命令。

#### SVD 文件从哪里来？

| 来源 | 路径规律 |
|------|----------|
| STM32CubeCLT | `C:\ST\STM32CubeCLT_*\STMicroelectronics_CMSIS_SVD\STM32F*.svd` |
| STM32CubeIDE | `C:\ST\STM32CubeIDE_*\plugins\com.st.stm32cube.ide.mcu.productdb.debug_*\resources\cmsis\STMicroelectronics_CMSIS_SVD\` |
| 社区仓库 | https://github.com/posborne/cmsis-svd |
| Keil DFP | `Keil_v5\ARM\PACK\STMicroelectronics\STM32F4xx_DFP\*\CMSIS\SVD\` |

> ⚠️ SVD 版本要匹配你的芯片型号。`STM32F412.svd` 覆盖 F412 全系列；如果用 F407 就要找 `STM32F407.svd`。

#### 验证 SVD 已生效

调试启动后：
1. 左侧边栏点击 🧩 Cortex-Debug 图标
2. 展开 **Peripherals** → 看到 `GPIOA`, `USART2`, `RCC` 等 → ✅ SVD 工作正常
3. 如果 Peripherals 节点为空或不显示 → Debug Console 查看 `[SVD]` 日志

---

### 5.2 备选：cppdbg（无外设视图）

Microsoft C/C++ 扩展的 `cppdbg` 是通用 GDB 前端，**不支持外设视图**，只能通过 Debug Console 敲 GDB 命令查看寄存器。

#### launch.json（cppdbg + Renode）

```json
{
    "name": "Debug STM32F412 on Renode (cppdbg)",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/firmware.elf",
    "cwd": "${workspaceFolder}",
    "miDebuggerPath": "C:\\ST\\STM32CubeCLT_1.18.0\\GNU-tools-for-STM32\\bin\\arm-none-eabi-gdb.exe",
    "miDebuggerServerAddress": "localhost:3333",
    "stopAtEntry": true,
    "preLaunchTask": "Start Renode",
    "postDebugTask": "Stop Renode",
    "setupCommands": [
        { "text": "set pagination off" },
        { "text": "set confirm off" },
        { "text": "set print pretty on" },
        { "text": "set mem inaccessible-by-default off" }
    ],
    "externalConsole": false,
    "MIMode": "gdb",
    "targetArchitecture": "arm"
}
```

#### cppdbg 字段说明

| 字段 | 说明 |
|------|------|
| `program` | ELF 文件（给 GDB 加载符号） |
| `miDebuggerPath` | arm-none-eabi-gdb.exe 路径 |
| `miDebuggerServerAddress` | GDB Server 地址（Renode 的 :3333） |
| `stopAtEntry` | 在入口点（Reset_Handler）暂停 |
| `setupCommands` | GDB 连接后自动执行的命令 |
| `targetArchitecture` | `"arm"` |

#### setupCommands 注意事项

- ⚠️ **不要写 `"file ${workspaceFolder}/build/firmware.elf"`** — `${workspaceFolder}` 展开后的反斜杠 `\` 会被 JSON 转义破坏
- `"program"` 字段已经指定 ELF，`file` 命令是多余的
- `"set mem inaccessible-by-default off"` 允许 WATCH 窗口读任意内存地址

---

## 6. VS Code 任务配置

### 6.1 tasks.json（完整模板）

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Build",
            "type": "shell",
            "command": "cmake",
            "args": ["--build", "${workspaceFolder}/build"],
            "problemMatcher": ["$gcc"]
        },
        {
            "label": "Start Renode",
            "type": "shell",
            "command": "renode",
            "args": ["-e", "include @renode/platform.resc"],
            "options": { "cwd": "${workspaceFolder}" },
            "isBackground": true,
            "problemMatcher": {
                "owner": "renode",
                "pattern": {
                    "regexp": "^.*\\[ERROR\\].*$",
                    "message": 0
                },
                "background": {
                    "activeOnStart": true,
                    "beginsPattern": "Loaded monitor commands",
                    "endsPattern": "Loading block"
                }
            }
        },
        {
            "label": "Stop Renode",
            "type": "shell",
            "command": "powershell",
            "args": ["-Command", "Stop-Process -Name renode -Force -ErrorAction SilentlyContinue"],
            "problemMatcher": []
        }
    ]
}
```

### 6.2 background task 工作方式

```
F5 → preLaunchTask "Start Renode"
       ├─ 启动 renode
       ├─ 等待匹配 beginsPattern: "Loaded monitor commands"  → 确认进程启动了
       ├─ 等待匹配 endsPattern:   "Loading block"            → 确认固件加载好了
       └─ VS Code 认为任务就绪
→ GDB 连接 :3333
→ 调试开始
→ 停止调试 → postDebugTask "Stop Renode" → 杀进程
```

### 6.3 problemMatcher 注意事项

- ⚠️ **不要把正则写成 `^(.*)$`** — 这会让所有 Renode 输出都变成 VS Code Problems 面板的错误
- 只匹配真正的错误：`^.*\\[ERROR\\].*$`
- `beginsPattern` / `endsPattern` 是 VS Code 判断后台任务进度的依据

---

## 7. 调试工作流

### 日常流程

```
1. VS Code 底部选 kit：GCC arm-none-eabi
2. Ctrl+Shift+P → CMake: Delete Cache and Reconfigure  （仅改 CMake 后需要）
3. Ctrl+Shift+P → CMake: Build
4. F5 ← 一键调试
5. F5 → 继续执行
6. F10 → 单步跳过
7. F11 → 单步进入
8. Shift+F5 → 停止调试 → Renode 自动退出
```

### 查看 UART 输出

Renode 启动后会自动弹出 **UART2 Analyzer** 窗口（因为 `showAnalyzer`），printf 输出会显示在这里。

### 重启调试

改代码后：
```
1. Ctrl+Shift+B → Build
2. F5
```

---

## 8. 查看寄存器和内存

### 四层查看系统

| 方式 | 在哪里 | 适合 | 推荐 |
|------|--------|------|------|
| 🧩 **Cortex-Debug 外设视图** | VS Code 左侧栏 | 外设寄存器（最直观！） | ⭐⭐⭐ |
| Renode Monitor | Renode 终端窗口 | 外设属性、快速验证 | ⭐⭐ |
| VS Code Debug Console | VS Code 底部面板 | 内存 dump、GDB 命令 | ⭐⭐ |
| VS Code WATCH | VS Code 左侧面板 | 变量、地址监视 | ⭐ |

---

### 8.1 ⭐ 推荐：Cortex-Debug 外设视图

这是查看外设寄存器**最舒服**的方式 — 不需要记地址，不需要敲命令，展开树就能看到所有寄存器和位字段。

#### 打开方式

```
F5 启动调试 → 断点命中/手动暂停
→ 左侧栏点击 🧩 Cortex-Debug 图标
→ 展开 Peripherals 节点
→ 找到目标外设，展开即可
```

#### 能做什么

| 操作 | 方法 |
|------|------|
| 查看寄存器值 | 展开外设 → 展开寄存器 → 看到每个字段的值 |
| **修改寄存器值** | 双击数值 → 输入新值 → 回车（调试暂停时生效） |
| 查看位字段含义 | 悬停在字段名上 → 显示描述 |
| 查看读写权限 | 字段前有 `r` / `rw` / `w` 标记 |

#### 典型外设展开示例

```
🧩 Peripherals
  ├─ GPIOA
  │  ├─ MODER    (0x40020000) = 0xA8000000
  │  │  ├─ MODER0  [1:0]   = 0b10 (Alternate function)
  │  │  ├─ MODER1  [3:2]   = 0b10
  │  │  └─ ...
  │  ├─ ODR      (0x40020014) = 0x00000020
  │  │  ├─ ODR0    [0]     = 0
  │  │  ├─ ODR5    [5]     = 1  ← PA5 输出高
  │  │  └─ ...
  │  └─ ...
  ├─ USART2
  │  ├─ SR       (0x40004400) = 0x000000C0
  │  │  ├─ TXE     [7]     = 1  ← 发送空
  │  │  ├─ TC      [6]     = 1  ← 发送完成
  │  │  └─ ...
  │  ├─ DR       (0x40004404)
  │  └─ BRR      (0x40004408)
  ├─ RCC
  │  ├─ CR       (0x40023800)
  │  │  ├─ HSEON   [16]    = 1
  │  │  ├─ HSERDY  [17]    = 1
  │  │  └─ ...
  │  └─ AHB1ENR  (0x40023830)
  │     ├─ GPIOAEN [0]     = 1  ← GPIOA 时钟已使能
  │     └─ ...
  └─ ...
```

#### 不显示外设视图？

| 症状 | 可能原因 |
|------|----------|
| Peripherals 节点不出现 | `svdFile` 路径错误 / SVD 文件不存在 |
| Peripherals 为空 | SVD 解析失败 → Debug Console 看 `[SVD]` 日志 |
| 寄存器值全是 0 | a) 外设时钟未使能（查 RCC） b) 调试未暂停 |
| 值不更新 | 必须暂停才能刷新；右键 → Refresh |

---

### 8.2 Renode Monitor 命令

```python
# 外设信息（SVD 加载后可用）
sysbus.gpioa                    # GPIOA 方法列表
sysbus.usart2                   # USART2 方法列表
sysbus.usart2 BaudRate          # 读属性

# 外设寄存器（通过偏移量）
sysbus.usart2 ReadDoubleWord 0x0    # SR 状态寄存器
sysbus.usart2 ReadDoubleWord 0x4    # DR 数据寄存器
sysbus.usart2 ReadDoubleWord 0xC    # CR1 控制寄存器
sysbus.gpioa ReadDoubleWord 0x0     # MODER
sysbus.gpioa ReadDoubleWord 0x14    # ODR

# 任意地址
sysbus ReadDoubleWord 0x40004400    # USART2 基址
sysbus ReadDoubleWord 0x20000000    # SRAM 起始

# 所有 CPU 寄存器
cpu
```

### 8.3 VS Code Debug Console（GDB 命令）

```
-exec x/1xw 0x40004400          # 读 USART2 基址 1 个字
-exec x/10xw 0x20000000         # dump SRAM 10 个字
-exec info registers            # 查看 R0-R15, PC, SP, CPSR
-exec p/x $sp                   # 栈指针
-exec p/x $pc                   # 程序计数器
-exec set $usart2 = *(uint32_t*)0x40004400   # 存到 GDB 变量
-exec p/x $usart2               # 打印
```

### 8.4 VS Code WATCH 窗口

**已启用** `set mem inaccessible-by-default off` 后，尝试这些写法：
```
{int}0x40004400
(int[1])0x40004400
```

如果 WATCH 不认（不同 MI 引擎行为不同），换 Debug Console 的 GDB 命令。

---

## 9. 常见问题与解决

### ❌ CMake: `is not able to compile a simple test program`

**现象：** CMake 配置失败，链接器报 `unrecognized option '--major-image-version'`

**根因：** CMake Tools 的 kit 命令行传了编译器，但 toolchain 文件没 `FORCE` 覆盖，导致 CMake 用 ARM 编译器尝试链接 Windows exe。

**解决：** toolchain 文件全部用 `CACHE STRING "" FORCE`：
```cmake
set(CMAKE_C_COMPILER    "..." CACHE STRING "C compiler"   FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "" FORCE)
set(CMAKE_C_COMPILER_FORCED TRUE CACHE BOOL "" FORCE)
```
然后删除 `build/` 目录重新配置。

---

### ❌ Renode: `Could not tokenize here: sysbus LoadELF ...`

**现象：** Renode 无法解析 ELF 路径

**根因：** 路径格式问题。Renode 的 `@` 路径相对脚本目录，`@../build/` 语法不被 `sysbus LoadELF` 支持。

**解决：** 用绝对路径 + 引号：
```python
sysbus LoadELF "D:/full/path/to/build/firmware.elf"
```
注意使用正斜杠 `/` 而非反斜杠。

---

### ❌ Renode: `sysbus does not provide a field, method or property LoadSVD`

**现象：** `sysbus LoadSVD` 命令不存在

**根因：** Renode v1.16 不支持通过 Monitor 命令加载外部 SVD。SVD 只能通过 `.repl` 平台文件加载。

**解决（推荐）：** 使用 Cortex-Debug 外设视图 — 在 VSCode 内直接图形化浏览所有外设寄存器，比 Renode Monitor 方便得多。参见 [8.1 Cortex-Debug 外设视图](#81--推荐cortex-debug-外设视图)。

**解决（备选）：** 依赖 `.repl` 平台文件自带的 SVD（如 STM32F40x），在 Renode Monitor 中用 `sysbus.ReadDoubleWord` + 偏移量查看。

---

### ❌ VS Code: F5 后单步按钮灰色，程序不停在入口

**现象：** F10/F11 灰色无法点击，CPU 在跑

**根因：** `platform.resc` 里写了 `start`，CPU 在 GDB 连上之前就跑飞了。

**解决：** 删掉 `platform.resc` 里的 `start`。让 GDB 完全控制 CPU：
```python
# 不要写 start
# 由 GDB 控制 CPU 启停
```

---

### ❌ VS Code: Problems 面板被 Renode 输出刷屏

**现象：** Renode 每行输出都显示为 VS Code 错误

**根因：** `tasks.json` 的 problemMatcher 正则是 `^(.*)$`，匹配所有行。

**解决：** 改成只匹配 `[ERROR]`：
```json
"regexp": "^.*\\[ERROR\\].*$"
```

---

### ❌ VS Code: GDB 报路径乱码 `D:\lXxx\\Progectll...`

**现象：** `file` 命令路径被 JSON 转义破坏

**根因：** `setupCommands` 里的 `"file ${workspaceFolder}/..."` 被 `${workspaceFolder}` 展开后，反斜杠被 JSON 二次转义。

**解决：** 删掉 `file` 命令 — `launch.json` 的 `"program"` 字段已经指定了 ELF。

---

### ❌ GDB 连不上 / 连接被拒绝

**现象：** `Connection refused` 或超时

**检查清单：**
1. Renode 是否先启动了？（`preLaunchTask` 配置了 吗）
2. 端口 3333 是否被上次的 Renode 占用？→ `netstat -ano | findstr 3333` 查并手动杀
3. GDB Server 是否在 ELF 加载之前启动？（顺序问题）

---

### ❌ WATCH 窗口 `*(uint32_t*)0x40004400` 不工作

**解决：**
- 添加 `"set mem inaccessible-by-default off"` 到 setupCommands
- 尝试 `{int}0x40004400` 语法
- 或在 Debug Console 用 `-exec x/1xw 0x40004400`

---

## 10. 快速参考卡

### 调试器选择

| 场景 | 用哪个 |
|------|--------|
| 想看外设寄存器（GPIO/USART/TIM…） | ✅ **Cortex-Debug**（外设视图） |
| 只想简单断点调试 | cppdbg 够用 |

### Cortex-Debug 快速排错

| 问题 | 检查 |
|------|------|
| 外设视图不显示 | `svdFile` 路径对不对？文件存在吗？ |
| 寄存器值全 0 | 调试暂停了吗？RCC 时钟使能了吗？ |
| 连接不上 | Renode 启动了吗？端口 3333 被占了吗？ |

### STM32F4 常用基址

| 外设 | 基址 | 
|------|------|
| USART2 | `0x40004400` |
| USART1 | `0x40011000` |
| GPIOA | `0x40020000` |
| GPIOB | `0x40020400` |
| GPIOC | `0x40020800` |
| RCC | `0x40023800` |
| NVIC_STIR | `0xE000EF00` |
| SYSTICK | `0xE000E010` |
| FLASH | `0x08000000` |
| SRAM | `0x20000000` |

### Cortex-M 通用寄存器（GDB）

| 寄存器 | GDB 名 |
|--------|--------|
| 栈指针 | `$sp` (MSP) |
| 程序计数器 | `$pc` |
| 链接寄存器 | `$lr` |
| 通用寄存器 | `$r0` ~ `$r12` |
| 程序状态 | `$cpsr` |
| 主栈指针 | `$msp` |
| 进程栈指针 | `$psp` |

### 快捷键

| 操作 | 快捷键 |
|------|--------|
| 构建 | `Ctrl+Shift+B` |
| 开始调试 | `F5` |
| 继续执行 | `F5` |
| 单步跳过 | `F10` |
| 单步进入 | `F11` |
| 跳出函数 | `Shift+F11` |
| 停止调试 | `Shift+F5` |
| 重启调试 | `Ctrl+Shift+F5` |
| 切换断点 | `F9` |

### 启动顺序总览

```
┌─────────────┐     ┌──────────────┐     ┌──────────────┐
│ 1. CMake    │ ──▶ │ 2. Renode    │ ──▶ │ 3. GDB       │
│    Build    │     │    启动模拟   │     │    连接调试   │
│  firmware.elf│     │  GDB :3333   │     │  F5 启动      │
│             │     │  加载ELF     │     │  F10/F11 单步 │
│             │     │  CPU 停住    │     │  F5 继续      │
└─────────────┘     └──────────────┘     └──────────────┘
       ↑                                      │
       │            ┌──────────────┐           │
       └────────────│ 4. 改代码    │◀──────────┘
                    │    Ctrl+B    │
                    └──────────────┘
```

---

## 附录：换其他芯片的 Checklist

将这个项目复制到新芯片时，按顺序修改：

1. **`cmake/toolchain-*.cmake`** — 改编译器路径、CPU 架构
2. **`CMakeLists.txt`** — 改 `-mcpu=cortex-m?`、链接脚本、启动文件
3. **`renode/platform.resc`** — 改平台 `.repl` 文件、ELF 绝对路径、串口号
4. **`.vscode/launch.json`** — 改 ELF 路径、`armToolchainPath`、**`svdFile`（替换为对应芯片的 SVD）**
5. **`.vscode/settings.json`** — 改 toolchain 文件路径
6. **`.vscode/tasks.json`** — 改 Renode 任务里的 `cwd`
7. **获取新芯片的 SVD** — 从 `${STM32CubeCLT}/STMicroelectronics_CMSIS_SVD/` 或社区仓库找
