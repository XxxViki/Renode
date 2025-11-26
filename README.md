# RtosOfArm layout

This repository now separates the kernel, port, and application so roles are clearer:

- `rtos/` — RTOS kernel submodule (core source and config headers).
- `ports/arm-cortex-m/` — Cortex-M port glue (context switch, SysTick setup, scheduler entry).
- `apps/blinky/` — Demo application entry (`main.c`) plus libc stubs.
- `cmsis/`, `drivers/` — Vendor submodules (startup/system files and device headers).
- `build/` — Out-of-source CMake build directory.

Build example:
```
cmake -S . -B build
cmake --build build
```

Notes:
- Kernel config remains in `rtos/config` (submodule) for now; if you want per-board overrides, mirror those headers into a project-local `config/<board>/` folder and point includes to it.
- Ports and apps are project-owned code; add more apps under `apps/` and new MCU ports under `ports/`.
