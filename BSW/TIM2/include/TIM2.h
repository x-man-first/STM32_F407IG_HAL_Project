#ifndef __TIM2_H__
#define __TIM2_H__

#include "stm32f4xx_hal.h"
#include "main.h"


extern TIM_HandleTypeDef htim2;
extern uint8_t Flag_500ms;
extern uint8_t Flag_100ms;
extern uint8_t Flag_50ms;
extern uint8_t Flag_20ms;
extern uint8_t Flag_10ms;
extern uint8_t Flag_1ms;

extern void MX_TIM2_Init(void);
extern void TIM2_Handler_IN_IRQ(void);


#endif // __TIM2_H__