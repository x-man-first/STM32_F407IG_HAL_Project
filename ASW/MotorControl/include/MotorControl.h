/*
 * MotorControl.h
 *
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H


#include "PWM.h"
#include "key.h"

/* 公共常量 */

#define MC_DEFAULT_PWM_MAX (((10000) - 1)*0.96) /* 占空比最大限值 */

#define SHUTDOWN_PIN                        GPIO_PIN_10     
#define SHUTDOWN_PIN_GPIO                   GPIOF

#define SHUTDOWN_EN                         HAL_GPIO_WritePin(SHUTDOWN_PIN_GPIO,SHUTDOWN_PIN,GPIO_PIN_SET);
#define SHUTDOWN_OFF                        HAL_GPIO_WritePin(SHUTDOWN_PIN_GPIO,SHUTDOWN_PIN,GPIO_PIN_RESET);

#define HALL1_TIM_CH1_PIN                   GPIO_PIN_10     /* HU */
#define HALL1_TIM_CH1_GPIO                  GPIOH

#define HALL1_TIM_CH2_PIN                   GPIO_PIN_11     /* HV */
#define HALL1_TIM_CH2_GPIO                  GPIOH

#define HALL1_TIM_CH3_PIN                   GPIO_PIN_12     /* HW */
#define HALL1_TIM_CH3_GPIO                  GPIOH

#define M1_LOW_SIDE_U_PORT                      GPIOB
#define M1_LOW_SIDE_U_PIN                       GPIO_PIN_13

#define M1_LOW_SIDE_V_PORT                      GPIOB
#define M1_LOW_SIDE_V_PIN                       GPIO_PIN_14

#define M1_LOW_SIDE_W_PORT                      GPIOB
#define M1_LOW_SIDE_W_PIN                       GPIO_PIN_15

#define CCW                         (1)
#define CW                          (2)
#define RUN                         (1)
#define STOP                        (0)

typedef struct {
    __IO uint8_t    run_flag;       /* 运行标志 */
    __IO uint8_t    step_sta;       /* 本次霍尔状态 */
    __IO uint8_t    step_last;      /* 上次霍尔状态 */
    __IO uint8_t    dir;            /* 电机旋转方向 */
    __IO uint16_t   pwm_duty;       /* 电机占空比 */
} bldc_status_type;

extern bldc_status_type bldc_status;

/* 初始化与反初始化 */
extern void MotorControl_Init(void);
extern void MotorControl_DeInit(void);

/* 启动/停止 */
extern void MotorControl_Start(void);
extern void MotorControl_Stop(void);

extern void MotorControl_Control(void);

extern void TIM1_Handler_IN_IRQ(void);


#endif /* MOTOR_CONTROL_H */