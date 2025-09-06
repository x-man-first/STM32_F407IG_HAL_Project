/*
 * ModeStateMechanic.c
 * Description: Implementation of mode state mechanic functions.
 */
 
// Add your includes here
#include "ModeStateMechanic.h"

void ModeStateMechanic_Init(void);
void ModeStateMechanic_Main(void);

// Add your function implementations below

void ModeStateMechanic_Init(void)
{
// Initialization code here
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Init();

  /* Initialize all configured peripherals */
  GPIO_Init();

}

void ModeStateMechanic_Main(void)
{
    // Update state machine logic here
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_0);
    HAL_Delay(100);
    HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);
    HAL_Delay(100);
}