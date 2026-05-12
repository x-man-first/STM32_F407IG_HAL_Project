# Code Wiki - STM32F407 BLDC 电机控制系统

---

## 目录

1. [项目概述](#项目概述)
2. [项目架构](#项目架构)
3. [主要模块职责](#主要模块职责)
4. [关键类与函数说明](#关键类与函数说明)
5. [依赖关系](#依赖关系)
6. [项目运行方式](#项目运行方式)
7. [CAN通信协议](#can通信协议)
8. [开发指南](#开发指南)

---

## 项目概述

### 项目简介
这是一个基于 STM32F407 微控制器的无刷直流电机（BLDC）控制系统，采用模块化的分层架构设计。系统支持通过 CAN 总线接收控制指令，并使用霍尔传感器进行 6 步换相控制。

### 主要特性
- 基于 STM32F407 和 HAL 库的硬件抽象
- 模块化的分层架构设计（Core、ASW、BSW、CDD）
- 电机 6 步霍尔换相控制
- CAN 总线通信，支持远程控制
- 按键本地控制（可选）
- 模块生命周期管理（初始化/反初始化）
- 定时调度与中断处理分离
- 心跳监控（CAN 同步帧 + LED 闪烁）

### 硬件平台
- MCU: STM32F407IGT6
- 定时器: TIM1 (PWM)、TIM2 (系统调度)
- 通信: CAN1
- 霍尔传感器接口
- 功率驱动: 三相桥臂（上下桥臂控制）

---

## 项目架构

### 整体架构

项目采用经典的汽车电子分层架构：

```
┌─────────────────────────────────────────────────┐
│                  Core Layer                    │
│  (主程序入口、系统初始化、主循环调度)            │
└────────────────────┬────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────┐
│              ASW (Application SW)              │
│  ┌──────────────────┐  ┌──────────────────┐  │
│  │ ModeStateMechanic│  │  MotorControl    │  │
│  │  (模式管理)      │  │  (电机控制)      │  │
│  └──────────────────┘  └──────────────────┘  │
└────────────────────┬────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────┐
│              BSW (Basic SW)                    │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌───────┐ │
│  │   CAN   │ │  PWM    │ │  GPIO   │ │ TIM2  │ │
│  └─────────┘ └─────────┘ └─────────┘ └───────┘ │
└────────────────────┬────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────┐
│            CDD (Complex Device Driver)         │
│  ┌──────────────────┐  ┌──────────────────┐  │
│  │   Hall Sensor    │  │    Key Input     │  │
│  └──────────────────┘  └──────────────────┘  │
└────────────────────┬────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────┐
│       STM32 HAL & CMSIS  (底层驱动库)          │
└─────────────────────────────────────────────────┘
```

### 目录结构

```
/workspace
├── Core/                  # 核心层
│   ├── Inc/              # 头文件
│   ├── Src/              # 源文件
│   └── Startup/          # 启动文件
├── ASW/                  # 应用层
│   ├── ModeStateMechanic/ # 模式状态管理
│   └── MotorControl/     # 电机控制
├── BSW/                  # 基础软件层
│   ├── CAN/              # CAN 驱动
│   ├── GPIO/             # GPIO 驱动
│   ├── PWM/              # PWM 驱动
│   ├── SystemClock/      # 系统时钟
│   ├── TIM2/             # TIM2 驱动
│   └── TIM_IRQ/          # 定时器中断
├── CDD/                  # 复杂设备驱动
│   ├── Hall/             # 霍尔传感器驱动
│   └── KEY/              # 按键驱动
├── Drivers/              # ST 官方驱动
│   ├── CMSIS/            # CMSIS 库
│   └── STM32F4xx_HAL_Driver/ # HAL 库
└── docs/                 # 文档
```

### 系统状态机

系统由 `ModeStateMechanic` 模块管理，包含以下状态：

| 状态名称            | 枚举值            | 描述                                      |
|--------------------|-------------------|-------------------------------------------|
| MM_INIT_MODE       | 0                 | 初始化模式                                |
| MM_STANDBY_MODE    | 1                 | 待机模式                                  |
| MM_MOTORCTRL_MODE  | 2                 | 电机控制模式（当前运行）                  |
| MM_SPEEDCTRL_MODE  | 3                 | 速度控制模式（预留）                      |
| MM_PRPSLEEP_MODE   | 4                 | 预睡眠模式（反初始化过程）                |
| MM_SLEEP_MODE      | 5                 | 睡眠模式                                  |
| MM_FAILURE_MODE    | 6                 | 故障模式                                  |

状态转换图：
```
INIT_MODE ──(初始化完成)──> STANDBY_MODE ──> MOTORCTRL_MODE
    │                                                           │
    │ (初始化失败)                                              │ (故障发生)
    ▼                                                           ▼
FAILURE_MODE <──────────────────────────────────────────────────
    │
    │
    ▼
STANDBY_MODE <── (可恢复)
```

---

## 主要模块职责

### Core 层

#### main.c ([Core/Src/main.c](file:///workspace/Core/Src/main.c))
- **职责**: 系统入口和主循环
- **主要功能**:
  - `MCU_Init()`: 系统初始化（时钟、外设、模块初始化）
  - `MCU_Main()`: 主循环调度函数
  - 基于 TIM2 的周期性任务调度（1ms/10ms/20ms/50ms/100ms/500ms 标志）
- **调度标志**:
  - `Flag_1ms`: 1ms 周期（调用 ModeStateMechanic）
  - `Flag_100ms`: 100ms 周期（CAN 心跳发送）
  - `Flag_500ms`: 500ms 周期（LED 闪烁）

### ASW 层

#### ModeStateMechanic ([ASW/ModeStateMechanic/](file:///workspace/ASW/ModeStateMechanic/))
- **职责**: 系统模式状态管理、模块生命周期控制
- **主要功能**:
  - 系统状态机调度
  - 模块初始化/反初始化流程控制
  - 故障检测与处理
- **关键文件**:
  - [ModeStateMechanic.h](file:///workspace/ASW/ModeStateMechanic/include/ModeStateMechanic.h): 模式定义和外部接口
  - [ModeStateMechanic.c](file:///workspace/ASW/ModeStateMechanic/src/ModeStateMechanic.c): 状态机实现
  - [ModeStateMechanic_InitTask.h](file:///workspace/ASW/ModeStateMechanic/include/ModeStateMechanic_InitTask.h): 初始化任务配置
  - [ModeStateMechanic_InitTask.c](file:///workspace/ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c): 初始化任务实现

#### MotorControl ([ASW/MotorControl/](file:///workspace/ASW/MotorControl/))
- **职责**: BLDC 电机核心控制逻辑
- **主要功能**:
  - 电机 6 步换相控制
  - 启动/停止控制
  - 正反转控制
  - PWM 占空比控制
  - 支持按键和 CAN 两种控制方式
- **关键文件**:
  - [MotorControl.h](file:///workspace/ASW/MotorControl/include/MotorControl.h): 电机控制接口和数据结构
  - [MotorControl.c](file:///workspace/ASW/MotorControl/src/MotorControl.c): 电机控制实现

### BSW 层

#### CAN ([BSW/CAN/](file:///workspace/BSW/CAN/))
- **职责**: CAN 总线通信
- **主要功能**:
  - CAN 初始化和过滤器配置
  - CAN 消息发送/接收
  - VCU 控制指令解析
  - 心跳同步帧发送
- **关键文件**:
  - [Can.h](file:///workspace/BSW/CAN/include/Can.h): CAN 接口定义
  - [Can.c](file:///workspace/BSW/CAN/src/Can.c): CAN 驱动实现

#### PWM ([BSW/PWM/](file:///workspace/BSW/PWM/))
- **职责**: TIM1 PWM 输出控制
- **主要功能**:
  - PWM 初始化（3 通道）
  - PWM 启动/停止
  - 占空比设置
- **关键文件**:
  - [PWM.h](file:///workspace/BSW/PWM/include/PWM.h): PWM 接口定义
  - [PWM.c](file:///workspace/BSW/PWM/src/PWM.c): PWM 驱动实现

#### TIM2 ([BSW/TIM2/](file:///workspace/BSW/TIM2/))
- **职责**: 系统定时调度器
- **主要功能**:
  - 1ms 定时中断
  - 生成周期性调度标志

### CDD 层

#### Hall ([CDD/Hall/](file:///workspace/CDD/Hall/))
- **职责**: 霍尔传感器驱动
- **主要功能**:
  - 读取霍尔传感器状态
  - 返回电机转子位置（1-6）

#### KEY ([CDD/KEY/](file:///workspace/CDD/KEY/))
- **职责**: 按键输入驱动
- **主要功能**:
  - 按键扫描
  - 按键状态检测

---

## 关键类与函数说明

### 数据结构

#### bldc_status_type ([MotorControl.h L39-45](file:///workspace/ASW/MotorControl/include/MotorControl.h#L39-L45))
```c
typedef struct {
    __IO uint8_t    run_flag;    // 运行标志 (RUN/STOP)
    __IO uint8_t    step_sta;    // 当前霍尔状态
    __IO uint8_t    step_last;   // 上一次霍尔状态
    __IO uint8_t    dir;         // 旋转方向 (CW/CCW)
    __IO uint16_t   pwm_duty;    // PWM 占空比
} bldc_status_type;
```

#### Bldc_Control_Req_type ([MotorControl.h L47-51](file:///workspace/ASW/MotorControl/include/MotorControl.h#L47-L51))
```c
typedef struct {
    __IO uint8_t    start_stop;  // 启动/停止请求
    __IO uint8_t    CW_CCW;      // 方向请求
    __IO uint16_t   pwm_duty;    // 占空比请求
} Bldc_Control_Req_type;
```

### 核心函数

#### ModeStateMechanic 相关

##### ModeStateMechanic_Init() ([ModeStateMechanic.c L33-44](file:///workspace/ASW/ModeStateMechanic/src/ModeStateMechanic.c#L33-L44))
- **功能**: 初始化模式状态机
- **参数**: 无
- **返回值**: 无
- **说明**: 设置初始模式为 MM_INIT_MODE，准备开始模块初始化

##### ModeStateMechanic_Main() ([ModeStateMechanic.c L46-55](file:///workspace/ASW/ModeStateMechanic/src/ModeStateMechanic.c#L46-L55))
- **功能**: 模式状态机主循环（1ms 调用一次）
- **参数**: 无
- **返回值**: 无
- **流程**: 输入采集 → 模式控制 → 输出处理

##### MM_InitDeinit() ([ModeStateMechanic_InitTask.c L157-227](file:///workspace/ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c#L157-L227))
- **功能**: 管理所有模块的初始化/反初始化
- **参数**: 无
- **返回值**: 无
- **说明**: 根据 MM_InitReq 状态，遍历所有模块执行相应的初始化/反初始化流程

#### MotorControl 相关

##### MotorControl_Init() ([MotorControl.c L41-54](file:///workspace/ASW/MotorControl/src/MotorControl.c#L41-L54))
- **功能**: 初始化电机控制模块
- **参数**: 无
- **返回值**: uint8_t (1 成功, 0 失败)
- **流程**:
  - 关闭功率级（SHUTDOWN_OFF）
  - 复位所有下桥臂
  - 停止 PWM 输出
  - 返回成功状态

##### MotorControl_Control() ([MotorControl.c L80-189](file:///workspace/ASW/MotorControl/src/MotorControl.c#L80-L189))
- **功能**: 电机控制主函数（周期性调用）
- **参数**: 无
- **返回值**: 无
- **功能**:
  - 检查控制请求（按键或 CAN）
  - 更新电机状态
  - 设置 PWM 占空比
  - 控制电机启动/停止

##### Motor_Handler_IN_TIM1IRQ() ([MotorControl.c L284-326](file:///workspace/ASW/MotorControl/src/MotorControl.c#L284-L326))
- **功能**: TIM1 中断回调（关键换相函数）
- **参数**: 无
- **返回值**: 无
- **流程**:
  - 检查电机运行标志
  - 读取霍尔传感器状态
  - 根据方向执行 6 步换相
  - 更新下桥臂状态
  - 设置 PWM 输出
- **注意**: 此函数在 ISR 中调用，必须保持简短

##### 6 步换相函数 ([MotorControl.c L212-277](file:///workspace/ASW/MotorControl/src/MotorControl.c#L212-L277))
- `m1_uhvl()`: U 上桥 + V 下桥
- `m1_uhwl()`: U 上桥 + W 下桥
- `m1_vhul()`: V 上桥 + U 下桥
- `m1_vhwl()`: V 上桥 + W 下桥
- `m1_whul()`: W 上桥 + U 下桥
- `m1_whvl()`: W 上桥 + V 下桥

#### CAN 相关

##### CAN1_Init() ([Can.c L27-91](file:///workspace/BSW/CAN/src/Can.c#L27-L91))
- **功能**: CAN 初始化
- **参数**: 无
- **返回值**: 无
- **配置**:
  - 波特率: 由分频器和时间段配置
  - 自动总线关闭: 启用
  - 过滤器: 接收所有 ID

##### CAN1TX() ([Can.c L61-65](file:///workspace/BSW/CAN/src/Can.c#L61-L65))
- **功能**: 发送 CAN 心跳帧（100ms 周期）
- **参数**: 无
- **返回值**: 无
- **帧信息**: ID = CANID_SYNC (0x001)

##### HAL_CAN_RxFifo0MsgPendingCallback() ([Can.c L170-177](file:///workspace/BSW/CAN/src/Can.c#L170-L177))
- **功能**: CAN 接收中断回调
- **参数**: hcan - CAN 句柄
- **返回值**: 无
- **流程**: 接收消息 → 解析处理

##### CAN_RX_Handle() ([Can.c L193-201](file:///workspace/BSW/CAN/src/Can.c#L193-L201))
- **功能**: CAN 接收消息解析
- **参数**: 无
- **返回值**: 无
- **说明**: 解析 VCU 控制指令并更新 Bldc_Control_Req

#### PWM 相关

##### PWM_Init() ([PWM.c L21-97](file:///workspace/BSW/PWM/src/PWM.c#L21-L97))
- **功能**: PWM 初始化（TIM1）
- **参数**: 无
- **返回值**: 无
- **配置**:
  - 周期: 10000 (0-9999)
  - 通道: 1/2/3
  - 模式: PWM1

##### PWM_SetDutyCycle() ([PWM.c L134-148](file:///workspace/BSW/PWM/src/PWM.c#L134-L148))
- **功能**: 设置 PWM 占空比
- **参数**:
  - Channel: PWM 通道 (TIM_CHANNEL_1/2/3)
  - dutycycle: 占空比 (0-9999)
- **返回值**: 无

---

## 依赖关系

### 模块依赖图

```
main.c
├── ModeStateMechanic
│   ├── ModeStateMechanic_InitTask
│   │   └── MotorControl
│   └── CAN (故障检测)
├── MotorControl
│   ├── PWM
│   ├── Hall
│   └── KEY (可选)
├── CAN
│   └── MotorControl (控制请求)
├── PWM
├── GPIO
├── TIM2
└── SystemClock
```

### 关键依赖关系

| 依赖模块           | 被依赖模块       | 依赖关系说明                          |
|--------------------|-------------------|---------------------------------------|
| ModeStateMechanic  | MotorControl      | 调用 MotorControl_Control()          |
| ModeStateMechanic  | CAN               | 检测 CAN 故障状态                     |
| MotorControl       | PWM               | 设置占空比、启停 PWM                  |
| MotorControl       | Hall              | 读取霍尔传感器状态                    |
| MotorControl       | KEY               | 读取按键输入（可选）                  |
| CAN                | MotorControl      | 更新 Bldc_Control_Req                |
| PWM                | TIM1 HAL          | 底层 PWM 控制                         |

### 全局变量依赖

| 全局变量名              | 定义位置                | 使用位置                | 说明                              |
|-------------------------|-------------------------|-------------------------|-----------------------------------|
| bldc_status             | MotorControl.c          | MotorControl.c          | 电机状态                          |
| Bldc_Control_Req        | MotorControl.c          | MotorControl.c, Can.c   | 控制请求（CAN/按键）              |
| ModeMechanic_CurrentMode| ModeStateMechanic.c     | ModeStateMechanic.c     | 当前模式                          |
| MM_InitSts[]            | ModeStateMechanic_InitTask.c | ModeStateMechanic_InitTask.c | 模块初始化状态 |
| MM_InitReq              | ModeStateMechanic_InitTask.c | ModeStateMechanic_InitTask.c | 初始化请求 |
| Flag_1ms / Flag_100ms / Flag_500ms | TIM2相关 | main.c | 调度标志 |

---

## 项目运行方式

### 编译与烧录

#### 使用 VSCode 任务（推荐）
1. 打开 VSCode 命令面板 (Ctrl+Shift+P)
2. 选择 `Tasks: Run Task`
3. 选择以下任务之一:
   - `build`: 编译项目
   - `flash`: 烧录固件
   - `build and flash`: 编译并烧录
   - `rebuild`: 清理并重新编译

#### 使用 J-Link 命令行
```bash
JLinkExe -CommandScript build/Debug/commands.jlink
```

### 运行流程

1. **上电初始化**
   - 时钟初始化 (SystemClock_Init)
   - GPIO 初始化 (GPIO_Init)
   - TIM2 初始化 (MX_TIM2_Init)
   - CAN 初始化 (CAN1_Init)
   - PWM 初始化 (PWM_Init)
   - 按键初始化 (key_init)
   - 模式状态机初始化 (ModeStateMechanic_Init)

2. **初始化模式 (INIT_MODE)**
   - 执行所有模块的初始化流程
   - 等待初始化完成

3. **待机模式 (STANDBY_MODE)**
   - 初始化完成后进入
   - 自动切换到电机控制模式

4. **电机控制模式 (MOTORCTRL_MODE)**
   - 1ms 周期调用 MotorControl_Control()
   - 等待控制请求（CAN 或按键）
   - TIM1 中断触发换相

5. **周期性任务**
   - 100ms: 发送 CAN 心跳帧
   - 500ms: 翻转 LED（PE0）

### 测试验证

1. **LED 检查**
   - 观察 PE0 LED 是否 500ms 翻转一次
   - 确认主循环正常运行

2. **CAN 通信检查**
   - 监听 CAN 总线
   - 确认每 100ms 收到一次 CANID_SYNC (0x001) 帧

3. **电机控制测试**
   - 发送 CANID_VCU_CMD (0x000) 控制帧
   - 观察电机运行状态

---

## CAN 通信协议

### CAN ID 定义

| CAN ID 名称      | ID 值 | 方向  | 描述                              |
|------------------|-------|-------|-----------------------------------|
| CANID_VCU_CMD    | 0x000 | 接收  | VCU 电机控制指令                  |
| CANID_SYNC       | 0x001 | 发送  | 心跳同步帧                        |

### CANID_VCU_CMD (0x000) 帧格式

数据字段长度: 4 字节（使用 Data[0]）

| Bit(s) | 名称          | 描述                                          |
|--------|---------------|-----------------------------------------------|
| 0      | start_stop    | 启动/停止 (1: 启动, 0: 停止)                  |
| 1      | CW_CCW        | 旋转方向 (0: 正转 CW, 1: 反转 CCW)            |
| 8-23   | pwm_duty      | PWM 占空比 (0-9999)                           |

### CANID_SYNC (0x001) 帧格式

数据字段: 8 字节固定值 `[0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA]`

### 控制示例

#### 启动电机，正转，占空比 5000
```
CAN ID: 0x000
Data: [0x01, 0x13, 0x00, 0x00, ...]
      └─ start_stop=1, CW_CCW=0, pwm_duty=5000
```

#### 停止电机
```
CAN ID: 0x000
Data: [0x00, 0x00, 0x00, 0x00, ...]
      └─ start_stop=0
```

---

## 开发指南

### 添加新模块

1. **创建模块代码**
   - 在 ASW 或 BSW 目录下创建模块文件夹
   - 头文件放在 `include/`，源文件放在 `src/`
   - 实现 `Xxx_Init()` 和 `Xxx_DeInit()` 函数

2. **注册到 ModeStateMechanic**
   - 在 [ModeStateMechanic_InitTask.h](file:///workspace/ASW/ModeStateMechanic/include/ModeStateMechanic_InitTask.h) 的 `MM_InitStsIndexes` 枚举中添加索引
   - 在 [ModeStateMechanic_InitTask.c](file:///workspace/ASW/ModeStateMechanic/src/ModeStateMechanic_InitTask.c) 中：
     - 添加初始化/反初始化函数
     - 在 `MM_InitCfg` 数组中添加配置项

3. **更新构建配置**
   - 添加源文件到编译列表
   - 添加头文件路径

### 代码规范

- **分层原则**: ASW 层不应直接调用 HAL，应通过 BSW/CDD
- **中断处理**: ISR 中只处理关键任务，复杂逻辑放主循环
- **命名规范**: 模块名前缀（如 `MotorControl_`）
- **注释**: 关键函数添加功能说明和参数/返回值描述

### 调试建议

1. **使用 LED 指示**
   - 在关键位置翻转 LED 指示状态

2. **TIM1 中断调试**
   - 在 `Motor_Handler_IN_TIM1IRQ()` 中添加 GPIO 翻转
   - 用示波器观察换相频率

3. **CAN 通信调试**
   - 使用 CANoe 或 PCAN-View 监控总线
   - 检查心跳帧是否正常发送

### 扩展建议

1. **速度闭环控制**
   - 在 MM_SPEEDCTRL_MODE 中实现
   - 添加速度传感器或基于霍尔的速度估算

2. **过流/过压保护**
   - 添加 ADC 采样
   - 在故障检测中集成保护逻辑

3. **位置闭环控制**
   - 添加编码器接口
   - 实现位置伺服控制

---

## 参考文档

- [ReadMe.md](file:///workspace/ReadMe.md): 项目快速入门
- [设计文档.md](file:///workspace/docs/设计文档.md): 系统设计说明
- [架构文档.md](file:///workspace/docs/架构文档.md): 架构详细说明
- [接口描述文档.md](file:///workspace/docs/接口描述文档.md): 接口说明
- [DEV_GUIDE.md](file:///workspace/docs/DEV_GUIDE.md): 开发指南

---

*文档最后更新: 2026-05-12*
