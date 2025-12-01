# STM32_F407IG_HAL_Project

这是一个基于 STM32F407 HAL 的 BLDC（无刷直流电机）控制示例工程，包含分层架构（ASW/BSW/CDD/Core）、CAN 控制、TIM/PWM 驱动与霍尔传感器换相。该工程适用于入门、定制以及在 STM32F407IGTX 硬件上快速验证控制逻辑。

## 主要特点
- 基于 STM32 HAL 的分层架构（ASW、BSW、CDD、Core）
- MotorControl 包含 6-step 霍尔换相逻辑
- CAN 控制（`CANID_VCU_CMD`）控制启动/停止/方向/占空比
- ModeStateMechanic 管理模块生命周期（Init/DeInit）
- 基本调试可视化：100ms 心跳 (CAN) 与 500ms LED toggle

## 目录（关键位置）
- `Core/Src/main.c` - 程序入口、main 循环
- `ASW/ModeStateMechanic` - 模式与模块管理、生命周期控制
- `ASW/MotorControl` - 电机控制主逻辑、TIM1 IRQ 换相逻辑
- `BSW/CAN` - CAN 初始化、过滤器、接收/发送、解析
- `BSW/PWM` - PWM 抽象（TIM1）
- `BSW/TIM2` - 系统调度器（1ms tick Flags）
- `CDD/Hall` - 霍尔传感器驱动
- `CDD/KEY` - 本地按键
- `docs/` - 文档（设计、架构、接口）

## 快速开始（在 VSCode 中）
工程已提供用于构建/刷写的任务（VSCode Task）：

- build: 使用工程默认构建工具链编译
- flash: 将构建产物上传到目标设备
- build and flash: 构建并刷写到设备
- rebuild / clean: 清理/重建项目

要使用任务菜单运行（建议）：

1. 打开 VSCode 命令面板 -> 运行 Task -> 选择 `build` 或 `flash` 等。
2. 若需要命令行刷写（JLink）：

```bash
# 如果电脑上安装了 JLinkExe
JLinkExe -CommandScript build/Debug/commands.jlink
```

## 运行与测试
1. 将固件刷写进目标板
2. 观察 500ms LED 翻转确认主循环运行
3. 通过 CAN 向主机发送 VCU 控制帧（`CANID_VCU_CMD`）测试启动/停止/方向/占空比；CAN 数据解析规则在 `BSW/CAN/src/Can.c` 中
4. 如需仿真霍尔或者做 bench 测试，可以在 `Motor_Handler_IN_TIM1IRQ` 中暂时插入 `HAL_GPIO_TogglePin()` 或调试输出

## CAN 协议概要
- `CANID_VCU_CMD` = 0x000（控制）
  - Data[0] bit0: start_stop（1：启动，0：停止）
  - Data[0] bit1: CW_CCW（0：正转，1：反转）
  - Data[0] bits [8..23]：pwm_duty（16-bit）

- `CANID_SYNC` = 0x001：心跳/同步帧（由 `CAN1TX()` 定期发送）

## 如何添加新模块
1. 在 `ASW` 或 `BSW` 中新增模块代码，按照工程约定把 header 放在 `include/`、实现放在 `src/`。
2. 若需要模块随系统启动管理生命周期：在 `ASW/ModeStateMechanic/include/ModeStateMechanic_InitTask.h` 的 `MM_InitStsIndexes` 添加枚举索引，并在 `ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c` 的 `MM_InitCfg` 中加入 `InitStart/InitPending/DeinitStart/DeInitPending` 回调。
3. 更新 `build/Debug/builder.params` 中 `incDirs` 与 `sourceList`（或使用 EIDE GUI添加）以包含新文件。

## 文档
- `docs/设计文档.md`：设计目标、数据流与测试点。
- `docs/架构文档.md`：系统架构、模块职责、架构图。
- `docs/接口描述文档.md`：模块公共接口与数据结构说明。

## 建议改进（可选）
- 使用 Doxygen 自动生成 API 文档并与 CI/CD 集成
- 使用 mermaid-cli 生成 PNG/SVG 架构图用于 README 或 Wiki
- 增加单元测试、硬件抽象层 Mock 以便于本地 CI 测试

## 贡献 & 许可
- 贡献请先参阅 `docs/DEV_GUIDE.md` 中的开发规范
- 仓库未声明许可时请遵循公司的开源/合规政策或联系仓库负责人

---

有需要我可以把 `docs/架构文档.md` 中的 mermaid 图渲染为 PNG 并把图片放到 `docs/diagram` 文件夹里，方便在 GitHub 或其他平台展示。是否需要我继续按此方式生成图像并更新 README？
