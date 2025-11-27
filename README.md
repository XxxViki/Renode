# RtosOfArm

一个最小可运行的 Cortex-M4 RTOS 学习工程，包含内核子模块、移植层和示例应用，方便在 GitHub 上直接浏览；默认使用 Renode 仿真，无需真机硬件。

## 目录结构
- `rtos/`：RTOS 内核子模块（任务、链表、配置）。
- `ports/arm-cortex-m/`：Cortex-M 端口与上下文切换、SysTick 配置。
- `apps/blinky/`：示例应用 `main.c` 与基本外设 stub。
- `cmsis/`、`drivers/`：芯片启动文件与寄存器头。
- `build/`：CMake 构建输出（建议保持空目录，由 CMake 生成）。
- `renode/`：Renode 仿真脚本与 GDB 端口设置。

## 快速构建
```sh
cmake -S . -B build
cmake --build build
```

## 调试/仿真提示
- 推荐使用 Renode 直接仿真：在 `renode` 目录运行脚本即可启动虚拟板卡，避免实体硬件依赖。
- Renode GDB 默认端口 `3333`（见 `renode` 配置），VSCode/IDE 请保持一致。
- SysTick 启动位使用 bit0，PendSV/SysTick 优先级需设为最低，便于稳定触发任务切换。
- 链表实现使用尾哨兵，`pvOwner` 指向实际 TCB，长度通过 `uxNumberOfItems` O(1) 维护。

## 贡献与扩展
- 新应用：在 `apps/` 下新增目录并提供 `main.c`。
- 新移植：在 `ports/` 下为不同内核或 MCU 增加端口实现。
- 如需自定义配置，可在本地添加 `config/<board>/` 并调整包含路径覆盖子模块默认配置。
