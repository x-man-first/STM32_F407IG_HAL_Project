# AGENTS.md — STM32 F407 BLDC Motor Control Project

## Project Overview

STM32F407 HAL-based BLDC (brushless DC) motor control firmware with 6-step hall commutation, CAN control, and layered architecture (ASW/BSW/CDD/Core).

## Build System

- **Toolchain**: ARM GCC (`arm-none-eabi`) via EIDE (Embedded IDE)
- **Build config**: `build/Debug/builder.params` (authoritative source for includes and source files)
- **Linker script**: `STM32F407IGTX_FLASH.ld`

### Commands (VSCode Tasks)

| Task | Command |
|------|---------|
| Build | `${command:eide.project.build}` |
| Flash | `${command:eide.project.uploadToDevice}` |
| Build & Flash | `${command:eide.project.buildAndFlash}` |
| Clean | `${command:eide.project.clean}` |

**CLI flash** (if JLink installed):
```
JLinkExe -CommandScript build/Debug/commands.jlink
```

## Architecture

```
ASW/          Application logic (ModeStateMechanic, MotorControl)
BSW/          Board/HAL wrappers (CAN, PWM, GPIO, TIM2, SystemClock)
CDD/          Complex device drivers (Hall sensor, KEY)
Core/         main.c, startup, interrupts, system init
Drivers/      STM32 HAL and CMSIS vendor code
```

### Key Entry Points

- `Core/Src/main.c` — main loop, reads `Flag_*` from TIM2
- `BSW/TIM2/src/TIM2.c` — system scheduler, sets `Flag_1ms/10ms/100ms/500ms`
- `ASW/ModeStateMechanic/` — module lifecycle management (`MM_InitCfg[]`, `MM_InitDeinit`)
- `ASW/MotorControl/src/MotorControl.c` — 6-step commutation logic
- `BSW/CAN/src/Can.c` — CAN RX/TX, populates `Bldc_Control_Req`
- `BSW/TIM_IRQ/TIM_IRQ.c` — routes TIM1→`Motor_Handler_IN_TIM1IRQ`, TIM2→`TIM2_Handler_IN_IRQ`

### Data Flow

- `Bldc_Control_Req` (written by CAN, read by MotorControl) — keep ABI stable
- `bldc_status` — global status shared across MotorControl/FSM/CAN
- Timer flags: `uint8_t Flag_*` in `BSW/TIM2/include/TIM2.h`

## Adding New Modules

1. Place code in appropriate layer (`ASW`/`BSW`/`CDD`), headers in `include/`, sources in `src/`
2. Update `build/Debug/builder.params`:
   - Add header dir to `incDirs`
   - Add `.c` file to `sourceList`
3. If lifecycle-managed: register in `ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c` (`MM_InitCfg[]`)

## Build Troubleshooting

- **Undefined reference**: Check `sourceList` in `builder.params`, verify `extern` declaration matches signature, ensure not `static`
- **Flag type conflicts**: All `Flag_*` must be `uint8_t` (not `boolean` or `int`)
- **Build logs**: `build/Debug/unify_builder.log`
- **Symbol map**: `build/Debug/STM32_F407IG_HAL_Project.map`

## Conventions

- **Language**: C11, Chinese comments in new code
- **Naming**: `ModuleName.c/.h`, public API in `include/` headers
- **ISR safety**: No blocking/complex ops in ISRs; set flags, defer to 1ms/10ms tasks
- **Feature toggles**: Check `MotorControl_Cfg.h` (`USE_CAN_CONTROL`, `USE_KEY_CONTROL`)
- **Globals**: Prefer accessor functions for new modules; avoid new shared globals

## Safety

- Verify PWM pins, duty cycle, and MOSFET connections before enabling motor outputs
- Use low duty cycle for first power-on tests
- `SHUTDOWN_EN` macros control power stage gates — use for safe disable

## Debug

- Toggle `HAL_GPIO_TogglePin()` in ISR handlers to verify timing with scope/LED
- CAN heartbeat: 100ms sync frame (`CANID_SYNC` = 0x001)
- LED toggle: 500ms in main loop (confirms scheduler running)

## Docs

- `docs/DEV_GUIDE.md` — build/flash/debug workflows
- `docs/设计文档.md` — design goals and data flow
- `docs/架构文档.md` — system architecture
- `docs/接口描述文档.md` — module APIs
