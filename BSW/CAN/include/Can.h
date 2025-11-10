#ifndef CAN_H
#define CAN_H

#include "stm32f4xx_hal.h"
#include "main.h"


extern CAN_HandleTypeDef hcan1;

extern void CAN1_Init(void);
extern void CAN1TX(void);

#endif // CAN_H