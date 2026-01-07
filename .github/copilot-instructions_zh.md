## STM32_F407IG_HAL_Project — Copilot / AI 代理指导文档

**目的**: 本文档通过概述项目架构、构建/调试工作流程、编码约定以及常见的扩展/集成点，帮助 AI 代理快速生成高价值的编辑。

### 1) 宏观架构

- **入口点**: `Core/Src/main.c` — 系统初始化和周期性的主循环。主循环由定时器事件驱动（标志位在 `BSW/TIM2` 中设置）。
- **分层**:
  - `ASW/` (Application SW / 应用软件) — 模式逻辑 (`ModeStateMechanic`)、电机控制。
  - `BSW/` (Basic SW / 基础软件 / HAL 封装层) — 硬件抽象，例如 `PWM`, `GPIO`, `CAN`, `TIM`。
  - `CDD/` (Complex Device Drivers / 复杂设备驱动) — 特定于设备的封装，例如霍尔传感器和按键输入。
  - `Drivers/` — STM32 HAL 和 CMSIS 厂商驱动。
  - `Core/` — `main` 函数、系统初始化和中断处理程序。

### 2) 主要运行时流程

- `MX_TIM2_Init()` (在 `BSW/TIM2` 中定义) 启动周期性中断。标志位 (`Flag_1ms`, `Flag_10ms` 等) 由 TIM2 设置，并由 `Core/Src/main.c` 读取。
- `ModeStateMechanic` (在 `ASW/ModeStateMechanic` 中) 通过 `MM_InitDeinit` 和 `MM_InitCfg[]` 管理功能模块的生命周期（初始化/反初始化）。在此处添加/删除模块以控制其生命周期。
  - **重要**: 生命周期管理使用 `MM_InitReq`, `MM_InitSts`, 和 `MM_InitDeinit()` (参见 `ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c`)。这些是全局变量，是生命周期协调的一部分。
- **电机控制**: `ASW/MotorControl` 包含六步换相的辅助函数（扇区/状态和 GPIO 翻转）。
- **实时性映射**: `TIM1` 用于电机 PWM/换相时序，其 IRQ 通过 `BSW/TIM_IRQ/TIM_IRQ.c` 路由以调用 `Motor_Handler_IN_TIM1IRQ` (在 `ASW/MotorControl` 中声明/定义)。`TIM2` 是系统调度器，用于翻转 `Flag_1ms/10ms/...` 等标志位。

### 3) 集成点与数据流

- **CAN**: `BSW/CAN` 与硬件交互；传入的消息会填充在 `Can.c` 中的 `Bldc_Control_Req` 结构体。`ModeStateMechanic`/`MotorControl` 消费这些请求。
- **PWM**: `BSW/PWM` 实现了 PWM 抽象；`MotorControl` 通过 `PWM_SetDutyCycle()` 设置占空比。

**关于共享全局变量和消息的说明**:

- `Bldc_Control_Req` (在 `ASW/MotorControl/include/MotorControl.h` 中声明) 由 `BSW/CAN` 写入，并由 `ASW/MotorControl` 读取。确保此处的任何更改都保持 ABI（应用程序二进制接口）的稳定性。
- `bldc_status` 是一个全局变量，用于 MotorControl, FSM (有限状态机) 和 CAN 之间报告运行状态。对于新模块，优先使用访问器函数（accessor functions）。

### 4) 构建与烧录工作流 (如何在本地运行)

- **主要任务** (使用 VSCode 任务或 EIDE):
  - **构建 (Build)**: 运行标记为 `build` 的工作区任务 (这将触发 `${command:eide.project.build}`).
  - **烧录 (Flash)**: 运行 `flash` 任务 (`${command:eide.project.uploadToDevice}`).
  - **构建并烧录 (Build & Flash)**: 运行 `build and flash` 任务。
  - **清理/重新构建 (Clean / Rebuild)**: `clean` 和 `rebuild` 任务可用。
- **CLI (命令行) 说明**:
  - **工具链**: `arm-none-eabi`，其路径可在 `build/Debug/builder.params` 中找到 (已包含 Windows 工具链路径)。
  - **JLink 烧录脚本**: `build/Debug/commands.jlink`。示例 (如果 JLink 工具存在):
    - `JLinkExe -CommandScript build/Debug/commands.jlink` (Windows 兼容路径)。

**调试技巧 (构建/链接/时序问题)**:

- 如果您看到链接错误 (undefined references)，请首先确认:
  1) 实现文件 (`.c`) 已在 `build/Debug/builder.params` -> `sourceList` 中列出。
  2) 头文件 (`.h`) 使用 `extern` 声明了该符号，并且与实现签名匹配 (例如, `CAN1TX(void)` 在 `BSW/CAN/include/Can.h` 中声明)。
  3) 该函数不是 `static` 的，并且具有外部链接属性。
- 如果您看到类型不匹配的编译器错误 (例如, `Flag_500ms` 类型冲突)，请搜索多个使用了不同类型的声明 (`uint8_t`, `int`, `boolean`)。请统一为 `BSW/TIM2/include/TIM2.h` 中使用的 `uint8_t`。
- 使用 `build/Debug/unify_builder.log` 来查找确切的编译器/链接器诊断信息。`*.map` 和 `*.objlist` 文件有助于跟踪符号地址和目标文件的贡献。

### 5) 指导编辑的重要项目文件 (经常引用)

