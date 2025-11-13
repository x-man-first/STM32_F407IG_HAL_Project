/**
 * @file PWM.c
 * Brief: PWM helper (template)
 * File: PWM.c
 * Place in: /BSW/PWM/src/PWM.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2025-11-13
 */

#include "PWM.h"

TIM_HandleTypeDef htim1;
void PWM_Init(void);
void PWM_Start(TIM_HandleTypeDef* htim);
void PWM_Stop(TIM_HandleTypeDef* htim);
void PWM_SetDutyCycle(uint32_t Channel, uint32_t dutycycle);

/* 初始化 */
void PWM_Init(void)
{
  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 9999;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  htim1.State = HAL_TIM_STATE_RESET;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

      /* 开启定时器通道1输出PWM */
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);

    /* 开启定时器通道2输出PWM */
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_2);

    /* 开启定时器通道3输出PWM */
  HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);
    
  HAL_TIM_Base_Start_IT(&htim1);  /* 启动高级定时器1，并使能更新中断 */
}

/* 启动 PWM 输出 */
void PWM_Start(TIM_HandleTypeDef* htim)
{
    PWM_SetDutyCycle(TIM_CHANNEL_1, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_2, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_3, 0);

    /* 开启定时器通道1输出PWM */
    HAL_TIM_PWM_Start(htim,TIM_CHANNEL_1);

    /* 开启定时器通道2输出PWM */
    HAL_TIM_PWM_Start(htim,TIM_CHANNEL_2);

    /* 开启定时器通道3输出PWM */
    HAL_TIM_PWM_Start(htim,TIM_CHANNEL_3);
}

/* 停止 PWM 输出 */
void PWM_Stop(TIM_HandleTypeDef* htim)
{
    PWM_SetDutyCycle(TIM_CHANNEL_1, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_2, 0);
    PWM_SetDutyCycle(TIM_CHANNEL_3, 0);

    /* 关闭定时器通道1输出PWM */
    HAL_TIM_PWM_Stop(htim,TIM_CHANNEL_1);

    /* 关闭定时器通道2输出PWM */
    HAL_TIM_PWM_Stop(htim,TIM_CHANNEL_2);

    /* 关闭定时器通道3输出PWM */
    HAL_TIM_PWM_Stop(htim,TIM_CHANNEL_3);
}

/* 设置占空比 */
void PWM_SetDutyCycle(uint32_t Channel, uint32_t dutycycle)
{
    if(Channel == TIM_CHANNEL_1)
    {
        htim1.Instance -> CCR1 = dutycycle;
    }
    else if(Channel == TIM_CHANNEL_2)
    {
        htim1.Instance -> CCR2 = dutycycle;
    }
    else if(Channel == TIM_CHANNEL_3)
    {
        htim1.Instance -> CCR3 = dutycycle;
    }
}

