#include "MotorControl.h"
#include "TIM2.h"

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == (&htim2))
  {
    TIM2_Handler_IN_IRQ();
  }
  if(htim == (&htim1))
  {
    Motor_Handler_IN_TIM1IRQ();
  }
}