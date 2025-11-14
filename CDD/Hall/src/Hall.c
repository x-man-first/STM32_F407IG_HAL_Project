#include "Hall.h"

uint32_t Hallsensor(void);

/*
 * @brief       获取霍尔传感器引脚状态
 * @param       无
 * @retval      霍尔传感器引脚状态
 */
uint32_t Hallsensor(void)
{
    uint32_t state = 0;
    if(HAL_GPIO_ReadPin(HALL1_TIM_CH1_GPIO,HALL1_TIM_CH1_PIN) != GPIO_PIN_RESET)
    {
        state |= 0x01;
    }
    if(HAL_GPIO_ReadPin(HALL1_TIM_CH2_GPIO,HALL1_TIM_CH2_PIN) != GPIO_PIN_RESET)
    {
        state |= 0x02;
    }
    if(HAL_GPIO_ReadPin(HALL1_TIM_CH3_GPIO,HALL1_TIM_CH3_PIN) != GPIO_PIN_RESET)
    {
        state |= 0x04;
    }
    return   state; 
}