#ifndef __HALL_H
#define __HALL_H

#include "stm32f4xx_hal.h"


#define HALL1_TIM_CH1_PIN                   GPIO_PIN_10     /* HU */
#define HALL1_TIM_CH1_GPIO                  GPIOH

#define HALL1_TIM_CH2_PIN                   GPIO_PIN_11     /* HV */
#define HALL1_TIM_CH2_GPIO                  GPIOH

#define HALL1_TIM_CH3_PIN                   GPIO_PIN_12     /* HW */
#define HALL1_TIM_CH3_GPIO                  GPIOH



extern uint32_t Hallsensor(void);

#endif