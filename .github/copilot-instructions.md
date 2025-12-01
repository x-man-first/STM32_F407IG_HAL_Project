## STM32_F407IG_HAL_Project — Copilot / AI agent guidance

Purpose: This document helps an AI agent produce high-value edits quickly by outlining the project architecture, build/debug workflows, coding conventions, and common extension/integration touchpoints.

1) Big-picture architecture

- Entry point: `Core/Src/main.c` — system init + periodic main loop. The main loop is event-driven by timers (flags set in `BSW/TIM2`).
- Layers:
  - `ASW/` (Application SW) — Mode logic (`ModeStateMechanic`), Motor Control.
  - `BSW/` (Basic SW / HAL wrappers) — hardware abstractions such as `PWM`, `GPIO`, `CAN`, `TIM`.
  - `CDD/` (Complex Device Drivers) — device-specific wrappers such as Hall sensors and key input.
  - `Drivers/` — STM32 HAL and CMSIS vendor drivers.
  - `Core/` — `main`, system initialization and interrupt handlers.

2) Primary runtime flow

- `MX_TIM2_Init()` (defined in `BSW/TIM2`) starts periodic interrupts. Flags (`Flag_1ms`, `Flag_10ms`, etc.) are set by TIM2 and read by `Core/Src/main.c`.
- `ModeStateMechanic` (in `ASW/ModeStateMechanic`) manages a lifecycle for functional modules (init/deinit) via `MM_InitDeinit` and `MM_InitCfg[]`. Add/remove modules there to control module lifecycle.
  - Important: lifecycle management uses `MM_InitReq`, `MM_InitSts`, and `MM_InitDeinit()` (see `ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c`). These variables are global and part of the lifecycle orchestration.
- Motor control: `ASW/MotorControl` contains 6-step commutation helpers (sector/state and GPIO toggles).
- Real-time mapping: `TIM1` is used for motor PWM/commutation timing and its IRQ is routed by `BSW/TIM_IRQ/TIM_IRQ.c` to call `Motor_Handler_IN_TIM1IRQ` (declared/defined in `ASW/MotorControl`). `TIM2` is the system scheduler that toggles `Flag_1ms/10ms/...`.

3) Integration points & data flow

- CAN: `BSW/CAN` interfaces with hardware; incoming messages populate `Bldc_Control_Req` in `Can.c`. `ModeStateMechanic`/`MotorControl` consumes those requests.
- PWM: `BSW/PWM` implements PWM abstraction; `MotorControl` sets duty cycles via `PWM_SetDutyCycle()`.

Notes on shared globals and messages:

- `Bldc_Control_Req` (declared in `ASW/MotorControl/include/MotorControl.h`) is written by `BSW/CAN` and read by `ASW/MotorControl`. Ensure any change there keeps ABI stable.
- `bldc_status` is a global used across MotorControl, FSM and CAN to report run status. Prefer accessor functions for new modules.

4) Build & flash workflows (how to run locally)

- Primary tasks (use VSCode Tasks or EIDE):
  - Build: run the workspace task labeled `build` (this triggers `${command:eide.project.build}`).
  - Flash: run the `flash` task (`${command:eide.project.uploadToDevice}`).
  - Build & Flash: run `build and flash` task.
  - Clean / Rebuild: `clean` and `rebuild` tasks are available.
- CLI notes:
  - Toolchain: `arm-none-eabi` at path seen in `build/Debug/builder.params` (Windows toolchain path present).
  - JLink flashing script: `build/Debug/commands.jlink`. Example (if JLink tools present):
    - `JLinkExe -CommandScript build/Debug/commands.jlink` (Windows-compatible path).

Debugging tips (build/link/time issues):

- If you see link errors (undefined references), first confirm:
  1) The implementation file is listed in `build/Debug/builder.params` -> `sourceList`.
  2) The header declares the symbol with `extern` and matches the implementation signature (e.g., `CAN1TX(void)` is declared in `BSW/CAN/include/Can.h`).
  3) The function isn't `static` and has external linkage.
- If you see type-mismatch compiler errors (e.g., `Flag_500ms` conflicting types), search for multiple declarations using different types (`uint8_t`, `int`, `boolean`). Standardize on `uint8_t` as used in `BSW/TIM2/include/TIM2.h`.
- Use `build/Debug/unify_builder.log` to find exact compiler/linker diagnostics. `*.map` and `*.objlist` help track symbol addresses and object file contributions.

5) Important project files that guide edits (frequently referenced):

