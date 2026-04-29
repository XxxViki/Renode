# VSCode 调试配置指南

## 调试流程（推荐）

### 步骤 1: 启动 Renode
在 VSCode 终端中运行：
```bash
renode -e "include @renode/platform.resc"
```

等待看到以下消息：
```
stm32f412: Machine started.
```

### 步骤 2: 开始调试
在 VSCode 中按 **F5** 键，或点击调试面板的绿色播放按钮。

### 步骤 3: 调试操作
- **F5**: 继续执行
- **F9**: 设置/取消断点
- **F10**: 单步跳过
- **F11**: 单步进入
- **Shift+F11**: 单步跳出
- **Shift+F5**: 停止调试

## 使用任务（可选）

VSCode 提供了一些有用的任务：

### 可用任务
- `CMake Configure`: 配置 CMake 项目
- `CMake Build`: 构建项目
- `Build All`: 完整构建流程
- `Start Renode`: 启动 Renode 模拟器（后台运行）
- `Stop Renode`: 停止 Renode 模拟器
- `Clean Build`: 清理并重新构建

### 使用任务
1. `Ctrl+Shift+P` 打开命令面板
2. 输入 "Tasks: Run Task"
3. 选择所需任务

## 调试配置详情

### launch.json 配置
```json
{
    "name": "Debug STM32F412 on Renode",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/firmware.elf",
    "cwd": "${workspaceFolder}",
    "miDebuggerPath": "C:\\ST\\STM32CubeCLT_1.18.0\\GNU-tools-for-STM32\\bin\\arm-none-eabi-gdb.exe",
    "miDebuggerServerAddress": "localhost:3333",
    "stopAtEntry": false,
    "setupCommands": [
        { "text": "set pagination off" },
        { "text": "set confirm off" },
        { "text": "set print pretty on" }
    ],
    "externalConsole": false,
    "MIMode": "gdb",
    "targetArchitecture": "arm"
}
```

## 故障排除

### 问题 1: F5 无法连接到 GDB
**解决方案**:
1. 确保 Renode 正在运行
2. 检查是否看到 "Machine started" 消息
3. 如果 Renode 未运行，在终端中启动：`renode -e "include @renode/platform.resc"`

### 问题 2: Renode 启动失败
**解决方案**:
1. 检查 `renode/platform.resc` 文件是否存在
2. 检查 `build/firmware.elf` 文件是否存在
3. 测试 Renode 安装：`renode --version`

### 问题 3: GDB 连接超时
**解决方案**:
1. 确保 Renode 完全启动（等待 "Machine started"）
2. 检查端口 3333 是否被占用
3. 重启 Renode

### 问题 4: 断点不生效
**解决方案**:
1. 确保使用 Debug 构建配置
2. 检查优化级别（Debug 使用 -O0）
3. 尝试在 main 函数入口处设置断点

## 停止 Renode

调试完成后，在 Renode 终端中按 `Ctrl+C`，然后输入：
```
quit
```

或者使用 VSCode 任务：
1. `Ctrl+Shift+P` → "Tasks: Run Task" → "Stop Renode"

## 文件位置

- **固件文件**: `build/firmware.elf`
- **Renode 配置**: `renode/platform.resc`
- **VSCode 配置**: `.vscode/launch.json`, `.vscode/tasks.json`
- **CMake 配置**: `cmake/toolchain-arm-none-eabi.cmake`

## 快速开始

1. 构建项目：`Ctrl+Shift+P` → "CMake: Build"
2. 启动 Renode：在终端运行 `renode -e "include @renode/platform.resc"`
3. 开始调试：按 F5

## 调试技巧

### 查看变量
- 将鼠标悬停在变量上
- 在调试面板的 "变量" 部分查看
- 在调试控制台输入：`print variable_name`

### 查看寄存器
- 在调试控制台输入：`info registers`
- 或在调试面板的 "寄存器" 部分查看

### 查看内存
- 在调试控制台输入：`x/10x 0x20000000`（查看 0x2000000 处的 10 个十六进制字）

### 查看调用栈
- 在调试面板的 "调用堆栈" 部分查看
- 或在调试控制台输入：`bt`