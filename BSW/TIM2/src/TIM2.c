/**
 * @file TIM2.c
    * @file    TIM2.c
    * @brief   TIM2 peripheral initialization and configuration
    ******************************************************************************
    */

#include "TIM2.h"

// TIM2 handle declaration
TIM_HandleTypeDef htim2;

static uint32_t counter_500ms;
static uint32_t counter_100ms;
static uint32_t counter_50ms;
static uint32_t counter_20ms;
static uint32_t counter_10ms;
static uint32_t counter_1ms;

uint8_t Flag_500ms;
uint8_t Flag_100ms;
uint8_t Flag_50ms;
uint8_t Flag_20ms;
uint8_t Flag_10ms;
uint8_t Flag_1ms;

void MX_TIM2_Init(void);
void TIM2_Handler_IN_IRQ(void);

static void TIM2_Value_Init(void);

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  TIM2_Value_Init();
  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 8399;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  htim2.State = HAL_TIM_STATE_RESET;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_Base_Start_IT(&htim2);
}

void TIM2_Value_Init(void)
{
    counter_1ms = 0;
    counter_10ms = 0;
    counter_20ms = 0;
    counter_50ms = 0;
    counter_100ms = 0;
    counter_500ms = 0;

    Flag_1ms = 0;
    Flag_10ms = 0;
    Flag_20ms = 0;
    Flag_50ms = 0;
    Flag_100ms = 0;
    Flag_500ms = 0;
}

void TIM2_Handler_IN_IRQ(void)
{
        counter_1ms++;
        counter_10ms++;
        counter_20ms++;
        counter_50ms++;
        counter_100ms++;
        counter_500ms++;

        if(counter_1ms == 1)
        {
            Flag_1ms = 1;
            counter_1ms = 0;
        }
        if (counter_10ms == 10)
        {
            Flag_10ms = 1;
            counter_10ms = 0;
        }
        if(counter_20ms == 20)
        {
            Flag_20ms = 1;
            counter_20ms = 0;
        }
        if (counter_50ms == 50)
        {
            Flag_50ms = 1;
            counter_50ms = 0;
        }
        if (counter_100ms == 100)
        {
            Flag_100ms = 1;
            counter_100ms = 0;
        }
        if (counter_500ms == 500)
        {
            Flag_500ms = 1;
            counter_500ms = 0;
        }

}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == (&htim2))
  {
    TIM2_Handler_IN_IRQ();
  }
  if(htim == (&htim1))
  {
    TIM1_Handler_IN_IRQ();
  }
}