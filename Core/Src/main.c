/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "ModeStateMechanic.h"
#include "GPIO.h"
#include "SystemClock.h"
#include "TIM2.h"
#include "Can.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void MCU_Init(void);
void MCU_Main(void);

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  MCU_Init();
  /* Infinite loop */
  while (1)
  {
    MCU_Main();
  }
}

void MCU_Init(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Init();

  /* Initialize all configured peripherals */
  GPIO_Init();

  MX_TIM2_Init();

  CAN1_Init();

  ModeStateMechanic_Init();
}

void MCU_Main(void)
{
  if(Flag_1ms == 1)
  {
      Flag_1ms = 0; 

      ModeStateMechanic_Main();
  }
  if(Flag_10ms == 1)
  {
      Flag_10ms = 0;
  }
  if(Flag_20ms == 1)
  {
      Flag_20ms = 0;
  }
  if(Flag_50ms == 1)
  {
      Flag_50ms = 0;
  }
  if(Flag_100ms == 1)
  {
      Flag_100ms = 0;

      CAN1TX();
  }
  if(Flag_500ms == 1)
  {
      Flag_500ms = 0;
      
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);//led toggle every 500ms
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