- **初始化与调度**: `Core/Src/main.c`, `BSW/TIM2/src/TIM2.c`。
- **生命周期与协调**: `ASW/ModeStateMechanic/*` (特别是 `ModeStateMechanic_InitTask.c`)。
  - **注意**: 初始化任务使用 `MM_InitSts[]` 数组来跟踪模块 (参见 `ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c`)。在注册新模块时，向 `MM_InitCfg` 添加条目。
- **电机控制**: `ASW/MotorControl/src/MotorControl.c`。
- **硬件封装与外设**: `BSW/CAN`, `BSW/PWM`, `BSW/GPIO`。
- **HAL 与启动**: `Drivers/*`, `Core/Startup/startup_stm32f407igtx.s`, `STM32F407IGTX_FLASH.ld` 链接器脚本。

### 6) 约定与模式 (项目特定)

- **目录布局**: `ASW` (应用), `BSW` (板级 & HAL 封装), `CDD` (设备驱动), `Core` (系统)。将业务逻辑保留在应用层，硬件调用保留在 BSW 层。
- **命名**: 模块文件名为 `ModuleName.c`/`.h`，公共 API 在每个模块下的 `include` 头文件中声明。将头文件放在 `include/` 中，源文件放在 `src/` 中。
- **初始化/反初始化 (Init/Deinit)**: 由 `MM_InitDeinit` 集中管理。任何新的功能模块都应实现 `start/pending/deinit` 函数，并在 `MM_InitCfg` 中注册。
- **标志位与定时器位**: 定时器滴答标志是 `BSW/TIM2` 中的 `uint8_t Flag_*`。使用 `TIM2_Handler_IN_IRQ` 来避免在 ISR (中断服务程序) 中直接执行复杂逻辑。
- **宏驱动的功能**: 可选功能在 `MotorControl_Cfg.h` 中进行切换 (例如, `USE_CAN_CONTROL`, `USE_KEY_CONTROL`)。请遵守这些宏以保持构建的一致性。

**风格与安全规则**:

- 将业务逻辑保留在 `ASW` 中，将硬件调用保留在 `BSW` 中。如果需要更深层次的硬件访问，优先选择定义良好的 BSW API。
- 避免在 ISR 中执行阻塞或重度操作 — 计算最小的控制状态并返回；将繁重的工作调度到 1ms/10ms 的任务中。
- 在新代码中**保留中文注释**以匹配项目风格。

### 7) 常见编辑与示例

- **添加一个新的传感器驱动**:

  1) 将硬件相关代码放在 `BSW` 中。
  2) 在 `builder.params` -> `incDirs` 中添加 `include` 路径，并在 `sourceList` 中添加 `src` 文件条目。
  3) 如果它们需要生命周期管理，请在 `ModeStateMechanic_InitTask` 中注册传感器的 `init/stop` 函数。

**常见调试流程**:

- **诊断缺失/不正确的 extern 或链接时问题**: 检查头文件中的 `extern` 声明 -> 源文件中的实现 -> `builder.params` 中的 `sourceList`。
- **诊断类型冲突 (例如 `Flag_*`)**: 搜索 `extern Flag_` 声明，并确保所有文件都使用 `uint8_t` — 如果发现其他声明，请修复它们。
- **调试错误的 ISR 路由或时序**: 在适当的 IRQ 处理程序中添加一个简单的 `HAL_GPIO_TogglePin()`，并用示波器/LED 进行观察。

### 9) 安全 / 硬件注意事项

- **SHUTDOWN 宏** (例如 `SHUTDOWN_EN`) 用于断言驱动器门控。使用这些来安全地启用/禁用功率级。
- 在启用输出之前，必须确保电机 GPIO 和 PWM 配置正确。

### 10) 如何添加新模块或功能 (简短清单)

- 1) 将 `.c`/.h 文件添加到适当的层 (BSW/ASW/CDD)。
- 2) 更新 `build/Debug/builder.params` -> `incDirs` 和 `sourceList`，或使用 EIDE 项目 GUI 添加文件。
  - **提示**: `builder.params` 是权威配置。将头文件位置添加到 `incDirs`，并将 `.c` 文件添加到 `sourceList`。
- 3) 在需要时，在 `ModeStateMechanic_InitTask` 中注册初始化/反初始化函数。
- 4) 在 `Tests/` 目录中添加单元测试框架 (如果不存在则创建一个)，并根据需要添加模拟/Mock ADC。
- 5) 使用 `build` 任务 + `flash` 在硬件上进行验证。

---

💡 **给代理的提示**: **保留中文注释**并维护现有的 API 约定 (函数名/签名)。对于任何跨领域的更改 (例如，更新 `startup` 或链接器脚本)，首先提出一个最小化的更改方案，并提供 PR 级别的测试和在硬件上手动验证的步骤。

如果本文档的任何部分不清楚，或者您希望涵盖某些缺失的领域 (例如，单元测试风格、CI 细节或更多示例)，请告诉我需要扩展哪些部分 — 我会相应地更新这些说明。

---

⚠️ **已知的构建陷阱** (来自 `build/Debug/unify_builder.log` 中的日志):

- **声明冲突**: `Flag_100ms`/`Flag_500ms` 在某些文件中可能被声明为 `boolean` 或 `int`；`BSW/TIM2/include/TIM2.h` 使用的是 `uint8_t` — 请保持一致。
- **链接时符号未定义**: `CAN1TX()` 之前出现过此问题 — 请验证函数原型、源文件列表以及 `static` vs 全局作用域。
