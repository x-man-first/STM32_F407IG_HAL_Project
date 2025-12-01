# 开发者指南（DEV_GUIDE）

此文档面向开发人员，概述如何在本项目中进行常见开发任务：构建与烧录、仿真/真实传感器切换、以及基础调试流程。

---

## 1. 环境与构建（Build）

- 推荐使用 VS Code + EIDE（工程已配置 EIDE 任务）。
- VS Code Tasks（在任务列表中可见）：

  - `build`：仅构建工程。对应命令 `${command:eide.project.build}`。
  - `flash`：将固件烧写到设备（对应 `${command:eide.project.uploadToDevice}`）。
  - `build and flash`：构建并烧写。
  - `clean`, `rebuild`：清理/重建工程。
- 命令行（JLink）烧录示例（Windows）：

```
JLinkExe -CommandScript build/Debug/commands.jlink
```

- Toolchain：`arm-none-eabi`（路径见 `build/Debug/builder.params`）。
- 直接调试与编译错误：查看 `build/Debug/unify_builder.log` 获取完整编译/链接输出；`build/Debug/STM32_F407IG_HAL_Project.map` 和 `.objlist` 可用于跟踪符号与对象文件。

---

## 2. 关键文件与模块关系

- 程序入口：`Core/Src/main.c`。
- 定时器与调度：
  - `BSW/TIM2`：系统调度（设置 `Flag_1ms/10ms/20ms/50ms/100ms/500ms`），在 `TIM2_Handler_IN_IRQ` 中更新。
  - `BSW/TIM_IRQ/TIM_IRQ.c`：将 TIM 中断路由到对应 handler（TIM2 -> `TIM2_Handler_IN_IRQ`，TIM1 -> `Motor_Handler_IN_TIM1IRQ`）。
- 状态机：`ASW/ModeStateMechanic` 管理系统运行模式与模块生命周期（`MM_InitDeinit` / `MM_InitSts` / `MM_InitReq`）。
- 电机控制：`ASW/MotorControl`（6-step 在 `MotorControl.c`）。
- 外设抽象：`BSW/*`（`PWM`, `GPIO`, `CAN`, `TIM` 等）。
- 硬件驱动：`CDD/`（例如 Hall Sensor、KEY）。

---

## 3. 灯塔说明（常见开发 / 测试场景）

### 3.1 构建失败 & 链接报错快速排查

- 错误：`undefined reference to <symbol>`：

  1. 检查实现是否在 `build/Debug/builder.params` 的 `sourceList` 中。
  2. 检查头文件是否声明为 `extern`，且函数签名与实现一致（例如 `CAN1TX()`）。
  3. 检查实现是否被声明为 `static`（若是，无法被外部引用）。
- 错误：符号类型冲突（例如 `Flag_100ms`）：确认所有地方的声明类型一致（项目标准为 `uint8_t`，位于 `BSW/TIM2/include/TIM2.h`）。
- 使用 `build/Debug/unify_builder.log` 查找编译器/链接器日志以获得更多上下文。

### 3.2 烧写与验证

- 使用 `build/Debug/commands.jlink` 配合 JLink 工具进行批量烧录与执行。
- 烧写完成后可在 `main` 的 `Flag_1ms/10ms` 等任务中插入 `Debug_Send...` 或 `HAL_GPIO_TogglePin()` 来验证运行。若没有输出，检查 `ASW/Debug` 的输出实现。

---

## 4. 切换仿真（模拟）传感器到真实传感器

项目现在默认在 `ASW/MotorControl/src/MotorControl.c` 的 `MotorControl_ReadSensors()` 使用模拟数据（例如三相正弦模拟电流、模拟角度）。要切换到真实硬件测量：

1.使用传感器接口：

- 霍尔传感器：`CDD/Hall.c` 中的 API；

2.若需要在工程中增加库文件，请更新 `build/Debug/builder.params` 的 `incDirs` 与 `sourceList`（`builder.params` 是构建的权威配置）。

3.在 `ModeStateMechanic_InitTask` 中注册驱动的 Init/DeInit 回调（若需要生命周期控制）。

注意：在首次上电或调试硬件时，保留安全阈值，并使用低占空比测试按钮或控制命令来验证输出。若使用霍尔验证角度，先检查传感器接线和清零/校准流程。

---

## 5. 基础调试方法

### 5.1 实时中断调试

- TIM1（PWM）/TIM2（调度）为关键时序路径。`BSW/TIM_IRQ/TIM_IRQ.c` 将 TIM1/2 的回调映射为 `Motor_Handler_IN_TIM1IRQ` / `TIM2_Handler_IN_IRQ`。
- 在 ISR 中避免打印、malloc、阻塞 I/O 或复杂运算。仅做最小状态计算，必要时设置标志或将任务推到 TIM2 的 1ms/10ms 层执行。
- 若需要可视化触发，临时在 IRQ 中添加 `HAL_GPIO_TogglePin()`（并用示波器/逻辑分析器观察）来确认中断是否到达以及持续时间。

---

## 6. 安全注意事项

- 修改 PWM 引脚、占空比或 MOSFET 使能之前，务必验证所有低侧/高侧连接正确、无短路。仅在保护良好且占空比低时进行第一次上电测试。
- 在体验电机之前，断开或限制负载（如无负载测评）以防出现机械故障。
- 修改与引导/启动流程相关的代码时（`startup`、链接脚本、时钟配置），建议做小步提交并保留原始版本以便快速回滚。

---

## 7. 快速问题解决清单

- 无法链接 `CAN1TX`：确保 `BSW/CAN/src/Can.c` 在 `sourceList`，且 `Can.h` 中声明 `extern void CAN1TX(void);`。
- `Flag_*` 报错类型不一致：统一使用 `uint8_t`（见 `BSW/TIM2/include/TIM2.h`），不要在其它文件声明为 `boolean` 或 `int`。
- 中断不触发：检查 `Core/Startup/startup_stm32f407igtx.s`、`stm32f4xx_it.c` 的 IRQ 向量及 `HAL_NVIC_SetPriority`/`HAL_NVIC_EnableIRQ` 的调用点。

---
