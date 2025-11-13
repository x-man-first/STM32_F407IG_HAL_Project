/**
 * @file PWM.h
 * @brief   PWM 驱动头文件（基于 STM32 HAL）
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-11-13
 * 
 */

#ifndef PWM_H
#define PWM_H

#include "stm32f4xx_hal.h"
#include "main.h"

extern TIM_HandleTypeDef htim1;

extern void PWM_Init(void);
extern void PWM_Start(TIM_HandleTypeDef* htim);
extern void PWM_Stop(TIM_HandleTypeDef* htim);
extern void PWM_SetDutyCycle(uint32_t Channel, uint32_t dutycycle);

#endif /* PWM_H */