- Initialization & scheduling: `Core/Src/main.c`, `BSW/TIM2/src/TIM2.c`.
- Lifecycle & orchestration: `ASW/ModeStateMechanic/*` (especially `ModeStateMechanic_InitTask.c`).
  - Note: The init task uses a `MM_InitSts[]` array to track modules (see `ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c`). Add entries to `MM_InitCfg` when registering new modules.
- Motor control: `ASW/MotorControl/src/MotorControl.c`.
- Hardware wrapper and peripherals: `BSW/CAN`, `BSW/PWM`, `BSW/GPIO`.
- HAL & startup: `Drivers/*`, `Core/Startup/startup_stm32f407igtx.s`, `STM32F407IGTX_FLASH.ld` linkers.

6) Conventions & patterns (project-specific)

- Directory layout: `ASW` (app), `BSW` (board & HAL wrappers), `CDD` (device drivers), `Core` (system). Keep logic at app-level and hardware calls in BSW.
- Naming: Module filenames are `ModuleName.c`/`.h`, with public API declared in `include` headers under each module. Keep headers in `include/` and sources in `src/`.
- Init/Deinit: Centralized by `MM_InitDeinit`. Any new functional module should implement start/pending/deinit functions and register in `MM_InitCfg`.
- Flags & timer bits: Timer tick flags are `uint8_t Flag_*` in `BSW/TIM2`. Use `TIM2_Handler_IN_IRQ` to avoid doing complex logic directly in ISRs.
- Macro-driven features: Optional features are toggled in `MotorControl_Cfg.h` (e.g., `USE_CAN_CONTROL`, `USE_KEY_CONTROL`). Respect these to keep consistent builds.

Style & safety rules:

- Keep business logic in `ASW` and hardware calls in `BSW`. If you need deeper hardware access, prefer a well-defined BSW API.
- Avoid blocking or heavy operations in ISRs — compute minimal control state and return; schedule heavy work to 1ms/10ms tasks.
- Keep Chinese comments in new code to match project style.

7) Common edits & examples

- Add a new sensor driver:

  1) Put hardware code in `BSW`.
  2) Add `include` to `builder.params` -> `incDirs` and add `src` entry to `sourceList`.
  3) Register sensor init/stop functions in `ModeStateMechanic_InitTask` if they are lifecycle managed.

Common debugging flows:

- To diagnose missing/incorrect externs or link-time issues: check header `extern` declaration -> implementation in source -> `sourceList` in `builder.params`.
- To diagnose conflicting types (e.g., `Flag_*`): search for `extern Flag_` declarations and ensure all files use `uint8_t` — fix other declarations if found.
- To debug wrong ISR routing or timing: add a simple toggle `HAL_GPIO_TogglePin()` in the appropriate IRQ handler and observe with a scope/LED.

9) Safety / hardware precautions

- SHUTDOWN macros (e.g., `SHUTDOWN_EN`) assert driver gates. Use these to safely enable/disable power stages.
- Motor GPIOs and PWM must be correct before enabling outputs.

10) How to add a new module or feature (short checklist)

- 1) Add `.c`/.h to the appropriate layer (BSW/ASW/CDD).
- 2) Update `build/Debug/builder.params` -> `incDirs` and `sourceList` or use the EIDE project GUI to add files.

  - Tip: `builder.params` is authoritative. Add both header location to `incDirs` and `.c` to `sourceList`. 
- 3) Register initialization / deinitialization in `ModeStateMechanic_InitTask` when needed.
- 4) Add unit-test harness in `Tests/` (create one), and add simulation/Mock ADC if required.
- 5) Use `build` task + `flash` to verify on hardware.

---

💡 Tips for agents: Preserve Chinese commenting and maintain existing API contracts (function names/signatures). For any cross-cutting change (e.g., updates to `startup` or linker), propose a minimal change-first approach and provide PR-level tests and steps for manual verification on hardware.

If any part of this doc is unclear or if there are missing areas you want covered (e.g., unit test style, CI details, or more examples), tell me which sections to expand — I’ll update the instructions accordingly.

---

⚠️ Known build pitfalls (from logs found in `build/Debug/unify_builder.log`):

- Conflicting declarations: `Flag_100ms`/`Flag_500ms` may be declared with `boolean` or `int` in some files; `BSW/TIM2/include/TIM2.h` uses `uint8_t` — keep it consistent.
- Undefined symbol during link: `CAN1TX()` occurred previously — verify function prototypes, source file listing, and `static` vs global scope.
