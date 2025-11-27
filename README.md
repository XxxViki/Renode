# RtosOfArm

一个最小可运行的 Cortex-M4 RTOS 学习工程，包含内核子模块、移植层和示例应用；默认使用 Renode 仿真，无需真机硬件。

## 目录结构
- `rtos/`：RTOS 内核子模块（任务、链表、配置）。
- `ports/arm-cortex-m/`：Cortex-M 端口与上下文切换、SysTick 配置。
- `apps/blinky/`：示例应用 `main.c` 与基本外设 stub。
- `cmsis/`、`drivers/`：芯片启动文件与寄存器头。
- `build/`：CMake 构建输出（建议保持空目录，由 CMake 生成）。
- `renode/`：Renode 仿真脚本与 GDB 端口设置。
- `.vscode/`：VSCode 任务与调试配置（tasks.json、launch.json）。
- `docs/`：调试指南与流程图。

## 快速构建
```sh
cmake -S . -B build
cmake --build build
```

## 调试/仿真提示
- 推荐使用 Renode：在 `renode` 目录运行脚本即可启动虚拟板卡，避免实体硬件依赖。
- Renode GDB 默认端口 `3333`（见 `renode/platform.resc`），VSCode/IDE 请保持一致。
- SysTick 启动位使用 bit0，PendSV/SysTick 优先级需设为最低，便于稳定触发任务切换。
- 链表实现使用尾哨兵，`pvOwner` 指向实际 TCB，长度通过 `uxNumberOfItems` O(1) 维护。
- 详见 `docs/DEBUG.md`，并可在 GitHub 直接查看 `docs/images/debug-flow.svg` 流程图。

## VSCode 集成
- `.vscode/tasks.json`：构建 + 推送 ELF/RESC + 远端启动 Renode 的任务入口（需可 SSH/scp 到远端）。
- `.vscode/launch.json`：预设 GDB 远程调试，指向 Renode GDB 端口 `3333`，F5 即可连接。

## 许可证
- 本仓库采用 MIT License（见 `LICENSE`）。子模块依其各自许可证使用。

## 免责声明
- 示例中的远端地址/路径（如 `192.168.31.135:/home/orangepi/object/renode_portable`）仅为演示，请在公开前替换为占位符或自定义变量，避免暴露内网信息或凭据。
